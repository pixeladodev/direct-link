/* Android drawing path (skeleton): the portable renderer draws into a PxCanvas buffer, this copies it to a Surface.
   Built only with the NDK (see README.md); nothing in core (include/, src/) depends on it. */
#ifndef PIXEL_SURFACE_H
#define PIXEL_SURFACE_H

#include <stdint.h>

#include "pixel.h"

#ifdef __ANDROID__
#include <android/native_window.h>

/* Attach to a Surface's ANativeWindow and size its buffers to w x h (the 640x480 unit grid times the density scale). */
int px_surface_attach(ANativeWindow *win, int w, int h);
/* Copy a finished frame (XRGB8888, as PxCanvas holds it) to the window: lock, swizzle to RGBX_8888, unlock and post. */
int px_surface_present(ANativeWindow *win, const PxCanvas *c);
#endif

/* XRGB8888 (0x00RRGGBB) to the byte order Android wants (R, G, B, A in memory = 0xAABBGGRR on little endian).
   Portable, so it can be tested on the Mac. stride_px is the destination row length in pixels. */
void px_xrgb_to_rgba(const uint32_t *src, uint32_t *dst, int w, int h, int stride_px);

#endif
