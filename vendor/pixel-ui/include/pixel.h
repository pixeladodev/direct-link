/* pixel: a small drawing kit for BananaOS apps that draw straight into a 32-bit framebuffer.
   It draws in the menu's look (ROCKNIXDS Pixel as banana-ui has it): a dithered screen, a rounded sheet with a
   bright 3 px edge and light/dark bands, banded cards, the accent plate for the picked row, drawn key caps and
   Pixelify Sans with spaced capitals for labels.

   Everything is laid out on the menu's 640x480 grid ("units") and scaled to the panel: 1x on the RG DS
   (640x480), 1.6x on the RG DS Plus (1024x768).

   The palettes, the accent table and the shape recipes are ported from JorreFog/BananaLinux (MIT, Copyright (c)
   2026 JorreFog and the Banana contributors): ui/src/settings_pal.cpp, ui/rnds/RndsUI.cpp and
   store/apps/activity/DESIGN.md. This is Pixelado's shared pixel UI library (pixel-ui): apps vendor it. */
#ifndef PIXEL_H
#define PIXEL_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t *px;               /* XRGB8888, w * h, row after row */
    int w, h;
    float s;                    /* units -> pixels: h / 480 */
} PxCanvas;

typedef struct { uint32_t base, light, dark, band, low; } PxAccent;

typedef struct {
    int light;
    uint32_t ink, muted, line, edge, dim;
    uint32_t screen_bg, screen_frame, screen_band;
    uint32_t panel_band, panel_body, panel_low;
    uint32_t bubble_band, bubble_body, bubble_low;
    uint32_t chip_band, chip_body, rail_bg, rule, brand_bg;
    uint32_t kbd_bg, kbd_ink;
    uint32_t red, green, warn, sheet;   /* warn: red text that reads on the sheet */
    uint32_t fav_border, fav_band, fav_body, fav_ink; /* RndsUI face1 / Fav button */
    uint32_t pick, pick_band, pick_low, pick_edge, on_pick, on_pick_sub;
    uint32_t dither_rgb;
    int dither_a, dither_b;     /* alpha of the 4x4 tile's two dots, 0-255 */
    PxAccent accent;
} PxPal;

/* what /data/banana/settings.conf says (theme, accent, accent_text, language, clock, sounds) */
typedef struct {
    char theme[96], accent[16], accent_text[8], language[16], clock[8];
    char sounds[8];             /* the menu's sounds: "on" (the default) or "off" */
} PxSettings;

enum { PX_REGULAR = 0, PX_MEDIUM = 1 };
enum { PX_LEFT = 0, PX_CENTER = 1, PX_RIGHT = 2 };
/* text flags */
#define PX_UPPER 1               /* in capitals */
#define PX_TRACK 2               /* spaced (the micro labels' tracking) */

typedef struct { int x, y, w, h; } PxRect;   /* in pixels, for touch */

/* settings: path NULL is /data/banana/settings.conf (PIXEL_UI_SETTINGS overrides it); missing keys are the
   menu's defaults (dark, yellow, auto, English, 24h) */
void px_settings_read(const char *path, PxSettings *st);
/* the palette: settings.conf's theme (dark, light, or user:<name>: banana/themes/<name>.json, its base, accent and
   colours as banana-ui reads them) and accent */
void px_pal_make(PxPal *p, const PxSettings *st);
/* Pixelado's own look (tokens/tokens.json, theme "night"): the same whatever the menu's theme is; turns on px_pixel_corners */
void px_pal_pixelado(PxPal *p);
extern int px_pixel_corners;     /* corners cut in two pixel steps instead of curves */
extern PxPal px;                 /* the palette in use */
/* a menu sound the theme names for name (scroll, select, back, ready...), as banana-ui reads it: the theme's
   "sounds": { "<name>": "<file>" }, the file beside the theme's JSON or an absolute path. 1 with its path in out,
   -1 when the theme turns it off ("off"), 0 for the menu's own */
int px_theme_sound(const PxSettings *st, const char *name, char *out, size_t n);

/* held buttons repeat, as banana-ui's lists do: after 400 ms, every 120 ms, every 60 ms once held 1.5 s.
   px_repeat_key on each key event (value 1 down, 0 up; repeatable: this key repeats here), px_repeat_due once a
   loop (the key code when a repeat is due, else 0), px_repeat_wait for poll's timeout (-1: nothing held) */
