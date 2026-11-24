/* Android drawing path (skeleton), see pixel_surface.h. */
#include "pixel_surface.h"

void px_xrgb_to_rgba(const uint32_t *src, uint32_t *dst, int w, int h, int stride_px) {
    for (int y = 0; y < h; y++) {
        const uint32_t *s = src + (long)y * w;
        uint32_t *d = dst + (long)y * stride_px;
        for (int x = 0; x < w; x++) {
            uint32_t p = s[x];
            d[x] = 0xFF000000u | ((p & 0xFF) << 16) | (p & 0xFF00) | ((p >> 16) & 0xFF);
        }
    }
}

#ifdef __ANDROID__
int px_surface_attach(ANativeWindow *win, int w, int h) {
    return ANativeWindow_setBuffersGeometry(win, w, h, WINDOW_FORMAT_RGBX_8888);
}

int px_surface_present(ANativeWindow *win, const PxCanvas *c) {
    ANativeWindow_Buffer b;
    if (ANativeWindow_lock(win, &b, NULL) != 0) return -1;
    int w = c->w < b.width ? c->w : b.width, h = c->h < b.height ? c->h : b.height;
    px_xrgb_to_rgba(c->px, (uint32_t *)b.bits, w, h, b.stride);
    return ANativeWindow_unlockAndPost(win);
}
#endif
