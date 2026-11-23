/* Direct Link's two panels, drawn from a model (main.c fills it on the handheld, preview.c on a computer). */
#ifndef VIEW_H
#define VIEW_H

#include "pixel.h"
#include "updater.h"

enum { V_MAIN, V_SETTINGS, V_UNINSTALL, V_DISC, V_ABOUT, V_UPDATE, V_UPDATED };
/* the main screen's two choices (Model.pick; -1: none picked yet) */
enum { PICK_NONE = -1, PICK_HOST = 0, PICK_JOIN = 1 };
enum { SET_UPDATES, SET_ABOUT, SET_COUNT };
/* how the two consoles are linked: only Wi-Fi works today */
enum { MODE_WIFI, MODE_ADAPTER, MODE_USB, MODE_COUNT };
enum { JOB_NONE, JOB_HOST, JOB_JOIN, JOB_DOWN };
/* what a touch on the bottom panel does */
enum { HIT_NONE, HIT_ROW, HIT_MODE, HIT_ROLE, HIT_BACK, HIT_OK, HIT_Y, HIT_MENU, HIT_PICK, HIT_CANCEL, HIT_X };

typedef struct {
    char state[32], role[16], ip[32], home[8], version[32];
    char phase[16];              /* wifi-link's phase: up, joining, starting, lost (a guest that can't hear the host) */
    char peer[48];               /* another console on the link, when it is known */
    int peers;                   /* consoles that answer on the link (the host's count; a guest: 1 while it hears the host) */
    char peer_ips[96];           /* the host: their addresses, comma-separated (wifi-link checks them every 5 s) */
    int lost;                    /* a guest whose host stopped answering (wifi-link looks for it again) */
    int radio;                   /* wlan0 is there */
    int linked;
    int job, dots;               /* the script running, and its "..." (0-2) */
    int mode, role_sel;          /* the mode (only Wi-Fi today, not shown); role_sel: the role being started */
    int pick;                    /* the main screen's choice: PICK_NONE, PICK_HOST, PICK_JOIN */
    int canceling;               /* a start or a join being cancelled */
    int view, cursor;
    PxUpdate upd;                /* the Updates row: update.py's last word */
    char err[240];
    int battery, charging;
    char clock[16];
    int beat;                    /* the characters' idle beat (0..CAST_BEATS-1; main.c advances it) */
} Model;

typedef struct { PxRect r; int id, arg; } Hit;

#define MAX_HITS 16
extern Hit g_hits[MAX_HITS];
extern int g_nhits;

extern const PxTr view_tr[];
void view_top(PxCanvas *c, const Model *m);
void view_bottom(PxCanvas *c, const Model *m);
int view_mode_works(int mode);
int view_can_connect(const Model *m);
/* the link up and another console answering on it */
int view_connected(const Model *m);
/* which characters the screens show now, as a bit per CAST_* (main.c redraws when one of them changes pose) */
unsigned view_cast(const Model *m);

#endif
