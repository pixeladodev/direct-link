/* Direct Link: the two panels in Pixelado's design system (pixel-ui, px_pal_pixelado).
   Top: the link's state, a drawing of the two consoles, which consoles are on it, and what to do next.
   Bottom: two big choices, Host and Join, side by side (docs/UX.md: how picking and confirming work); while a link
   starts, the one picked with Cancel; once up, the link's card with Disconnect. Only Wi-Fi links today: the mode is
   not shown. */
#include "view.h"
#include "cast.h"

#include <stdio.h>
#include <string.h>

Hit g_hits[MAX_HITS];
int g_nhits;

/* Spanish: neutral, and the words BananaOS's own translation uses (Conectar, Desconectar, Unirse, Consola...) */
const PxTr view_tr[] = {
    { "Mode", "Modo" },
    { "Role", "Rol" },
    { "Status", "Estado" },
    { "This console", "Esta consola" },
    { "Other console", "Otra consola" },
    { "Adapter", "Adaptador" },
    { "USB-C cable", "Cable USB-C" },
    { "Host", "Anfitrión" },
    { "Join", "Unirse" },
    { "Guest", "Invitado" },
    { "Searching", "Buscando" },
    { "Host lost", "Sin anfitrión" },
    { "The host stopped answering. Looking for it again",
      "El anfitrión dejó de responder. Buscándolo de nuevo" },
    { "If it doesn't come back in a minute, the link closes and home Wi-Fi returns.",
      "Si no vuelve en un minuto, el enlace se cierra y vuelve el Wi-Fi de casa." },
    { "If it doesn't come back in a minute, the link closes.",
      "Si no vuelve en un minuto, el enlace se cierra." },
    { "The host closed the link.", "El anfitrión cerró el enlace." },
    { "Home Wi-Fi is back.", "Volvió el Wi-Fi de casa." },
    { "About", "Acerca de" },
    { "Made by", "Hecha por" },
    { "Source code", "Código fuente" },
    { "Credits", "Créditos" },
    { "Two consoles play together over their own Wi-Fi, no router needed. Pixelado Party and other apps with LAN play find the host at 10.42.0.1.",
      "Dos consolas juegan juntas por su propio Wi-Fi, sin router. Pixelado Party y otras apps con juego en red encuentran al anfitrión en 10.42.0.1." },
    { "BananaOS and BananaLinux by JorreFog and the Banana contributors (MIT); wifi-link is adapted from BananaLinux. Pixelify Sans (OFL).",
      "BananaOS y BananaLinux, de JorreFog y los colaboradores de Banana (MIT); wifi-link está adaptado de BananaLinux. Pixelify Sans (OFL)." },
    { "An independent app, not part of BananaOS. Built and tested on the RG DS and the RG DS Plus. Source available (PolyForm Strict).",
      "Una app independiente, no es parte de BananaOS. Hecha y probada en la RG DS y la RG DS Plus. Código disponible (PolyForm Strict)." },
    { "Off", "Apagado" },
    { "Exit", "Salir" },
    { "Connect", "Conectar" },
    { "Disconnect", "Desconectar" },
    { "Uninstall", "Desinstalar" },
    { "Cancel", "Cancelar" },
    { "Back", "Atrás" },
    { "Waiting", "Esperando" },
    { "Coming soon", "Próximamente" },
    { "Updates wait until the link is off.", "Las actualizaciones esperan a que el enlace esté apagado." },
    { "Needs BananaOS support", "Requiere soporte de BananaOS" },
    { "Starting", "Iniciando" },
    { "Joining", "Uniéndose" },
    { "Closing", "Cerrando" },
    { "Linked", "Enlazado" },
    { "No Wi-Fi radio.", "No hay radio Wi-Fi." },
    { "Don't change Wi-Fi in Settings.", "No cambies el Wi-Fi en Ajustes." },
    { "Open Pixelado Party or another app with LAN play: the host is 10.42.0.1.",
      "Abre Pixelado Party u otra app con juego en red: el anfitrión es 10.42.0.1." },
    { "Home Wi-Fi will disconnect.", "El Wi-Fi de casa se desconectará." },
    { "Starting the link", "Iniciando el enlace" },
    { "Joining the link", "Uniéndose al enlace" },
    { "Restoring Wi-Fi", "Restaurando el Wi-Fi" },
    { "Closing the link", "Cerrando el enlace" },
    { "Two consoles on their own Wi-Fi, no router. Host on one, Guest on the other.",
      "Dos consolas en su propio Wi-Fi, sin router. Anfitrión en una, Invitado en la otra." },
    { "Coming soon: a USB-C Ethernet adapter on each console, joined by a network cable.",
      "Próximamente: un adaptador USB-C a Ethernet en cada consola, unidos por un cable de red." },
    { "Needs a BananaOS with USB link: the OTG port of one console to the DC/USB port of the other.",
      "Requiere un BananaOS con enlace USB: el puerto OTG de una consola al puerto DC/USB de la otra." },
    { "The console on the OTG side powers the other.", "La consola del lado OTG le da energía a la otra." },
    { "network cable", "cable de red" },
    { "Disconnect?", "¿Desconectar?" },
    { "Home Wi-Fi comes back.", "Vuelve el Wi-Fi de casa." },
    { "Settings", "Ajustes" },
    { "Updates", "Actualizaciones" },
    { "Open", "Abrir" },
    { "connected", "conectadas" },
    { "connected (1)", "1 conectada" },
    { "%d connected", "%d conectadas" },
    { "Delete this app?", "¿Borrar esta app?" },
    { "Start the link on this console", "Inicia el enlace en esta consola" },
    { "Join the other console's link", "Únete al enlace de la otra consola" },
    { "Confirm", "Confirmar" },
    { "Cancelling", "Cancelando" },
    { "Connected", "Conectado" },
    { "Waiting", "Esperando" },
    { "Waiting for the other console", "Esperando a la otra consola" },
    { "On the other console: Direct Link, then Join.", "En la otra consola: Direct Link y luego Unirse." },
    { "1 console connected", "1 consola conectada" },
    { "%d consoles connected", "%d consolas conectadas" },
    { "Connected to the host", "Conectado al anfitrión" },
    { "Looking for the host's link", "Buscando el enlace del anfitrión" },
    { "Looking for the host", "Buscando al anfitrión" },
    { "On the other console: Direct Link, then Host.", "En la otra consola: Direct Link y luego Crear." },
    { "Pick Host on one console and Join on the other.", "Elige Anfitrión en una consola y Unirse en la otra." },
    { "Starting the link: you can cancel it.", "Iniciando el enlace: puedes cancelarlo." },
    { "Joining: you can cancel it.", "Uniéndose: puedes cancelarlo." },
    { "Other consoles", "Otras consolas" },
    { "You are the host", "Eres el anfitrión" },
    { "You are a guest", "Eres invitado" },
    { "This deletes Direct Link.", "Esto borra Direct Link." },
    { NULL, NULL }
};

