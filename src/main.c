#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL2_gfxPrimitives.h>
#include <stdbool.h>

#include <input.h>

// Window Settings
#define WINDOW_HEIGHT 480                   // window height in pixels
#define WINDOW_WIDTH 640                    // window width in pixels
#define DEPTH 16                            // window depth in pixels

static SDL_Window *window = NULL;
static SDL_Surface *screen = NULL;
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer = NULL;

// Function to initialize SDL components
void InitializeSDL(SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** texture, SDL_Surface** screen, SDL_Surface** font) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    *window = SDL_CreateWindow(
        "Konpacto",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    *renderer = SDL_CreateRenderer(
        *window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    *texture = SDL_CreateTexture(
        *renderer,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    *screen = SDL_CreateRGBSurface(
        0,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        DEPTH,
        0, 0, 0, 0
    );

    *font = SDL_LoadBMP("assets/chunkfont.bmp");//LoadImage(font, font_path);
    if (*font == NULL) {
        printf("Font could not initialize! SDL_image Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_FillRect(
        *screen,
        &(*screen)->clip_rect,
        SDL_MapRGB((*screen)->format, 0x00, 0x00, 0x00)
    );
}

// Function to clean up SDL components
void CleanupSDL(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture, SDL_Surface* screen) {
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}



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
        quit = input.HandleInputs();

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