#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL2_gfxPrimitives.h>
#include <stdbool.h>

#include "input.h"
#include "screen.h"

// Window Settings
#define WINDOW_HEIGHT 480                   // window height in pixels
#define WINDOW_WIDTH 640                    // window width in pixels
#define DEPTH 16                            // window depth in pixels

static SDL_Window *window = NULL;
static SDL_Surface *screen = NULL;
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer = NULL;

int main(int argc, char* argv[])
{
    SDL_Window* window = NULL;
    SDL_Surface* screen = NULL;
    SDL_Texture* texture = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Surface* font = NULL;

    // Initialization code
    InitializeSDL(&window, &renderer, &texture, &screen, &font);

    SDL_Surface* single_char = SDL_CreateRGBSurfaceWithFormat(0,6,6,1,SDL_PIXELFORMAT_INDEX8);

    SDL_Surface* intermediate = SDL_CreateRGBSurfaceWithFormat(0,6,6,8,SDL_PIXELFORMAT_RGB565);
    SDL_FillRect(
        intermediate,
        &intermediate->clip_rect,
        SDL_MapRGB(intermediate->format, 0x00, 0x00, 0x00)
    );

    // Main loop
    bool quit = false;
    while (!quit) {
        quit = HandleInputs();

/*        SDL_Color* palette = single_char->format->palette->colors;

        printf("colr: %u\n",sizeof(*palette) / sizeof(palette[0]));

        return EXIT_SUCCESS;*/

        //printf("screen: %s\n",SDL_GetPixelFormatName());
        //printf("font: %s\n",SDL_GetPixelFormatName(font->format->format));
        //SDL_Surface* render_target = SDL_CreateSurface();

        SDL_Rect src_rect;
        src_rect.x = 0;
        src_rect.y = 0;
        src_rect.w = 6;
        src_rect.h = 6;

        SDL_Color colors[2];
        colors[0].r = 0;
        colors[0].g = 77;
        colors[0].b = 132;

        colors[1].r = 110;
        colors[1].g = 247;
        colors[1].b = 188;



        int success = SDL_SetPaletteColors(font->format->palette,colors,0,2);
        if (success != 0) {
            printf("could not set all colors: %s\n",SDL_GetError());
        }

        SDL_BlitSurface(font,&src_rect,intermediate,NULL);
        


        //SDL_BlitSurface(single_char,NULL,intermediate,NULL);
        SDL_Rect dst_rect;
        dst_rect.x = 80;
        dst_rect.y = 0;
        dst_rect.w = 24;
        dst_rect.h = 24;
        SDL_BlitScaled(intermediate,NULL,screen,&dst_rect);

        // Main loop continuation
        // Flip the backbuffer
        SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

    }
    // Cleanup code
    CleanupSDL(window, renderer, texture, screen);

    return EXIT_SUCCESS;
}