#define T(s) px_tr(s)

int view_mode_works(int mode)
{
    return mode == MODE_WIFI;
}

int view_can_connect(const Model *m)
{
    return m->view == V_MAIN && m->pick != PICK_NONE && !m->linked && m->job == JOB_NONE && m->radio
           && view_mode_works(m->mode);
}

int view_connected(const Model *m)
{
    /* (a guest: wifi-link says peers=1 only while the host answers; before that its phase is "waiting") */
    if (!m->linked || m->lost) return 0;
    return m->peers > 0;
}

/* the top panel's hint while there is no link (also while one starts, joins or is cancelled): Nilo and Lumi beside it */
static int top_idle(const Model *m)
{
    return !m->linked && m->radio && m->job != JOB_DOWN && !(m->view == V_SETTINGS && m->cursor == SET_UPDATES);
}

/* on the link's card: this console's character, and the other one's once it answers */
static void card_cast(const Model *m, int *host, int *guest)
{
    int join = !strcmp(m->role, "join"), on = view_connected(m);
    *host = !join || on;
    *guest = join || on;
}

unsigned view_cast(const Model *m)
{
    unsigned u = 0;
    if (m->view == V_ABOUT) return 1u << CAST_PIXO_ABOUT;
    if (top_idle(m)) u |= 1u << CAST_NILO | 1u << CAST_LUMI;
    if (m->view == V_MAIN && m->linked) {
        int h, g;
        card_cast(m, &h, &g);
        if (h) u |= 1u << CAST_NILO;
        if (g) u |= 1u << CAST_LUMI;
    }
    return u;
}

/* the logo's spark: a small cyan diamond with a white core, `u` units a pixel */
static void spark(PxCanvas *c, float x, float y, float u)
{
    px_rect(c, x + u, y, 2 * u, 4 * u, 0x45D4FF);
    px_rect(c, x, y + u, 4 * u, 2 * u, 0x45D4FF);
    px_rect(c, x + u, y + u, 2 * u, 2 * u, 0xEAF0FF);
}

static void add_hit(const PxCanvas *c, float x, float y, float w, float h, int id, int arg)
{
    if (g_nhits >= MAX_HITS) return;
    g_hits[g_nhits].r.x = px_u(c, x);
    g_hits[g_nhits].r.y = px_u(c, y);
    g_hits[g_nhits].r.w = px_u(c, x + w) - g_hits[g_nhits].r.x;
    g_hits[g_nhits].r.h = px_u(c, y + h) - g_hits[g_nhits].r.y;
    g_hits[g_nhits].id = id;
    g_hits[g_nhits].arg = arg;
    g_nhits++;
}

static void add_face_hit(PxRect r, int id)
{
    if (g_nhits >= MAX_HITS) return;
    g_hits[g_nhits].r = r;
    g_hits[g_nhits].id = id;
    g_hits[g_nhits].arg = 0;
    g_nhits++;
}

static int ndots(const Model *m)
{
    return m->dots + 1;
}

