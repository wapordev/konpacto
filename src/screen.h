#ifndef WWSCREEN_H_
#define WWSCREEN_H_
/* ^^ these are the include guards */

/* Prototypes for the functions */
/* Sums two ints */
bool HandleInputs();

void InitializeSDL(SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** texture, SDL_Surface** screen, SDL_Surface** font);

void CleanupSDL(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* screen);

#endif