typedef struct { int code; long since, last; } PxRepeat;
void px_repeat_key(PxRepeat *r, int code, int value, int repeatable, long now_ms);
int px_repeat_due(PxRepeat *r, long now_ms);
int px_repeat_wait(const PxRepeat *r, long now_ms);
/* L/R page repeat: same as repeat, gaps ~145 ms / 75 ms (slightly slower than the list step) */
int px_repeat_due_page(PxRepeat *r, long now_ms);
int px_repeat_wait_page(const PxRepeat *r, long now_ms);

/* fonts: Pixelify Sans Regular and Medium from dir (NULL: PIXEL_UI_FONTS, else /usr/share/banana/rnds/fonts) */
int px_fonts_load(const char *dir);
/* PIXEL_UI_FONTS, the system's, then own_dir (the app's own copy): 1 when one of them had the fonts */
int px_fonts_load_any(const char *own_dir);
void px_fonts_free(void);

/* translations: a table of {english, spanish} pairs ended by {NULL, NULL}; px_tr gives the line in the language
   settings.conf names ("es": Spanish), else the English text as it is */
typedef struct { const char *en, *es; } PxTr;
void px_lang_set(const char *code, const PxTr *table);
const char *px_tr(const char *en);
/* a second table, looked in after the first (a shared module's own lines) */
void px_lang_add(const PxTr *table);
int px_lang_is(const char *code);

/* the canvas */
void px_canvas(PxCanvas *c, uint32_t *buf, int w, int h);
int px_u(const PxCanvas *c, float units);    /* units -> pixels */

/* drawing (every position and size in units) */
void px_rect(PxCanvas *c, float x, float y, float w, float h, uint32_t rgb);
void px_rect_a(PxCanvas *c, float x, float y, float w, float h, uint32_t rgb, int a);
void px_round(PxCanvas *c, float x, float y, float w, float h, float r, uint32_t rgb, int a);
void px_line_h(PxCanvas *c, float x, float y, float w, float t, uint32_t rgb, int a);
void px_dither(PxCanvas *c, float x, float y, float w, float h, float r);
void px_triangle(PxCanvas *c, float cx, float cy, float size, int dir, uint32_t rgb); /* -1 left, 1 right, -2 up, 2 down */

/* text: size in units; returns the width in units */
float px_text(PxCanvas *c, int face, float size, float x, float y, const char *s, uint32_t rgb, int flags);
float px_text_w(const PxCanvas *c, int face, float size, const char *s, int flags);
/* text ending in an animated 0-3 dots, laid out as if all three were there (it doesn't move); align as px_text_fit */
float px_text_dots(PxCanvas *c, int face, float size, float x, float y, const char *s, int dots, uint32_t rgb,
                   int flags, int align);
/* text cut with "..." to max_w units */
float px_text_fit(PxCanvas *c, int face, float size, float x, float y, const char *s, uint32_t rgb, int flags,
                  float max_w, int align);
/* picked: BananaOS marquee (1500 ms pause, 36 u/s); unpicked: "..." with pad at the right edge of max_w */
float px_text_label(PxCanvas *c, int face, float size, float x, float y, const char *s, uint32_t rgb, int flags,
                    float max_w, int picked);
/* ms until the marquee needs another frame, or -1 (set during px_text_label) */
int px_marquee_wake_ms(void);
void px_marquee_reset(void);
void px_marquee_begin(void);
void px_marquee_preview(const char *s, double elapsed_ms);
int px_marquee_consume_seed(void);
/* wrapped text; returns the lines used */
int px_text_wrap(PxCanvas *c, int face, float size, float x, float y, const char *s, uint32_t rgb, float max_w,
                 int max_lines, float line_h);
/* the lines px_text_wrap would use (nothing is drawn) */
int px_text_lines(const PxCanvas *c, int face, float size, const char *s, float max_w, int max_lines, float line_h);
/* the cover shine BananaOS sweeps across box art. The first pass starts 1 s after
   px_cover_shine_reset (a new item); later passes start every 5 s and last 560 ms.
   Live: px_cover_shine only remembers the rectangle. px_cover_shine_paint draws it, so the
   caller can restore a clean cover between frames. A preview time (ms, >= 0) paints in
   px_cover_shine and holds the still. */