/* the status, in a word or two; returns the dots it ends in (-1: none) */
static int status_word(const Model *m, char *buf, size_t n)
{
    if (m->canceling) { snprintf(buf, n, "%s", T("Cancelling")); return ndots(m); }
    if (m->job == JOB_HOST) { snprintf(buf, n, "%s", T("Starting")); return ndots(m); }
    if (m->job == JOB_JOIN) { snprintf(buf, n, "%s", T("Joining")); return ndots(m); }
    if (m->job == JOB_DOWN) { snprintf(buf, n, "%s", T("Closing")); return ndots(m); }
    if (!m->linked) { snprintf(buf, n, "%s", T("Off")); return -1; }
    if (m->lost) { snprintf(buf, n, "%s", T("Searching")); return ndots(m); }
    if (!view_connected(m)) { snprintf(buf, n, "%s", T("Waiting")); return ndots(m); }
    snprintf(buf, n, "%s", T("Connected"));
    return -1;
}

/* ---- the drawings ---------------------------------------------------------------------------------------- */

/* speaker grille beside a screen: one hole, then two, then one, each row centered (from Pixelado Party) */
static void speaker(PxCanvas *c, float x, float y, float s, uint32_t hole)
{
    float d = 3 * s, gap = 2 * s, row = 5 * s, span = d * 2 + gap, mid = (span - d) / 2;
    px_rect(c, x + mid, y, d, d, hole);
    px_rect(c, x, y + row, d, d, hole);
    px_rect(c, x + d + gap, y + row, d, d, hole);
    px_rect(c, x + mid, y + row * 2, d, d, hole);
}

/* an open dual-screen handheld, Pixelado Party's drawing in the design system's colours: both screens 4:3, the top one
   between the speaker grilles, the pad and the buttons beside the bottom one. w: the body's width (64 at scale 1);
   about 1.2 w tall. lit: the screens on (the accent). on_plate: drawn on the accent plate (a picked choice) */
static void handheld(PxCanvas *c, float x, float y, float w, int lit, int on_plate)
{
    float s = w / 64.0f;
    uint32_t body = on_plate ? px.pick_low : px.panel_body;
    uint32_t rim = on_plate ? px.on_pick : lit ? px.accent.base : px.edge;
    uint32_t glass = on_plate ? px.pick_band : lit ? px_mix(px.accent.dark, px.accent.base, 0.35f) : px.screen_bg;
    uint32_t hole = on_plate ? px.pick : px.screen_bg, keys = on_plate ? px.pick_band : px.muted;
    px_round(c, x - 2 * s, y - 2 * s, 68 * s, 38 * s, 6 * s, rim, 255);
    px_round(c, x, y, 64 * s, 34 * s, 5 * s, body, 255);
    px_rect(c, x + 16 * s, y + 5 * s, 32 * s, 24 * s, glass);
    if (lit && !on_plate) px_rect(c, x + 16 * s, y + 5 * s, 32 * s, 3 * s, px.accent.light);
    speaker(c, x + 4 * s, y + 9 * s, s, hole);
    speaker(c, x + 52 * s, y + 9 * s, s, hole);
    px_rect(c, x + 6 * s, y + 36 * s, 52 * s, 4 * s, rim);
    px_round(c, x - 2 * s, y + 40 * s, 68 * s, 36 * s, 6 * s, rim, 255);
    px_round(c, x, y + 42 * s, 64 * s, 32 * s, 5 * s, body, 255);
    px_rect(c, x + 16 * s, y + 46 * s, 32 * s, 24 * s, glass);
    px_rect(c, x + 4 * s, y + 57 * s, 10 * s, 3 * s, keys);
    px_rect(c, x + 7 * s, y + 54 * s, 3 * s, 9 * s, keys);
    px_rect(c, x + 54 * s, y + 54 * s, 4 * s, 4 * s, keys);
    px_rect(c, x + 59 * s, y + 59 * s, 4 * s, 4 * s, keys);
    px_rect(c, x + 49 * s, y + 59 * s, 4 * s, 4 * s, keys);
    px_rect(c, x + 54 * s, y + 64 * s, 4 * s, 4 * s, keys);
}

/* a short dashed line between two points on a row */
static void dashed(PxCanvas *c, float x0, float x1, float y, uint32_t rgb)
{
    float x;
    for (x = x0; x < x1; x += 12) px_rect(c, x, y, x + 7 > x1 ? x1 - x : 7, 3, rgb);
}

/* Wi-Fi: three arcs from each console, drawn as pixel steps */
static void waves(PxCanvas *c, float x, float y, int dir, uint32_t rgb)
{
    int k, i;
    for (k = 0; k < 3; k++) {
        float r = 10 + k * 10;
        for (i = -3; i <= 3; i++) {
            float dy = i * r / 4.0f, dx = r - (i * i) * r / 24.0f;
            px_rect(c, x + dir * dx - 2, y + dy - 2, 4, 4, rgb);
        }
    }
}

static void tag(PxCanvas *c, float cx, float y, const char *s, int active)
{
    float w = px_text_w(c, PX_MEDIUM, 13, s, PX_UPPER | PX_TRACK) + 18;
    px_chip(c, cx - w / 2, y, w, 24, active);
    px_text(c, PX_MEDIUM, 13, cx - w / 2 + 9, y + 5, s, active ? px_ink_on(px.accent.base) : px.ink,
            PX_UPPER | PX_TRACK);
}

