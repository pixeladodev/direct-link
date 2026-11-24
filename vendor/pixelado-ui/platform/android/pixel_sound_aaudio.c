/* Android sound (skeleton): implements include/pixel_sound.h on AAudio (or Oboe). Not built yet: the functions
   are the contract; the bodies are the next step. Clips are the same .wav files the handhelds use, read from the
   APK's assets (AAssetManager) instead of /usr/share/banana/rnds/sounds. */
#include "pixel_sound.h"

void px_sound_init(const PxSettings *st, const char *dir) { (void)st; (void)dir; /* TODO: load clips, open a low-latency AAudio output stream */ }
void px_sound_play(const char *name) { (void)name; /* TODO: mix the clip into the stream, cutting the one playing */ }
void px_sound_close(int max_ms) { (void)max_ms; /* TODO: let it finish, close the stream */ }
void px_sound_release(int max_ms) { px_sound_close(max_ms); /* Android shares the output: nothing extra to give back */ }
