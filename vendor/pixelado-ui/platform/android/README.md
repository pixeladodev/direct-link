# platform/android

How the portable library (`include/`, `src/`) runs on Android. Skeleton only: no Gradle project here (the app lives
in pixelado-party's `platforms/android/`). The portable core never includes anything from this folder.

| Piece | Here | Status |
|---|---|---|
| Drawing | `pixel_surface.{h,c}` | `px_xrgb_to_rgba` works (portable); attach/present compile with the NDK |
| Sound | `pixel_sound_aaudio.c` | stub of `pixel_sound.h` |
| Input | below | design |
| Updater | none | `updater.c` is Linux only (it runs `update.py`); Android updates through the APK |

## Drawing

1. Kotlin gives JNI a `Surface` (a `SurfaceView` or a `TextureView`'s). `ANativeWindow_fromSurface` turns it into an
   `ANativeWindow`.
2. The native side keeps one `PxCanvas` over a `uint32_t` buffer (`px_canvas(&c, buf, w, h)`), at `640x480 units *
   scale` (1 unit = 1 dp, so the scale is the screen density).
3. Each frame the core's view code draws into it exactly as on the handhelds, then `px_surface_present` locks the
   window, copies with an R/B swizzle (the canvas is XRGB8888, Android's RGBX_8888 is R,G,B in memory) and posts.
4. Alternative with no NDK window: draw into a Java `Bitmap` (`AndroidBitmap_lockPixels`, same swizzle) and draw it
   with Compose; `px_xrgb_to_rgba` serves both.
5. Pixel look rules hold: no scaling filter. Use an integer scale where possible, nearest-neighbour otherwise
   (`SurfaceHolder.setFixedSize` to the unit grid and let the compositor scale with nearest).

## Input

The core takes the handheld's keys. Map Android to them in the JNI layer: D-pad / hat axis to up/down/left/right,
`KEYCODE_BUTTON_A/B/X/Y` to A/B/X/Y (swap A/B if the layout asks), L1/R1 to L/R, `BUTTON_START/SELECT` to
Start/Select. Touch events become `PxRect` hits (`px_face` takes its touch area in `hit`); the on-screen controls use
the sizes in `docs/DESIGN.md` (never below 48 dp).

## Sound

`pixel_sound.h` is the contract. On Android: AAudio (API 26+) or Oboe, one output stream kept open, the clips decoded
from the APK's assets and mixed in the audio callback; a new sound cuts the one playing, as on the handhelds.

## Fonts

Pixelify Sans is read by `px_fonts_load_any`; on Android pass a directory the app extracted from its assets on first run.