static void diagram(PxCanvas *c, const Model *m)
{
    const float y = 182, lx = 90, rx = 430;   /* (centred between the rules at 156 and 380) */
    int live = m->linked, both = view_connected(m);
    uint32_t wire = both ? px.accent.base : live || m->job != JOB_NONE ? px.edge : px.line;
    handheld(c, lx + 12, y - 2, 96, live && !(m->lost && !strcmp(m->role, "join")), 0);
    handheld(c, rx + 12, y - 2, 96, both, 0);
    if (m->mode == MODE_WIFI) {
        waves(c, lx + 128, y + 55, 1, wire);
        waves(c, rx - 8, y + 55, -1, wire);
        /* (with the link up the addresses are written under each console instead) */
        if (!live) {
            /* (this console is the host unless Join is picked or running: then it takes one of 10.42.0.2-9) */
            int guest = m->pick == PICK_JOIN || m->job == JOB_JOIN;
            tag(c, lx + 60, y + 118, guest ? "10.42.0.2-9" : "10.42.0.1", 0);
            tag(c, rx + 60, y + 118, guest ? "10.42.0.1" : "10.42.0.2-9", 0);
        }
    } else if (m->mode == MODE_ADAPTER) {
        /* each console: a USB-C to Ethernet adapter; a network cable between them */
        px_rect(c, lx + 123, y + 76, 14, 6, px.muted);
        px_round(c, lx + 137, y + 66, 30, 26, 4, px.muted, 255);
        px_rect(c, lx + 143, y + 72, 18, 14, px.screen_bg);
        px_rect(c, rx - 17, y + 76, 14, 6, px.muted);
        px_round(c, rx - 47, y + 66, 30, 26, 4, px.muted, 255);
        px_rect(c, rx - 41, y + 72, 18, 14, px.screen_bg);
        dashed(c, lx + 170, rx - 50, y + 78, wire);
        px_text_fit(c, PX_REGULAR, 13, 320, y + 92, T("network cable"), px.muted, PX_UPPER | PX_TRACK, 200, PX_CENTER);
        tag(c, lx + 60, y + 118, "USB-C > LAN", 0);
        tag(c, rx + 60, y + 118, "USB-C > LAN", 0);
    } else {
        /* the OTG port of one console to the DC/USB port of the other; the OTG side gives the power */
        px_rect(c, lx + 123, y + 76, 10, 6, px.muted);
        px_rect(c, rx - 13, y + 76, 10, 6, px.muted);
        dashed(c, lx + 136, rx - 16, y + 78, wire);
        /* a bolt over the cable: the power goes from left to right */
        px_rect(c, 314, y + 56, 8, 6, px.accent.base);
        px_rect(c, 310, y + 62, 14, 4, px.accent.base);
        px_rect(c, 316, y + 66, 6, 8, px.accent.base);
        px_triangle(c, rx - 30, y + 64, 10, 1, px.muted);
        tag(c, lx + 60, y + 118, "OTG", 0);
        tag(c, rx + 60, y + 118, "DC/USB", 0);
    }
}

static void stat_centered(PxCanvas *c, float cx, float y, float w, const char *label, const char *value, uint32_t rgb)
{
    const char *v = value && value[0] && value[0] != ' ' ? value : "";
    float lw;
    px_text_fit(c, PX_MEDIUM, 22, cx, y, v, rgb, 0, w, PX_CENTER);
    lw = px_text_w(c, PX_MEDIUM, 13, label, 0);
    px_text(c, PX_MEDIUM, 13, cx - lw * 0.5f, y + 28, label, px.muted, 0);
}

