#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <string>
using namespace std;

// Window Settings
#define WINDOW_HEIGHT 480                   // window height in pixels
#define WINDOW_WIDTH 640                    // window width in pixels
#define DEPTH 16                            // window depth in pixels
#define MAX_VISIBLE_ITEMS 5                 // Set the maximum number of visible

// Controller inputs
#define SW_BTN_UP SDLK_UP
#define SW_BTN_DOWN SDLK_DOWN
#define SW_BTN_LEFT SDLK_LEFT
#define SW_BTN_RIGHT SDLK_RIGHT

#define SW_BTN_A SDLK_SPACE
#define SW_BTN_B SDLK_LCTRL
#define SW_BTN_X SDLK_LSHIFT
#define SW_BTN_Y SDLK_LALT

#define SW_BTN_L1 SDLK_e
#define SW_BTN_R1 SDLK_t
#define SW_BTN_L2 SDLK_TAB
#define SW_BTN_R2 SDLK_BACKSPACE

#define SW_BTN_SELECT SDLK_RCTRL
#define SW_BTN_START SDLK_RETURN
#define SW_BTN_MENU SDLK_ESCAPE
#define SW_BTN_POWER SDLK_FIRST

#define PREFIX "[SDL2 TestApp] " 
const int w = 320;
const int h = 240;
const int bpp = 32;
static SDL_Window *window = NULL;
static SDL_Surface *screen = NULL;
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Rect rt = {0};

// Function to load an image
SDL_Surface* LoadImage(SDL_Surface** imgSurafce, string& imagePath) {
    printf("starting: LoadImage\n");
    SDL_Surface* optimizedSurface = NULL;
    *imgSurafce = IMG_Load(imagePath.c_str());
    if (*imgSurafce == NULL) {
        printf("Unable to load image! SDL Error: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    optimizedSurface = SDL_ConvertSurfaceFormat(*imgSurafce, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(*imgSurafce);
    if (optimizedSurface == NULL) {
        printf("Unable to optimize image! SDL Error: %s\n", SDL_GetError());
        exit;
    }
    printf("ending: LoadImage\n");
    return optimizedSurface;
}

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

    string font_path = "assets/chunkfont.bmp";
    *font = SDL_LoadBMP(font_path.c_str());//LoadImage(font, font_path);
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

void HandleInputs(SDL_Event& windowEvent, bool& quit) {
    while (SDL_PollEvent(&windowEvent))
    {
        if (windowEvent.type == SDL_QUIT) {
            quit = true;
        }
        else if (windowEvent.type == SDL_KEYDOWN) {
            switch (windowEvent.key.keysym.sym) {
            case SDLK_RIGHT:
                break;
            case SDLK_LEFT:
                break;
            case SDLK_UP:
                break;
            case SDLK_DOWN:
                break;
            case SW_BTN_R1:
                break;
            case SW_BTN_L1:
                break;
            case SW_BTN_R2:
                break;
            case SW_BTN_L2:
                break;
            case SDLK_ESCAPE:
                quit = true;
                break;
            default:
                break;
            }
        }
    }
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

    SDL_Surface* single_char = SDL_CreateRGBSurfaceWithFormat(0,6,6,2,SDL_PIXELFORMAT_INDEX8);

    SDL_Surface* intermediate = SDL_CreateRGBSurfaceWithFormat(0,6,6,8,SDL_PIXELFORMAT_RGB565);
    SDL_FillRect(
        intermediate,
        &intermediate->clip_rect,
        SDL_MapRGB(intermediate->format, 0x00, 0x00, 0x00)
    );

    // Main loop
    SDL_Event windowEvent;
    bool quit = false;
    while (!quit) {
        HandleInputs(windowEvent, quit);

        SDL_Color* palette = single_char->format->palette->colors;

        printf("colr: %u\n",(sizeof(*palette)/sizeof(palette)));

        return EXIT_SUCCESS;

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



        
        SDL_BlitSurface(font,&src_rect,single_char,NULL);
        
        int success = SDL_SetPaletteColors(single_char->format->palette,colors,0,2);
        if (success != 0) {
            printf("could not set all colors: %s\n",SDL_GetError());
        }

        SDL_BlitSurface(single_char,NULL,intermediate,NULL);
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