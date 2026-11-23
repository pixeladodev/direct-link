/* Pixelado's characters: each map pixel a square of the character's ink; the idle bobs one pixel and blinks. */
#include "cast.h"
#include "cast_sprites.h"

static const struct { const char *const *map; const CastInk *ink; } CAST[CAST_COUNT] = {
    { cast_nilo_map, cast_nilo_ink },
    { cast_lumi_map, cast_lumi_ink },
    { cast_pixo_map, cast_pixo_ink },
    { cast_pixo_about_map, cast_pixo_about_ink },
};

/* per beat: up (1: one pixel higher), blink (1: eyes closed) */
static const unsigned char BEAT_UP[CAST_BEATS] = { 0, 0, 1, 1, 0, 0, 0, 0 };
static const unsigned char BEAT_BLINK[CAST_BEATS] = { 0, 0, 0, 0, 0, 1, 0, 0 };

static uint32_t ink_of(const CastInk *ink, char ch)
{
    for (; ink->ch; ink++)
        if (ink->ch == ch) return ink->rgb;
    return 0xFF00FF;
}

int cast_pose(int who, int beat)
{
    int b = ((beat + who * 2) % CAST_BEATS + CAST_BEATS) % CAST_BEATS;
    return BEAT_UP[b] | BEAT_BLINK[b] << 1;
}

void cast_draw(PxCanvas *c, int who, float x, float y, float unit, int beat)
{
    const char *const *map;
    const CastInk *ink;
    int pose, up, blink, row, col;
    if (who < 0 || who >= CAST_COUNT) return;
    map = CAST[who].map;
    ink = CAST[who].ink;
    pose = cast_pose(who, beat);
    up = pose & 1;
    blink = pose >> 1 & 1;
    y -= up * unit;
    for (row = 0; row < CAST_H; row++) {
        for (col = 0; col < CAST_W; col++) {
            char ch = map[row][col];
            int run = 1;
            if (ch == '.') continue;
            if (blink && (ch == 'W' || ch == 'E')) {
                /* closed eyes: the eye's lowest row a dark line, the rest skin */
                char below = row + 1 < CAST_H ? map[row + 1][col] : '.';
                ch = ch == 'E' && below != 'W' && below != 'E' ? 'K' : 'S';
                px_rect(c, x + col * unit, y + row * unit, unit, unit, ink_of(ink, ch));
                continue;
            }
            /* a run of the same ink in one rectangle (fewer, larger fills) */
            while (col + run < CAST_W && map[row][col + run] == ch &&
                   !(blink && (ch == 'W' || ch == 'E')))
                run++;
            px_rect(c, x + col * unit, y + row * unit, unit * run, unit, ink_of(ink, ch));
            col += run - 1;
        }
    }
}