void view_top(PxCanvas *c, const Model *m)
{
    char buf[320], other[96];
    const char *lines[4];
    uint32_t rgb[4];
    int n = 0, i, d, kind;
    float y, bh = 46;
    px_screen(c);
    px_status(c, "Direct Link", m->battery, m->charging, m->clock);
    px_sheet(c, 18, 60, 604, 406);
    /* the state, big: green connected, the accent on its way (or waiting for the other console), grey off */
    px_label(c, 44, 84, T("Status"), px.muted);
    d = status_word(m, buf, sizeof buf);
    kind = m->job != JOB_NONE || m->canceling || m->lost || (m->linked && !view_connected(m)) ? PX_BADGE_PRIMARY
           : view_connected(m) ? PX_BADGE_SUCCESS : PX_BADGE_NEUTRAL;
    px_badge_dots(c, 42, 104, bh, buf, d, kind);
    if (m->linked)
        px_text_fit(c, PX_MEDIUM, 19, 598, 118, !strcmp(m->role, "join") ? T("You are a guest") : T("You are the host"),
                    px.muted, 0, 260, PX_RIGHT);
    px_line_h(c, 42, 156, 556, 2, px.edge, 70);
    diagram(c, m);
    /* who is on the link: this console's address, the others' */
    other[0] = 0;
    if (view_connected(m)) {
        if (!strcmp(m->role, "join")) snprintf(other, sizeof other, "%s", "10.42.0.1");
        else snprintf(other, sizeof other, "%s", m->peer_ips[0] ? m->peer_ips : m->peer);
    }
    stat_centered(c, 150, 314, 132, T("This console"), m->ip[0] ? m->ip : "", m->ip[0] ? px.ink : px.muted);
    stat_centered(c, 490, 314, 200, m->peers > 1 ? T("Other consoles") : T("Other console"), other,
                  other[0] ? px.ink : px.muted);
    px_line_h(c, 42, 380, 556, 2, px.edge, 70);

    /* what to do next */
    if (m->view == V_SETTINGS && m->cursor == SET_UPDATES && m->job == JOB_NONE && !m->lost) {
        int quiet = m->linked && (m->upd.state == UPD_OFFLINE || m->upd.state == UPD_FAILED || m->upd.state == UPD_IDLE);
        if (!quiet) {
            upd_help(&m->upd, "Direct Link", buf, sizeof buf);
            lines[n] = buf;
            rgb[n++] = px.ink;
        }
        if (m->linked && !upd_running(&m->upd)) {
            lines[n] = T("Updates wait until the link is off.");
            rgb[n++] = px.muted;
        }
    } else if (m->canceling) {
        snprintf(buf, sizeof buf, "%s%.*s", !strcmp(m->home, "on") ? T("Restoring Wi-Fi") : T("Closing the link"), ndots(m), "...");
        lines[n] = buf; rgb[n++] = px.ink;
    } else if (m->job == JOB_HOST || m->job == JOB_JOIN) {
        lines[n] = m->job == JOB_HOST ? T("Starting the link: you can cancel it.") : T("Joining: you can cancel it.");
        rgb[n++] = px.ink;
        if (!strcmp(m->home, "on")) { lines[n] = T("Home Wi-Fi will disconnect."); rgb[n++] = px.muted; }
    } else if (m->job == JOB_DOWN) {
        snprintf(buf, sizeof buf, "%s%.*s", !strcmp(m->home, "on") ? T("Restoring Wi-Fi") : T("Closing the link"), ndots(m), "...");
        lines[n] = buf;
        rgb[n++] = px.ink;
    } else if (m->linked && m->lost) {
        snprintf(buf, sizeof buf, "%s%.*s", T("The host stopped answering. Looking for it again"), ndots(m), "...");
        lines[n] = buf; rgb[n++] = px.ink;
        lines[n] = !strcmp(m->home, "on") ? T("If it doesn't come back in a minute, the link closes and home Wi-Fi returns.")
                                          : T("If it doesn't come back in a minute, the link closes.");
        rgb[n++] = px.muted;
    } else if (m->linked && !view_connected(m)) {
        lines[n] = !strcmp(m->role, "join") ? T("On the other console: Direct Link, then Host.")
                                             : T("On the other console: Direct Link, then Join.");
        rgb[n++] = px.ink;
        lines[n] = T("Don't change Wi-Fi in Settings."); rgb[n++] = px.muted;
    } else if (m->linked) {
        lines[n] = T("Open Pixelado Party or another app with LAN play: the host is 10.42.0.1."); rgb[n++] = px.ink;
        lines[n] = T("Don't change Wi-Fi in Settings."); rgb[n++] = px.muted;
    } else if (!m->radio) {
        lines[n] = T("No Wi-Fi radio."); rgb[n++] = px.warn;
    } else {
        lines[n] = T("Pick Host on one console and Join on the other."); rgb[n++] = px.ink;
    }
    if (m->err[0] && m->job == JOB_NONE && !m->canceling && n < 4) { lines[n] = m->err; rgb[n++] = m->linked ? px.warn : px.ink; }
    y = 392;
    for (i = 0; i < n && y < 440; i++)
        y += 24 * px_text_wrap(c, PX_REGULAR, 18, 42, y, lines[i], rgb[i], top_idle(m) ? 440 : 556, 2, 24) + 4;
    if (top_idle(m)) {
        /* Nilo and Lumi, waiting for someone to pick */
        cast_draw(c, CAST_NILO, 500, 404, 2, m->beat);
        cast_draw(c, CAST_LUMI, 552, 404, 2, m->beat);
    }
}

/* one of the two big choices on the main screen: Host (left) or Join (right). Picked: the accent plate; while its
   link starts: its state with the dots; the other one, while a link starts or when there is no radio: greyed */
