/* Direct Link: host or join a Wi-Fi link between two handhelds, and its own updates (update.py, through updater.c).
   Drawn in the menu's look by view.c and pixel.c (the theme and accent from settings.conf, Pixelify Sans).
   libdrm, both panels; the buttons, and touch on the bottom panel. */
#include "view.h"
#include "cast.h"
#include "pixel_sound.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/input.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

static volatile sig_atomic_t g_stop;

static void on_signal(int sig) { (void)sig; g_stop = 1; }

typedef struct {
    uint32_t conn, crtc, fb, handle, pitch, size;
    uint8_t *map;
    drmModeModeInfo mode;
} Panel;

static int g_fd = -1, g_w, g_h, g_shown;
static Panel g_p[2];
static uint32_t *g_buf[2];

static char g_app[512];
static int g_exit;                /* UPD_RESTART after an update: tile.sh opens the new version */

static int g_job;
static pid_t g_child;
static int g_pipe = -1;
static char g_acc[240];
static int g_acc_n;
static char g_job_line[240];

static void copy_str(char *dst, size_t n, const char *src)
{
    size_t i = 0;
    if (n == 0) return;
    while (i + 1 < n && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

static void planes_upright(void)
{
    drmModePlaneRes *pr;
    uint32_t j;
    drmSetClientCap(g_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    pr = drmModeGetPlaneResources(g_fd);
    if (!pr) return;
    for (j = 0; j < pr->count_planes; j++) {
        drmModeObjectProperties *pp = drmModeObjectGetProperties(g_fd, pr->planes[j], DRM_MODE_OBJECT_PLANE);
        uint32_t i;
        if (!pp) continue;
        for (i = 0; i < pp->count_props; i++) {
            drmModePropertyRes *prop = drmModeGetProperty(g_fd, pp->props[i]);
            if (!prop) continue;
            if (!strcmp(prop->name, "rotation") && pp->prop_values[i] != DRM_MODE_ROTATE_0)
                drmModeObjectSetProperty(g_fd, pr->planes[j], DRM_MODE_OBJECT_PLANE, prop->prop_id, DRM_MODE_ROTATE_0);
            drmModeFreeProperty(prop);
        }
        drmModeFreeObjectProperties(pp);
    }
    drmModeFreePlaneResources(pr);
}

static void panel_free(Panel *p)
{
    if (p->map && p->map != MAP_FAILED) munmap(p->map, p->size);
    if (p->fb) drmModeRmFB(g_fd, p->fb);
    if (p->handle) {
        struct drm_mode_destroy_dumb d;
        memset(&d, 0, sizeof d);
        d.handle = p->handle;
        drmIoctl(g_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &d);
    }
    memset(p, 0, sizeof *p);
}

static void display_close(void)
{
    int s;
    if (g_fd < 0) return;
    drmDropMaster(g_fd);
    for (s = 0; s < 2; s++) panel_free(&g_p[s]);
    close(g_fd);
    g_fd = -1;
}

static int make_buf(Panel *p)
{
    struct drm_mode_create_dumb c;
    struct drm_mode_map_dumb m;
    uint32_t handles[4], pitches[4], offsets[4];
    memset(&c, 0, sizeof c);
    c.width = (uint32_t)g_w;
    c.height = (uint32_t)g_h;
    c.bpp = 32;
    if (drmIoctl(g_fd, DRM_IOCTL_MODE_CREATE_DUMB, &c)) return 0;
    p->handle = c.handle;
    p->pitch = c.pitch;
    p->size = c.size;
    handles[0] = p->handle; pitches[0] = p->pitch; offsets[0] = 0;
    handles[1] = handles[2] = handles[3] = 0;
    pitches[1] = pitches[2] = pitches[3] = 0;
    offsets[1] = offsets[2] = offsets[3] = 0;
    if (drmModeAddFB2(g_fd, (uint32_t)g_w, (uint32_t)g_h, DRM_FORMAT_XRGB8888, handles, pitches, offsets, &p->fb, 0))
        return 0;
    memset(&m, 0, sizeof m);
    m.handle = p->handle;
    if (drmIoctl(g_fd, DRM_IOCTL_MODE_MAP_DUMB, &m)) return 0;
    p->map = mmap(NULL, p->size, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, (off_t)m.offset);
    if (p->map == MAP_FAILED) return 0;
    memset(p->map, 0, p->size);
    return 1;
}

static int display_open(void)
{
    const char *top = getenv("BANANA_TOP");
        char path[288];
    DIR *d;
    struct dirent *e;
    int t;
    if (!top || !top[0]) top = "DSI-2";
    d = opendir("/dev/dri");
    if (!d) { fprintf(stderr, "direct-link: no /dev/dri (%s)\n", strerror(errno)); return 0; }
    while ((e = readdir(d))) {
        drmModeRes *res;
        typedef struct { uint32_t conn, crtc; drmModeModeInfo mode; int istop; } Found;
        Found found[2], tmp;
        int nfound = 0;
        uint32_t used[2];
        int i;
        if (strncmp(e->d_name, "card", 4)) continue;
        snprintf(path, sizeof path, "/dev/dri/%s", e->d_name);
        g_fd = open(path, O_RDWR | O_CLOEXEC);
        if (g_fd < 0) continue;
        res = drmModeGetResources(g_fd);
        if (!res) { close(g_fd); g_fd = -1; continue; }
        for (i = 0; i < res->count_connectors && nfound < 2; i++) {
            drmModeConnector *c = drmModeGetConnector(g_fd, res->connectors[i]);
            const char *tn;
            char name[32];
            drmModeModeInfo mode;
            uint32_t crtc = 0;
            int m, en;
            if (!c) continue;
            tn = drmModeGetConnectorTypeName(c->connector_type);
            if (!tn || strcmp(tn, "DSI") || c->connection != DRM_MODE_CONNECTED || c->count_modes < 1) {
                drmModeFreeConnector(c);
                continue;
            }
            mode = c->modes[0];
            for (m = 0; m < c->count_modes; m++)
                if (c->modes[m].type & DRM_MODE_TYPE_PREFERRED) { mode = c->modes[m]; break; }
            for (en = 0; en < c->count_encoders && !crtc; en++) {
                drmModeEncoder *enc = drmModeGetEncoder(g_fd, c->encoders[en]);
                int k;
                if (!enc) continue;
                for (k = 0; k < res->count_crtcs && !crtc; k++) {
                    int u, taken = 0;
                    if (!(enc->possible_crtcs & (1u << k))) continue;
                    for (u = 0; u < nfound; u++) if (used[u] == res->crtcs[k]) taken = 1;
                    if (!taken) crtc = res->crtcs[k];
                }
                drmModeFreeEncoder(enc);
            }
            if (crtc) {
                snprintf(name, sizeof name, "%s-%u", tn, c->connector_type_id);
                used[nfound] = crtc;
                found[nfound].conn = c->connector_id;
                found[nfound].crtc = crtc;
                found[nfound].mode = mode;
                found[nfound].istop = !strcmp(name, top);
                nfound++;
            }
            drmModeFreeConnector(c);
        }
        drmModeFreeResources(res);
        if (nfound < 2) { close(g_fd); g_fd = -1; continue; }
        if (!found[0].istop && !found[1].istop) {
            tmp = found[0];
            found[0] = found[1];
            found[1] = tmp;
        }
        if (found[1].istop) {
            tmp = found[0];
            found[0] = found[1];
            found[1] = tmp;
        }
        if (found[0].mode.hdisplay != found[1].mode.hdisplay || found[0].mode.vdisplay != found[1].mode.vdisplay) {
            fprintf(stderr, "direct-link: the two panels differ in size\n");
            close(g_fd); g_fd = -1; closedir(d); return 0;
        }
        for (t = 0; t < 50; t++) { if (drmSetMaster(g_fd) == 0) break; usleep(20000); }
        g_w = found[0].mode.hdisplay;
        g_h = found[0].mode.vdisplay;
        for (i = 0; i < 2; i++) {
            memset(&g_p[i], 0, sizeof g_p[i]);
            g_p[i].conn = found[i].conn;
            g_p[i].crtc = found[i].crtc;
            g_p[i].mode = found[i].mode;
            if (!make_buf(&g_p[i])) {
                fprintf(stderr, "direct-link: no scanout buffer (%s)\n", strerror(errno));
                display_close();
                closedir(d);
                return 0;
            }
        }
        planes_upright();
        closedir(d);
        return 1;
    }
    closedir(d);
    fprintf(stderr, "direct-link: no two DSI panels\n");
    return 0;
}

static void present(void)
{
    int s;
    if (g_shown) return;
    for (s = 0; s < 2; s++)
        drmModeSetCrtc(g_fd, g_p[s].crtc, g_p[s].fb, 0, 0, &g_p[s].conn, 1, &g_p[s].mode);
    g_shown = 1;
}
static void note_out(char c)
{
    if (c == '\n' || c == '\r') {
        if (g_acc_n > 0) {
            g_acc[g_acc_n] = 0;
            copy_str(g_job_line, sizeof g_job_line, g_acc);
            g_acc_n = 0;
        }
        return;
    }
    if (g_acc_n + 1 < (int)sizeof g_acc) g_acc[g_acc_n++] = c;
}

static void drain_job(void)
{
    char b[128];
    ssize_t n;
    int i;
    if (g_pipe < 0) return;
    while ((n = read(g_pipe, b, sizeof b)) > 0)
        for (i = 0; i < (int)n; i++) note_out(b[i]);
}

static void start_job(int job)
{
    int p[2];
    pid_t pid;
    char path[640];
    const char *script;
    if (g_child > 0) return;
    script = job == JOB_HOST ? "host.sh" : job == JOB_JOIN ? "join.sh" : "down.sh";
    if (pipe(p) < 0) return;
    pid = fork();
    if (pid < 0) { close(p[0]); close(p[1]); return; }
    if (pid == 0) {
        close(p[0]);
        dup2(p[1], 1);
        snprintf(path, sizeof path, "%s/%s", g_app, script);
        execl("/bin/sh", "sh", path, (char *)NULL);
        _exit(127);
    }
    close(p[1]);
    fcntl(p[0], F_SETFL, fcntl(p[0], F_GETFL, 0) | O_NONBLOCK);
    g_pipe = p[0];
    g_child = pid;
    g_job = job;
    g_acc_n = 0;
    g_job_line[0] = 0;
}

static int reap_job(char *err, int errn)
{
    int st, kind;
    pid_t p;
    const char *m;
    if (g_child <= 0) return 0;
    p = waitpid(g_child, &st, WNOHANG);
    if (p == 0 || p < 0) return 0;
    drain_job();
    note_out('\n');
    close(g_pipe);
    g_pipe = -1;
    g_child = 0;
    kind = g_job;
    g_job = JOB_NONE;
    if (!WIFEXITED(st) || WEXITSTATUS(st) != 0) {
        m = g_job_line;
        if (!strncmp(m, "error ", 6)) m += 6;
        if (!m[0]) m = kind == JOB_DOWN ? "could not disconnect" : "the link did not start";
        copy_str(err, (size_t)errn, m);
    } else err[0] = 0;
    return 1;
}


/* a drawn canvas into its panel's scanout buffer */
static void blit(int s)
{
    int y;
    for (y = 0; y < g_h; y++)
        memcpy(g_p[s].map + (size_t)y * g_p[s].pitch, g_buf[s] + (size_t)y * (size_t)g_w, (size_t)g_w * 4);
}

static void kv(char *line, const char *key, char *dst, int n)
{
    size_t k = strlen(key);
    if (n > 0 && strncmp(line, key, k) == 0 && line[k] == '=')
        copy_str(dst, (size_t)n, line + k + 1);
}

static int radio_ok(void)
{
    return access("/sys/class/net/wlan0", F_OK) == 0;
}

static int linked_now(const Model *m)
{
    if (!strcmp(m->state, "up") && (!strcmp(m->role, "host") || !strcmp(m->role, "join")))
        return 1;
    if (g_job == JOB_DOWN && strcmp(m->state, "down") &&
        (!strcmp(m->role, "host") || !strcmp(m->role, "join")))
        return 1;
    return 0;
}

/* the other console on the link: a neighbour in 10.42.0.x that answered (the kernel's ARP table) */
static void peer_ip(Model *m)
{
    char line[256];
    FILE *f;
    m->peer[0] = 0;
    if (!m->linked) return;
    f = fopen("/proc/net/arp", "r");
    if (f) {
        while (fgets(line, sizeof line, f)) {
            char ip[64], hw[32], flags[16], mac[32], mask[16], dev[32];
            if (sscanf(line, "%63s %31s %15s %31s %15s %31s", ip, hw, flags, mac, mask, dev) != 6) continue;
            if (strncmp(ip, "10.42.0.", 8) || !strcmp(ip, m->ip) || !strcmp(flags, "0x0")) continue;
            if (strcmp(dev, "wlan0")) continue;
            copy_str(m->peer, sizeof m->peer, ip);
            break;
        }
        fclose(f);
    }
    /* a guest knows where the host is, while wifi-link hears it */
    if (!strcmp(m->role, "join")) copy_str(m->peer, sizeof m->peer, m->lost ? "" : "10.42.0.1");
}

/* what status.sh says (wifi-link's state, role, ip, home_wifi; the version) */
/* what the screens show about the link, one line each time it changes (with wifi-link's own log, it tells after a test
   whether the app and the link agreed: the Mac can't reach a console while it is on the link). */
#define UI_LOG "/data/banana/logs/direct-link-ui.log"
static void ui_log(const Model *m)
{
    static char last[512];
    char now[512], ts[32];
    time_t t = time(NULL);
    FILE *f;
    snprintf(now, sizeof now, "state=%s role=%s phase=%s ip=%s peers=%d peer_ips=%s lost=%d job=%d shows=%s", m->state,
             m->role, m->phase, m->ip, m->peers, m->peer_ips, m->lost, m->job,
             !m->linked ? "off" : view_connected(m) ? "connected" : m->lost ? "searching" : "waiting");
    if (!strcmp(now, last)) return;
    snprintf(last, sizeof last, "%s", now);
    if (!(f = fopen(UI_LOG, "a"))) return;
    if (ftell(f) > 200000) { fclose(f); if (!(f = fopen(UI_LOG, "w"))) return; }   /* (kept small: started anew) */
    strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", localtime(&t));
    fprintf(f, "%s %s\n", ts, now);
    fclose(f);
}

static void load_link(Model *m)
{
    char cmd[640], line[512];
    FILE *f;
    m->state[0] = m->role[0] = m->ip[0] = m->home[0] = m->phase[0] = m->peer_ips[0] = 0;
    m->peers = 0;
    snprintf(m->state, sizeof m->state, "down");
    snprintf(cmd, sizeof cmd, "sh '%s/status.sh' 2>/dev/null", g_app);
    f = popen(cmd, "r");
    if (f) {
        while (fgets(line, sizeof line, f)) {
            char *nl = strchr(line, '\n');
            if (nl) *nl = 0;
            kv(line, "state", m->state, (int)sizeof m->state);
            kv(line, "role", m->role, (int)sizeof m->role);
            kv(line, "ip", m->ip, (int)sizeof m->ip);
            kv(line, "home_wifi", m->home, (int)sizeof m->home);
            kv(line, "phase", m->phase, (int)sizeof m->phase);
            kv(line, "version", m->version, (int)sizeof m->version);
            if (!strncmp(line, "peers=", 6)) m->peers = atoi(line + 6);
            kv(line, "peer_ips", m->peer_ips, (int)sizeof m->peer_ips);
        }
        pclose(f);
    }
    m->radio = radio_ok();
    m->job = g_job;
    m->linked = linked_now(m);
    if (m->linked) m->mode = MODE_WIFI;           /* the link that is up is a Wi-Fi one */
    /* a guest whose host stopped answering: wifi-link looks for it again, then gives up (phase=lost) */
    m->lost = m->linked && !strcmp(m->role, "join") && !strcmp(m->phase, "lost");
    peer_ip(m);
    ui_log(m);
}

static int run_sh(const char *script, char *err, int errn)
{
    char cmd[640], line[512];
    FILE *f;
    int st;
    err[0] = 0;
    snprintf(cmd, sizeof cmd, "sh '%s/%s' 2>&1", g_app, script);
    f = popen(cmd, "r");
    if (!f) { snprintf(err, (size_t)errn, "could not run %s", script); return 0; }
    if (fgets(line, sizeof line, f)) {
        char *nl = strchr(line, '\n');
        if (nl) *nl = 0;
        copy_str(err, (size_t)errn, line);
    }
    st = pclose(f);
    if (st == 0) err[0] = 0;
    return st == 0;
}

static int ticks(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static PxSettings g_settings;

/* the status row's battery and clock; 1 when they changed */
static int status_tick(Model *m)
{
    char clock[16];
    int ch = 0, bat = px_battery(&ch), changed;
    px_clock(clock, sizeof clock, &g_settings);
    changed = bat != m->battery || ch != m->charging || strcmp(clock, m->clock);
    m->battery = bat;
    m->charging = ch;
    copy_str(m->clock, sizeof m->clock, clock);
    return changed;
}

static void draw(const Model *m)
{
    PxCanvas c;
    px_canvas(&c, g_buf[0], g_w, g_h);
    view_top(&c, m);
    blit(0);
    px_canvas(&c, g_buf[1], g_w, g_h);
    view_bottom(&c, m);
    blit(1);
    present();
}

static int bit_on(int fd, int ev, int code)
{
    unsigned long b[KEY_MAX / (8 * (int)sizeof(long)) + 1];
    memset(b, 0, sizeof b);
    if (ioctl(fd, EVIOCGBIT(ev, sizeof b), b) < 0) return 0;
    return (b[code / (8 * (int)sizeof(long))] >> (code % (8 * (int)sizeof(long)))) & 1;
}

typedef struct {
    int fd;
    int touch;
    int minx, maxx, miny, maxy;
    int x, y, down, was;
} Dev;

enum { ACT_NONE, ACT_UP, ACT_DOWN, ACT_LEFT, ACT_RIGHT, ACT_OK, ACT_BACK, ACT_Y, ACT_MENU, ACT_ROW, ACT_MODE,
       ACT_ABOUT, ACT_PICK, ACT_CANCEL, ACT_X };

/* Nintendo layout: A is the east button, B the south one, Y the west one */
static int key_action(int code, int value)
{
    if (value != 1) return ACT_NONE;
    switch (code) {
    case KEY_UP: case BTN_DPAD_UP: return ACT_UP;
    case KEY_DOWN: case BTN_DPAD_DOWN: return ACT_DOWN;
    case KEY_LEFT: case BTN_DPAD_LEFT: return ACT_LEFT;
    case KEY_RIGHT: case BTN_DPAD_RIGHT: return ACT_RIGHT;
    case BTN_EAST: case KEY_ENTER: return ACT_OK;
    case BTN_WEST: return ACT_Y;
    case BTN_NORTH: return ACT_X;
    case BTN_SOUTH: case KEY_BACKSPACE: case KEY_ESC: return ACT_BACK;
    case BTN_START: return ACT_MENU;
    default: return ACT_NONE;
    }
}

static int touch_px(int v, int lo, int hi, int span)
{
    if (hi <= lo || span <= 0) return 0;
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    return (int)((long)(v - lo) * span / (hi - lo + 1));
}

/* a touch that lifted: what is under it on the bottom panel (the last match: a mode chip over its row) */
static int touch_action(int hx, int hy, int *arg)
{
    int h, act = ACT_NONE;
    for (h = 0; h < g_nhits; h++) {
        PxRect r = g_hits[h].r;
        if (hx < r.x || hx >= r.x + r.w || hy < r.y || hy >= r.y + r.h) continue;
        switch (g_hits[h].id) {
        case HIT_ROW: act = ACT_ROW; *arg = g_hits[h].arg; break;
        case HIT_MODE: act = ACT_MODE; *arg = g_hits[h].arg; break;
        case HIT_BACK: act = ACT_BACK; break;
        case HIT_OK: act = ACT_OK; break;
        case HIT_Y: act = ACT_Y; break;
        case HIT_MENU: act = ACT_MENU; break;
        case HIT_PICK: act = ACT_PICK; *arg = g_hits[h].arg; break;
        case HIT_CANCEL: act = ACT_CANCEL; break;
        case HIT_X: act = ACT_X; break;
        default: break;
        }
    }
    return act;
}

static int app_dir(void)
{
    char exe[512];
    ssize_t n;
    char *slash;
    n = readlink("/proc/self/exe", exe, sizeof exe - 1);
    if (n < 0) return 0;
    exe[n] = 0;
    slash = strrchr(exe, '/');
    if (!slash) return 0;
    *slash = 0;
    slash = strrchr(exe, '/');
    if (!slash) return 0;
    *slash = 0;
    snprintf(g_app, sizeof g_app, "%s", exe);
    return 1;
}

static void connect_now(Model *m)
{
    if (!view_can_connect(m)) return;
    m->err[0] = 0;
    m->role_sel = m->pick == PICK_JOIN;
    start_job(m->role_sel ? JOB_JOIN : JOB_HOST);
}

/* Cancel while a link starts (or joins): wifi-link down, which doesn't wait for the start's lock; the start then
   ends on its own ("stopped before it came up"), and reap_job drops that message (docs/UX.md) */
static void cancel_start(Model *m)
{
    pid_t pid;
    char path[640];
    if (m->canceling || (g_job != JOB_HOST && g_job != JOB_JOIN)) return;
    m->canceling = 1;
    m->err[0] = 0;
    pid = fork();
    if (pid == 0) {
        int fd = open("/dev/null", O_RDWR);
        if (fd >= 0) { dup2(fd, 0); dup2(fd, 1); dup2(fd, 2); }
        setsid();
        snprintf(path, sizeof path, "%s/down.sh", g_app);
        execl("/bin/sh", "sh", path, (char *)NULL);
        _exit(127);
    }
}

typedef struct { int view, cursor, mode, role_sel, pick; } Snap;
static Snap snap(const Model *m)
{
    Snap s;
    s.view = m->view;
    s.cursor = m->cursor;
    s.mode = m->mode;
    s.role_sel = m->role_sel;
    s.pick = m->pick;
    return s;
}

/* scroll when the highlight moves; select when opening a view or confirming; back when leaving */
static const char *sound_of(int act, Snap b, const Model *a, int leave)
{
    if (leave) return act == ACT_BACK ? "back" : "select";
    if (a->view != b.view) return act == ACT_BACK ? "back" : "select";
    if (a->cursor != b.cursor || a->mode != b.mode || a->role_sel != b.role_sel || a->pick != b.pick) return "scroll";
    if (act == ACT_OK) return "select";
    return NULL;
}

/* one action (never while a script runs) */
static void handle(int act, int arg, Model *m)
{
    if (act == ACT_NONE) return;
    /* while a link starts, only Cancel (B or its key on the screen) */
    if (g_child > 0) {
        if ((act == ACT_CANCEL || act == ACT_BACK) && m->view == V_MAIN) cancel_start(m);
        return;
    }
    if (m->view == V_SETTINGS) {
        if (act == ACT_BACK || act == ACT_MENU) { m->view = V_MAIN; m->cursor = 0; return; }
        if (m->linked && act != ACT_BACK) return;
        if (act == ACT_UP) { if (m->cursor > 0) m->cursor--; return; }
        if (act == ACT_DOWN) { if (m->cursor < SET_COUNT - 1) m->cursor++; return; }
        if (act == ACT_Y) { m->view = V_UNINSTALL; return; }
        if (act == ACT_OK && m->cursor == SET_ABOUT) { m->view = V_ABOUT; return; }
        if (act == ACT_OK && m->cursor == SET_UPDATES) {
            if (m->upd.state == UPD_AVAILABLE) m->view = V_UPDATE;
            else upd_start(&m->upd, g_app, "direct-link", 0);
            return;
        }
        if (act == ACT_ROW) {
            if (arg != m->cursor) { m->cursor = arg; return; }
            if (arg == SET_ABOUT) { m->view = V_ABOUT; return; }
            act = ACT_OK;
        }
        return;
    }
    if (m->view == V_ABOUT) {
        if (act == ACT_BACK || act == ACT_ABOUT || act == ACT_OK) m->view = V_SETTINGS;
        return;
    }
    if (m->view == V_UPDATED) {
        /* the new version is in place: tile.sh opens it */
        if (act == ACT_OK) { g_exit = UPD_RESTART; g_stop = 1; }
        return;
    }
    if (m->view == V_UPDATE) {
        if (act == ACT_OK && !m->linked) { m->err[0] = 0; upd_start(&m->upd, g_app, "direct-link", 1); }
        if (act == ACT_OK || act == ACT_BACK) m->view = V_MAIN;
        return;
    }
    /* while it installs, the app stays open and nothing starts a link */
    if (upd_installing(&m->upd) && act != ACT_UP && act != ACT_DOWN && act != ACT_ABOUT && act != ACT_ROW) return;
    if (m->view == V_UNINSTALL) {
        if (act == ACT_OK) {
            if (run_sh("uninstall.sh", m->err, (int)sizeof m->err)) g_stop = 1;
            else m->view = V_MAIN;
        } else if (act == ACT_BACK) { m->view = V_MAIN; m->err[0] = 0; }
        return;
    }
    if (m->view == V_DISC) {
        if (act == ACT_OK) { m->view = V_MAIN; m->err[0] = 0; start_job(JOB_DOWN); }
        else if (act == ACT_BACK) { m->view = V_MAIN; m->err[0] = 0; }
        return;
    }
    switch (act) {
    case ACT_BACK: g_stop = 1; return;
    case ACT_MENU:
        if (!m->linked) { m->view = V_SETTINGS; m->cursor = 0; }
        return;
    default: break;
    }
    if (m->linked) {
        /* the link is up: X (or its red key) asks to disconnect */
        if (act == ACT_X && !upd_installing(&m->upd)) m->view = V_DISC;
        return;
    }
    /* Host and Join (docs/UX.md): left and right pick one; A confirms the one picked. A touch picks the one under
       it, and a touch on the one already picked confirms it: from a fresh start, two touches start the link */
    switch (act) {
    case ACT_LEFT: m->pick = PICK_HOST; return;
    case ACT_RIGHT: m->pick = PICK_JOIN; return;
    case ACT_PICK:
        if (m->pick == arg) connect_now(m);
        else m->pick = arg;
        return;
    case ACT_OK:
        if (!upd_installing(&m->upd)) connect_now(m);
        return;
    default: return;
    }
}

int main(void)
{
    Model m;
    Dev dev[32];
    int ndev = 0, i, dirty = 1, dots = -1, cancel_t = 0, status_t = 0;
    DIR *d;
    struct dirent *e;
    memset(&m, 0, sizeof m);
    m.pick = PICK_NONE;
    signal(SIGTERM, on_signal);
    signal(SIGINT, on_signal);
    signal(SIGCHLD, SIG_DFL);
    if (!app_dir()) { fprintf(stderr, "direct-link: cannot find the app directory\n"); return 1; }
    px_settings_read(NULL, &g_settings);
    px_pal_pixelado(&px);   /* (Pixelado's design system, whatever the menu's theme) */
    px_lang_set(g_settings.language, view_tr);
    px_lang_add(upd_tr);
    px_sound_init(&g_settings, NULL);
    if (!display_open()) return 1;
    g_buf[0] = calloc((size_t)g_w * (size_t)g_h, sizeof(uint32_t));
    g_buf[1] = calloc((size_t)g_w * (size_t)g_h, sizeof(uint32_t));
    if (!g_buf[0] || !g_buf[1]) { display_close(); return 1; }
    {
        /* the system's Pixelify Sans, else the app's own copy (packed on a system without it) */
        char own[600];
        snprintf(own, sizeof own, "%s/fonts", g_app);
        if (!px_fonts_load_any(own)) fprintf(stderr, "direct-link: Pixelify Sans was not found\n");
    }
    d = opendir("/dev/input");
    if (d) {
        while ((e = readdir(d)) && ndev < 32) {
            char path[320], sys[320], real[PATH_MAX];
            int fd, bottom = 0;
            if (strncmp(e->d_name, "event", 5)) continue;
            snprintf(path, sizeof path, "/dev/input/%s", e->d_name);
            fd = open(path, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
            if (fd < 0) continue;
            snprintf(sys, sizeof sys, "/sys/class/input/%s/device", e->d_name);
            if (realpath(sys, real) && strstr(real, "fe5e0000.i2c")) bottom = 1;
            memset(&dev[ndev], 0, sizeof dev[ndev]);
            dev[ndev].fd = fd;
            if (bottom && (bit_on(fd, EV_ABS, ABS_MT_POSITION_X) || bit_on(fd, EV_ABS, ABS_X))) {
                struct input_absinfo ax, ay;
                int mx = bit_on(fd, EV_ABS, ABS_MT_POSITION_X) ? ABS_MT_POSITION_X : ABS_X;
                int my = bit_on(fd, EV_ABS, ABS_MT_POSITION_Y) ? ABS_MT_POSITION_Y : ABS_Y;
                memset(&ax, 0, sizeof ax);
                memset(&ay, 0, sizeof ay);
                ioctl(fd, EVIOCGABS(mx), &ax);
                ioctl(fd, EVIOCGABS(my), &ay);
                dev[ndev].touch = 1;
                dev[ndev].minx = ax.minimum;
                dev[ndev].maxx = ax.maximum;
                dev[ndev].miny = ay.minimum;
                dev[ndev].maxy = ay.maximum;
                dev[ndev].x = ax.value;
                dev[ndev].y = ay.value;
            }
            ndev++;
        }
        closedir(d);
    }
    load_link(&m);
    copy_str(m.upd.current, sizeof m.upd.current, m.version);
    /* a look for a newer version as it opens (in the background; on a link, or with no network, it just says so) */
    if (m.view == V_MAIN) upd_start(&m.upd, g_app, "direct-link", 0);
    status_tick(&m);
    while (!g_stop) {
        struct pollfd pf[33];
        int nf = 0, r, act = ACT_NONE, arg = 0, now_dots, was_busy;
        for (i = 0; i < ndev; i++) {
            pf[nf].fd = dev[i].fd;
            pf[nf].events = POLLIN;
            nf++;
        }
        if (g_pipe >= 0) { pf[nf].fd = g_pipe; pf[nf].events = POLLIN; nf++; }
        m.job = g_job;
        m.dots = dots < 0 ? 0 : dots;
        if (dirty) draw(&m);
        dirty = 0;
        r = poll(pf, (nfds_t)nf, upd_running(&m.upd) ? 200 : view_cast(&m) ? 250 : g_child > 0 || m.lost || m.canceling || (m.linked && !view_connected(&m)) ? 400 : 1000);
        if (r < 0) { if (errno == EINTR) continue; break; }
        if (g_pipe >= 0 && nf > 0 && (pf[nf - 1].revents & POLLIN)) drain_job();
        for (i = 0; i < ndev; i++) {
            struct input_event ev;
            if (!(pf[i].revents & POLLIN)) continue;
            while (read(dev[i].fd, &ev, sizeof ev) == (ssize_t)sizeof ev) {
                if (dev[i].touch) {
                    if (ev.type == EV_ABS && (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X))
                        dev[i].x = ev.value;
                    else if (ev.type == EV_ABS && (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y))
                        dev[i].y = ev.value;
                    else if (ev.type == EV_ABS && ev.code == ABS_MT_TRACKING_ID)
                        dev[i].down = ev.value >= 0;
                    else if (ev.type == EV_KEY && ev.code == BTN_TOUCH)
                        dev[i].down = ev.value != 0;
                    else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
                        if (!dev[i].down && dev[i].was) {
                            int hx = touch_px(dev[i].x, dev[i].minx, dev[i].maxx, g_w);
                            int hy = touch_px(dev[i].y, dev[i].miny, dev[i].maxy, g_h);
                            int a = touch_action(hx, hy, &arg);
                            if (a) act = a;
                        }
                        dev[i].was = dev[i].down;
                    }
                    continue;
                }
                if (ev.type == EV_KEY) {
                    int a = key_action(ev.code, ev.value);
                    if (a) act = a;
                }
            }
        }
        was_busy = g_child > 0;
        if (reap_job(m.err, (int)sizeof m.err)) {
            if (m.canceling) m.err[0] = 0;   /* (a cancelled start ends with "stopped before it came up") */
            load_link(&m); dirty = 1;
        }
        /* a cancel is over once nothing runs and the link is down */
        if (m.canceling && g_child <= 0 && ticks() - cancel_t >= 500) {
            cancel_t = ticks();
            load_link(&m);
            if (!strcmp(m.state, "down")) { m.canceling = 0; m.err[0] = 0; dirty = 1; }
        }
        if (act != ACT_NONE && (!was_busy || act == ACT_CANCEL || act == ACT_BACK)) {
            Snap before = snap(&m);
            handle(act, arg, &m);
            m.job = g_job;
            if (g_job != JOB_NONE) load_link(&m);
            {
                const char *snd = sound_of(act, before, &m, g_stop);
                if (snd) px_sound_play(snd);
            }
            dirty = 1;
        }
        if (upd_poll(&m.upd)) {
            dirty = 1;
            if (!upd_running(&m.upd) && m.upd.installing && m.upd.state == UPD_DONE) {
                m.view = V_UPDATED;
                load_link(&m);
            }
        }
        /* the characters' idle: a beat every 250 ms, a redraw only when one on screen changes pose */
        {
            unsigned who = view_cast(&m);
            int beat = (ticks() / 250) % CAST_BEATS, k;
            if (beat != m.beat) {
                for (k = 0; k < CAST_COUNT; k++)
                    if ((who >> k & 1) && cast_pose(k, beat) != cast_pose(k, m.beat)) dirty = 1;
                m.beat = beat;
            }
        }
        now_dots = g_child > 0 || m.lost || m.canceling || (m.linked && !view_connected(&m)) || upd_running(&m.upd)
                   ? (ticks() / 400) % 3 : -1;
        if (now_dots != dots) {
            dots = now_dots;
            if (g_child > 0) load_link(&m);
            dirty = 1;
        }
        /* the status script once a second, by the clock: not only when poll() times out, since an input device may
           never let it (the RG DS Plus's joypad reports its sticks' noise all the time, so a host never saw its guest
           arrive), and the characters wake the loop more often too */
        if (g_child <= 0 && ticks() - status_t >= 900) {
            Model n = m;
            status_t = ticks();
            load_link(&n);
            if (strcmp(n.state, m.state) || strcmp(n.role, m.role) || strcmp(n.ip, m.ip) || strcmp(n.home, m.home) ||
                strcmp(n.peer, m.peer) || n.peers != m.peers || strcmp(n.peer_ips, m.peer_ips) || n.radio != m.radio || n.linked != m.linked || n.lost != m.lost) {
                /* the link went down without us: a guest whose host left (wifi-link gave up and restored) */
                if (m.linked && !n.linked && !strcmp(m.role, "join"))
                    snprintf(n.err, sizeof n.err, "%s%s%s", px_tr("The host closed the link."),
                             !strcmp(m.home, "on") ? " " : "", !strcmp(m.home, "on") ? px_tr("Home Wi-Fi is back.") : "");
                m = n;
                dirty = 1;
            }
        }
        if (status_tick(&m)) dirty = 1;
    }
    if (g_pipe >= 0) close(g_pipe);
    for (i = 0; i < ndev; i++) close(dev[i].fd);
    display_close();
    free(g_buf[0]);
    free(g_buf[1]);
    px_sound_close(g_stop ? 0 : 400);
    px_fonts_free();
    return g_exit;
}
