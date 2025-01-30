#ifndef WWSOUND_SDL_H_
#define WWSOUND_SDL_H_

#include <SDL.h>

void _InitializeSound();

void _CleanupSound();

extern SDL_AudioSpec returnedSpec;

#endif