static void choice(PxCanvas *c, const Model *m, int which, float x, float y, float w, float h)
{
    int busy = m->job == JOB_HOST || m->job == JOB_JOIN || m->canceling;
    int mine = busy && (which == PICK_JOIN) == (m->job == JOB_JOIN || (m->canceling && m->role_sel));
    int sel = m->pick == which && (!busy || mine), dis = !m->radio || (busy && !mine);
    uint32_t ink = sel ? px.on_pick : dis ? px.dim : px.ink, sub = sel ? px.on_pick_sub : dis ? px.dim : px.muted;
    const char *title = which == PICK_JOIN ? T("Join") : T("Host");
    const char *line = which == PICK_JOIN ? T("Join the other console's link") : T("Start the link on this console");
    float cx = x + w / 2, iy = y + 22;
    char buf[64];
    px_card(c, x, y, w, h, sel, dis);
    if (!busy) add_hit(c, x, y, w, h, HIT_PICK, which);
    /* the drawing: this console sending (Host) or a console with the link coming to it (Join) */
    handheld(c, which == PICK_HOST ? cx - 62 : cx - 34, iy, 96, 0, sel);
    if (which == PICK_HOST) waves(c, cx + 50, iy + 57, 1, sel ? px.on_pick : dis ? px.dim : px.accent.light);
    else waves(c, cx - 46, iy + 57, -1, sel ? px.on_pick : dis ? px.dim : px.accent.light);
    px_text_fit(c, PX_MEDIUM, 30, cx, y + 150, title, ink, 0, w - 32, PX_CENTER);
    if (mine) {
        int d = status_word(m, buf, sizeof buf);
        px_text_dots(c, PX_MEDIUM, 17, cx, y + 196, buf, d, sub, PX_UPPER | PX_TRACK, PX_CENTER);
    } else px_text_wrap(c, PX_REGULAR, 16, x + 20, y + 194, line, sub, w - 40, 2, 21);
}

/* the link's card on the main screen once it is up: who this console is, who answers, Disconnect below */
static void link_card(PxCanvas *c, const Model *m, float y)
{
    char buf[128];
    int on = view_connected(m);
    px_card(c, 18, y, 604, 320, 0, 0);
    px_label(c, 40, y + 22, !strcmp(m->role, "join") ? T("You are a guest") : T("You are the host"), px.muted);
    {
        /* Nilo hosts, Lumi joins: this console's one, and the other once it answers, with the spark between */
        int host, guest;
        card_cast(m, &host, &guest);
        if (host) cast_draw(c, CAST_NILO, 392, y + 18, 4, m->beat);
        if (guest) cast_draw(c, CAST_LUMI, 512, y + 18, 4, m->beat);
        if (on) spark(c, 486, y + 58, 5);
    }
    if (!strcmp(m->role, "join"))
        snprintf(buf, sizeof buf, "%s%.*s", m->lost ? T("Looking for the host's link") : on ? T("Connected to the host")
                                                                                         : T("Looking for the host"),
                 on ? 0 : ndots(m), "...");
    else if (m->peers <= 0) snprintf(buf, sizeof buf, "%s%.*s", T("Waiting for the other console"), ndots(m), "...");
    else if (m->peers == 1) snprintf(buf, sizeof buf, "%s", T("1 console connected"));
    else snprintf(buf, sizeof buf, T("%d consoles connected"), m->peers);
    /* (the state's badge is on the top panel; here the words, beside the characters) */
    px_text_wrap(c, PX_MEDIUM, 26, 40, y + 56, buf, on ? px.ink : px.muted, 336, 3, 32);
    if (m->ip[0]) {
        px_label(c, 40, y + 206, T("This console"), px.muted);
        px_text(c, PX_MEDIUM, 20, 40, y + 226, m->ip, px.ink, 0);
    }
    if (on) {
        px_label(c, 330, y + 206, m->peers > 1 ? T("Other consoles") : T("Other console"), px.muted);
        px_text_fit(c, PX_MEDIUM, 20, 330, y + 226, !strcmp(m->role, "join") ? "10.42.0.1" : (m->peer_ips[0] ? m->peer_ips : m->peer),
                    px.ink, 0, 270, PX_LEFT);
    }
}

static void set_row(PxCanvas *c, const Model *m, int i, float y, const char *title)
{
    int sel = m->cursor == i && m->view == V_SETTINGS, dis = 0;
    uint32_t ink = sel ? px.on_pick : px.ink, sub = sel ? px.on_pick_sub : px.muted;
    char buf[64];
    int d;
    (void)dis;
    px_card(c, 18, y, 588, 54, sel, 0);
    add_hit(c, 18, y, 588, 54, HIT_ROW, i);
    px_text_fit(c, PX_MEDIUM, 20, 38, y + 14, title, ink, 0, 320, PX_LEFT);
    if (i == SET_UPDATES) {
        uint32_t v = upd_lit(&m->upd) || upd_running(&m->upd) ? ink : sub;
        upd_value(&m->upd, buf, sizeof buf, &d, m->dots + 1);
        if (m->upd.state == UPD_DOWNLOADING) {
            char pct[8];
            int p = m->upd.pct < 0 ? 0 : m->upd.pct > 100 ? 100 : m->upd.pct;
            float pw = px_text_w(c, PX_MEDIUM, 20, "100%", 0);
            snprintf(pct, sizeof pct, "%d%%", p);
            px_text(c, PX_MEDIUM, 20, 584 - px_text_w(c, PX_MEDIUM, 20, pct, 0), y + 14, pct, v, 0);
            px_text_fit(c, PX_MEDIUM, 20, 584 - pw - 10, y + 14, T("Downloading"), v, 0, 240, PX_RIGHT);
            px_rect(c, 30, y + 46, 564, 3, sel ? px.on_pick_sub : px.line);
            px_rect(c, 30, y + 46, 564.0f * p / 100, 3, sel ? px.on_pick : px.accent.base);
        } else if (d >= 0) px_text_dots(c, PX_MEDIUM, 20, 584, y + 14, buf, d, v, 0, PX_RIGHT);
        else px_text_fit(c, PX_MEDIUM, 20, 584, y + 14, buf, v, 0, 300, PX_RIGHT);
    }
}