void px_cover_shine(PxCanvas *c, int x, int y, int w, int h);
void px_cover_shine_paint(PxCanvas *c);
void px_cover_shine_reset(void);
int px_cover_shine_armed(int *x, int *y, int *w, int *h);
int px_cover_shine_wake_ms(void);
void px_cover_shine_preview(double ms);

/* the menu's parts */
void px_screen(PxCanvas *c);                                     /* background: frame, top band, dither */
void px_sheet(PxCanvas *c, float x, float y, float w, float h);  /* the tinted hero panel */
/* a card (a list row): sel draws the accent plate; dis greys it */
void px_card(PxCanvas *c, float x, float y, float w, float h, int sel, int dis);
void px_bubble(PxCanvas *c, float x, float y, float w, float h, int pointer);
void px_chip(PxCanvas *c, float x, float y, float w, float h, int active);
/* a state badge: text, border and fill in one family. h units high.
   neutral grey, success green, warning amber, danger the alert red, info blue, primary the theme accent */
enum {
    PX_BADGE_NEUTRAL = 0,
    PX_BADGE_SUCCESS = 1,
    PX_BADGE_WARNING = 2,
    PX_BADGE_DANGER = 3,
    PX_BADGE_INFO = 4,
    PX_BADGE_PRIMARY = 5
};
float px_badge(PxCanvas *c, float x, float y, float h, const char *text, int kind);
float px_badge_w(const PxCanvas *c, float h, const char *text);
/* the same with an animated "..." after the word (0-3 dots), its width kept for all three */
float px_badge_dots(PxCanvas *c, float x, float y, float h, const char *text, int dots, int kind);
/* a key cap ("A", "B", "START"); returns its width */
float px_keycap(PxCanvas *c, float x, float y, const char *key);
float px_keycap_w(const PxCanvas *c, const char *key);
/* a face button: a key cap and its label, 40 high; align PX_LEFT/RIGHT/CENTER around x; dis greys it.
   Its area in pixels goes to *hit (may be NULL).
   neutral: the usual legend. primary: the theme accent, three tones (text, border, fill), as the Party button is.
   danger: the alert red, the same three tones (Exit, disconnect). */
enum { PX_FACE_NEUTRAL = 0, PX_FACE_PRIMARY = 1, PX_FACE_DANGER = 2 };
void px_face(PxCanvas *c, float x, float y, int align, const char *key, const char *label, int dis, PxRect *hit,
              int variant);
void px_fav_mark(PxCanvas *c, float x, float y, float s);
float px_face_w(const PxCanvas *c, const char *key, const char *label);
/* the face row along the bottom (y): the first `left` from the left edge, the rest against the right one, their
   labels made smaller (19 down to 15) until all of them fit; hits[i] (may be NULL) gets each one's area */
typedef struct { const char *key, *label; int dis; } PxFace;
void px_faces(PxCanvas *c, float y, int n, int left, const PxFace *f, PxRect *hits);
/* the scroll rail on the right: first shown, shown, total */
void px_rail(PxCanvas *c, float x, float y, float h, int first, int shown, int total, PxRect *track, PxRect *thumb);
/* a horizontal track (y): pos out of total-1; count text at the right; *hit is the draggable track in pixels */
void px_hrail(PxCanvas *c, float x, float y, float w, int pos, int total, const char *count, PxRect *track,
              PxRect *thumb);
/* the status row (y 18-46): the app's brand plate on the left, battery and clock on the right */
void px_status(PxCanvas *c, const char *brand, int battery, int charging, const char *clock);
/* a section label (13, capitals, spaced, muted) */
void px_label(PxCanvas *c, float x, float y, const char *s, uint32_t rgb);
/* "<  value  >" in a row: arrows only where there is somewhere to go */
void px_value_arrows(PxCanvas *c, float right, float y, float size, const char *value, uint32_t rgb, int left,
                     int right_on);

/* what the status row shows */
int px_battery(int *charging);                 /* percent, -1 without one */
void px_clock(char *buf, int n, const PxSettings *st);

/* colours */
uint32_t px_mix(uint32_t a, uint32_t b, float t);
uint32_t px_ink_on(uint32_t rgb);

#endif
