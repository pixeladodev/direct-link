# core

The portable part of Direct Link, in C: `view.c` (the two screens, drawn from a model), `pixel.c` (the pixel renderer
in Pixelado's design system: `px_pal_pixelado`, pixel corners, Pixelify Sans), `pixel_png.c` (PNG output for the
previews). Interfaces each front end implements: `pixel_sound.h` (menu sounds), `updater.h` (the app's updates).