static void dialog(PxCanvas *c, const char *title, const char *sub, const char *mono)
{
    px_rect_a(c, 0, 0, 640, 480, 0x000000, px.light ? 70 : 150);
    px_card(c, 50, 120, 540, 200, 0, 0);
    px_text_wrap(c, PX_MEDIUM, 26, 76, 146, title, px.ink, 488, 2, 32);
    if (mono) px_text_fit(c, PX_REGULAR, 18, 76, 222, mono, px.muted, 0, 488, PX_LEFT);
    if (sub && sub[0]) px_text_wrap(c, PX_REGULAR, 18, 76, mono ? 252 : 222, sub, px.muted, 488, 3, 25);
}

/* SELECT: who made it, where its source is, what it is built on */
static void view_about(PxCanvas *c, const Model *m)
{
    char buf[96];
    PxRect r;
    float y;
    g_nhits = 0;
    px_screen(c);
    px_bubble(c, 18, 14, 604, 54, 0);
    px_text_fit(c, PX_MEDIUM, 26, 38, 25, T("About"), px.ink, 0, 440, PX_LEFT);
    px_card(c, 18, 82, 604, 326, 0, 0);
    px_text(c, PX_MEDIUM, 28, 40, 98, "Direct Link", px.ink, 0);
    snprintf(buf, sizeof buf, "v%s", m->version[0] ? m->version : "?");
    px_text_fit(c, PX_MEDIUM, 20, 50 + px_text_w(c, PX_MEDIUM, 28, "Direct Link", 0), 104, buf, px.muted, 0, 160, PX_LEFT);
    /* Pixo, Pixelado Dev's own character, in the corner */
    cast_draw(c, CAST_PIXO_ABOUT, 534, 96, 3, m->beat);
    y = 138;
    y += 25 * px_text_wrap(c, PX_REGULAR, 18, 40, y,
                           T("Two consoles play together over their own Wi-Fi, no router needed. Pixelado Party and other apps with LAN play find the host at 10.42.0.1."),
                           px.ink, 476, 3, 25) + 10;
    px_label(c, 40, y + 4, T("Made by"), px.muted);
    px_text(c, PX_MEDIUM, 18, 200, y, "Pixelado Dev", px.ink, 0);
    y += 26;
    px_label(c, 40, y + 4, T("Source code"), px.muted);
    px_text_fit(c, PX_MEDIUM, 18, 200, y, "github.com/pixeladodev/direct-link", px.ink, 0, 400, PX_LEFT);
    y += 32;
    px_label(c, 40, y, T("Credits"), px.muted);
    y += 20;
    y += 23 * px_text_wrap(c, PX_REGULAR, 16, 40, y,
                           T("BananaOS and BananaLinux by JorreFog and the Banana contributors (MIT); wifi-link is adapted from BananaLinux. Pixelify Sans (OFL)."),
                           px.muted, 560, 3, 23) + 6;
    px_text_wrap(c, PX_REGULAR, 16, 40, y,
                 T("An independent app, not part of BananaOS. Built and tested on the RG DS and the RG DS Plus. Source available (PolyForm Strict)."),
                 px.muted, 560, 3, 23);
    px_face(c, 18, 422, PX_LEFT, "B", T("Back"), 0, &r, PX_FACE_NEUTRAL);
    add_face_hit(r, HIT_BACK);
}

