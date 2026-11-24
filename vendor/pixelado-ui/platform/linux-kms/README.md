# platform/linux-kms

The Linux / BananaOS pieces both handheld apps share: `pixel_sound.c` (ALSA through `dlopen`, implements
`include/pixel_sound.h`) and `updater.c` (runs `update.py`, implements `include/updater.h`). Compile them with
`-Iinclude`; they need POSIX, pthreads and `-ldl`. Not used on Android.
