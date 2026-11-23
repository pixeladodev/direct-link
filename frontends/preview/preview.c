/* Preview: Direct Link's panels as PNG files, on a computer (build/preview.sh). Both panels in one picture, top
   over bottom, for each panel size (RG DS 640x480, RG DS Plus 1024x768) and theme. Nothing here runs on a handheld. */
#include "pixel_png.h"
#include "view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Model base(void)
{
    Model m;
    memset(&m, 0, sizeof m);
    snprintf(m.state, sizeof m.state, "down");
    snprintf(m.home, sizeof m.home, "on");
    snprintf(m.version, sizeof m.version, "0.1.0");
    m.radio = 1;
    m.pick = PICK_NONE;
    m.battery = 64;
    snprintf(m.clock, sizeof m.clock, "14:30");
    m.upd.state = UPD_UPTODATE;
    snprintf(m.upd.current, sizeof m.upd.current, "0.1.0");
    snprintf(m.upd.latest, sizeof m.upd.latest, "0.1.0");
    return m;
}

static Model linked(void)
{
    Model m = base();
    snprintf(m.state, sizeof m.state, "up");
    snprintf(m.role, sizeof m.role, "host");
    snprintf(m.ip, sizeof m.ip, "10.42.0.1");
    snprintf(m.peer, sizeof m.peer, "10.42.0.2");
    snprintf(m.peer_ips, sizeof m.peer_ips, "10.42.0.2");
    snprintf(m.phase, sizeof m.phase, "up");
    m.linked = 1;
    m.peers = 1;
    return m;
}

static int shot(const char *dir, const char *name, int w, int h, const char *theme, const char *accent,
                const char *lang, const Model *m)
{
    PxSettings st;
    PxCanvas top, bot;
    int gap = h / 24, ok, i;
    uint32_t *buf = calloc((size_t)w * (size_t)(h * 2 + gap), sizeof *buf);
    char path[512];
    if (!buf) return 0;
    px_settings_read("/nonexistent", &st);
    snprintf(st.theme, sizeof st.theme, "%s", theme);
    snprintf(st.accent, sizeof st.accent, "%s", accent);
    (void)accent;
    px_pal_pixelado(&px);   /* (the app's own look: theme only names the file) */
    px_lang_set(lang, view_tr);
    px_lang_add(upd_tr);
    px_canvas(&top, buf, w, h);
    px_canvas(&bot, buf + (size_t)w * (size_t)(h + gap), w, h);
    for (i = 0; i < w * gap; i++) buf[(size_t)w * (size_t)h + (size_t)i] = 0x000000;
    view_top(&top, m);
    view_bottom(&bot, m);
    {
        const char *stamp = getenv("PREVIEW_STAMP");
        if (stamp && stamp[0])
            snprintf(path, sizeof path, "%s/%s-direct-link-%s-%dx%d-%s%s%s.png", dir, stamp, name, w, h,
                     strncmp(theme, "user:", 5) ? theme : theme + 5,
                     strcmp(lang, "en") ? "-" : "", strcmp(lang, "en") ? lang : "");
        else
            snprintf(path, sizeof path, "%s/direct-link-%s-%dx%d-%s%s%s.png", dir, name, w, h,
                     strncmp(theme, "user:", 5) ? theme : theme + 5,
                     strcmp(lang, "en") ? "-" : "", strcmp(lang, "en") ? lang : "");
    }
    ok = px_png_write(path, buf, w, h * 2 + gap);
    if (ok) printf("%s\n", path);
    free(buf);
    return ok;
}

int main(int argc, char **argv)
{
    const char *dir = argc > 1 ? argv[1] : ".";
    Model m;
    int ok = 1, s;
    static const int sizes[2][2] = { { 640, 480 }, { 1024, 768 } };
    if (!px_fonts_load(NULL)) { fprintf(stderr, "preview: set PIXEL_UI_FONTS to a folder with Pixelify Sans\n"); return 1; }
    for (s = 0; s < 2; s++) {
        /* opening the app: nothing picked; Host picked (Confirm lit); all in English, the README tour uses them */
        m = base();
        ok &= shot(dir, "01-open", sizes[s][0], sizes[s][1], "night", "", "en", &m);
        m.pick = PICK_HOST;
        ok &= shot(dir, "02-host-picked", sizes[s][0], sizes[s][1], "night", "", "en", &m);
    }
    m = base();
    m.pick = PICK_JOIN;
    ok &= shot(dir, "03-join-picked", 640, 480, "night", "", "en", &m);
    /* starting, with Cancel; joining; cancelling */
    m = base();
    m.pick = PICK_HOST;
    m.job = JOB_HOST;
    m.dots = 2;
    ok &= shot(dir, "04-starting", 640, 480, "night", "", "en", &m);
    m = base();
    m.pick = PICK_JOIN;
    m.role_sel = 1;
    m.job = JOB_JOIN;
    m.dots = 1;
    ok &= shot(dir, "05-joining", 640, 480, "night", "", "en", &m);
    m.job = JOB_NONE;
    m.canceling = 1;
    ok &= shot(dir, "06-cancelling", 640, 480, "night", "", "en", &m);
    /* the host: up, nobody yet; then one console */
    m = linked();
    m.peers = 0;
    m.peer_ips[0] = 0;
    m.peer[0] = 0;
    m.dots = 2;
    ok &= shot(dir, "07-host-waiting", 640, 480, "night", "", "en", &m);
    m = linked();
    ok &= shot(dir, "08-host-connected", 640, 480, "night", "", "en", &m);
    ok &= shot(dir, "08-host-connected", 1024, 768, "night", "", "en", &m);
    /* a guest, connected; then its host gone quiet */
    m = linked();
    snprintf(m.role, sizeof m.role, "join");
    snprintf(m.ip, sizeof m.ip, "10.42.0.2");
    snprintf(m.peer_ips, sizeof m.peer_ips, "10.42.0.1");
    m.role_sel = 1;
    ok &= shot(dir, "09-guest-connected", 640, 480, "night", "", "en", &m);
    /* a guest up but not hearing the host yet (wifi-link's phase "waiting": peers=0) */
    m.peers = 0;
    m.peer_ips[0] = 0;
    m.dots = 2;
    ok &= shot(dir, "09-guest-looking", 640, 480, "night", "", "en", &m);
    m.peers = 1;
    snprintf(m.peer_ips, sizeof m.peer_ips, "10.42.0.1");
    m.lost = 1;
    m.dots = 1;
    ok &= shot(dir, "10-host-lost", 640, 480, "night", "", "en", &m);
    m = linked();
    m.view = V_DISC;
    ok &= shot(dir, "11-disconnect", 640, 480, "night", "", "en", &m);
    m = base();
    m.view = V_SETTINGS;
    m.cursor = SET_UPDATES;
    ok &= shot(dir, "12-settings", 640, 480, "night", "", "en", &m);
    m.view = V_ABOUT;
    ok &= shot(dir, "13-about", 640, 480, "night", "", "en", &m);
    px_fonts_free();
    return ok ? 0 : 1;
}