void view_bottom(PxCanvas *c, const Model *m)
{
    char count[16];
    PxRect r;
    int i, busy = m->job != JOB_NONE, rows;
    g_nhits = 0;
    px_screen(c);
    px_bubble(c, 18, 14, 604, 54, 0);
    if (m->view == V_SETTINGS) {
        px_text_fit(c, PX_MEDIUM, 26, 38, 25, T("Settings"), px.ink, 0, 440, PX_LEFT);
        rows = SET_COUNT;
    } else {
        px_text_fit(c, PX_MEDIUM, 26, 38, 25, "Direct Link", px.ink, 0, 440, PX_LEFT);
        rows = 0;
    }
    if (m->view == V_SETTINGS) {
        snprintf(count, sizeof count, "%02d/%02d", m->cursor + 1, rows);
        px_text(c, PX_MEDIUM, 19, 602 - px_text_w(c, PX_MEDIUM, 19, count, 0), 30, count, px.muted, 0);
        for (i = 0; i < SET_COUNT; i++) set_row(c, m, i, 84 + i * 66, i == SET_ABOUT ? T("About") : T("Updates"));
        px_rail(c, 615, 84, SET_COUNT * 66 - 12, m->cursor, 1, SET_COUNT, NULL, NULL);
    }

    if (m->view == V_SETTINGS) {
        PxRect r;
        float y = 422, bw;
        int inst = upd_installing(&m->upd);
        px_face(c, 18, y, PX_LEFT, "B", T("Back"), 0, &r, PX_FACE_NEUTRAL);
        add_face_hit(r, HIT_BACK);
        bw = px_face_w(c, "B", T("Back"));
        px_face(c, 18 + bw + 8, y, PX_LEFT, "Y", T("Uninstall"), busy || inst || m->linked, &r, PX_FACE_NEUTRAL);
        add_face_hit(r, HIT_Y);
        px_face(c, 622, y, PX_RIGHT, "A", m->cursor == SET_UPDATES ? upd_action(&m->upd) : T("Open"),
                  inst || (m->cursor == SET_UPDATES && (upd_running(&m->upd) || m->linked)), &r, PX_FACE_NEUTRAL);
        add_face_hit(r, HIT_OK);
        return;
    }
    if (m->view == V_MAIN) {
        PxRect r;
        float y = 422, bw;
        int inst = upd_installing(&m->upd);
        int starting = m->job == JOB_HOST || m->job == JOB_JOIN;
        if (m->linked && !starting && !m->canceling) link_card(c, m, 84);
        else {
            choice(c, m, PICK_HOST, 18, 84, 296, 320);
            choice(c, m, PICK_JOIN, 326, 84, 296, 320);
        }
        if (starting && !m->canceling) {
            /* B cancels the start (or the join): the link comes down and home Wi-Fi comes back */
            px_face(c, 18, y, PX_LEFT, "B", T("Cancel"), 0, &r, PX_FACE_NEUTRAL);
            add_face_hit(r, HIT_CANCEL);
            return;
        }
        px_face(c, 18, y, PX_LEFT, "B", T("Exit"), busy || inst || m->canceling, &r, PX_FACE_NEUTRAL);
        add_face_hit(r, HIT_BACK);
        bw = px_face_w(c, "B", T("Exit"));
        px_face(c, 18 + bw + 8, y, PX_LEFT, "START", T("Settings"), busy || m->canceling || m->linked, &r, PX_FACE_NEUTRAL);
        add_face_hit(r, HIT_MENU);
        if (m->linked) {
            px_face(c, 622, y, PX_RIGHT, "X", T("Disconnect"), busy || inst || m->canceling, &r, PX_FACE_DANGER);
            add_face_hit(r, HIT_X);
        } else {
            px_face(c, 622, y, PX_RIGHT, "A", T("Confirm"), !view_can_connect(m) || inst || m->canceling, &r, PX_FACE_NEUTRAL);
            add_face_hit(r, HIT_OK);
        }
        return;
    }
    if (m->view == V_ABOUT) {
        view_about(c, m);
        return;
    }
    g_nhits = 0;
    if (m->view == V_UPDATE || m->view == V_UPDATED) {
        char title[96], sub[320];
        int done = m->view == V_UPDATED;
        snprintf(title, sizeof title, done ? T("Updated to v%s") : T("Update to v%s?"),
                 done ? m->upd.current : m->upd.latest);
        snprintf(sub, sizeof sub, "%s%s%s",
                 done ? T("A opens the new version.")
                      : T("It downloads from GitHub and checks it before anything changes. Your settings stay."),
                 done && m->upd.msg[0] ? " " : "", done ? m->upd.msg : "");
        dialog(c, title, sub, NULL);
        if (!done) {
            px_face(c, 18, 422, PX_LEFT, "B", T("Cancel"), 0, &r, PX_FACE_NEUTRAL);
            add_face_hit(r, HIT_BACK);
        }
        px_face(c, 622, 422, PX_RIGHT, "A", done ? T("Restart") : T("Update"), 0, &r, PX_FACE_NEUTRAL);
        add_face_hit(r, HIT_OK);
    } else if (m->view == V_DISC) {
        dialog(c, T("Disconnect?"), !strcmp(m->home, "on") ? T("Home Wi-Fi comes back.") : "", NULL);
        px_face(c, 18, 422, PX_LEFT, "B", T("Cancel"), 0, &r, PX_FACE_NEUTRAL);
        add_face_hit(r, HIT_BACK);
        px_face(c, 622, 422, PX_RIGHT, "A", T("Disconnect"), 0, &r, PX_FACE_DANGER);
        add_face_hit(r, HIT_OK);
    } else if (m->view == V_UNINSTALL) {
        dialog(c, T("Delete this app?"), m->err[0] ? m->err : T("This deletes Direct Link."), NULL);
        px_face(c, 18, 422, PX_LEFT, "B", T("Cancel"), 0, &r, PX_FACE_NEUTRAL);
        add_face_hit(r, HIT_BACK);
        px_face(c, 622, 422, PX_RIGHT, "A", T("Uninstall"), 0, &r, PX_FACE_NEUTRAL);
        add_face_hit(r, HIT_OK);
    }
}
