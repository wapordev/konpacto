#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL2_gfxPrimitives.h>
#include <stdbool.h>

#include "screen.h"

// Window Settings
#define WINDOW_HEIGHT 480                   // window height in pixels
#define WINDOW_WIDTH 640                    // window width in pixels
#define DEPTH 16                            // window depth in pixels

TextmodeCell textmodeGrid[400];

SDL_Color palette[4] = {
    {131, 118, 156, 255},
    {255, 119, 168, 255},
    {255, 204, 170, 255},
    {255, 241, 232, 255}
};

SDL_Surface* intermediate = NULL;

SDL_Window* window = NULL;
SDL_Surface* screen = NULL;
SDL_Texture* texture = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* font = NULL;



void PrintText(char string[], int xPos, int yPos) {
    int bg_color = 3;
    int fg_color = 4;
    int leftMargin = xPos;
    int i = 0;
    while (string[i] != '\0')
    {
        bool print = true;
        char op = string[i];
        int out = 0;
        if(op >= 'a' && op <= 'z'){
            out = (int)(op-'0') + 26;
        }else if (op >= '0' && op <= '9'){
            out = (int)(op-'0') + 16;
        }else if (op == ';'){
            print=false;
            i++;
            bg_color = (int)string[i] - '0';
        }else if (op == ','){
            print=false;
            i++;
            fg_color = (int)string[i] - '0';
        }else if (op == '\n'){
            print=false;
            xPos = leftMargin;
            yPos += 1;
        }else if (op == '~'){
            out=1;
        }else if (op == '!'){
            out=2;
        }else if (op == '#'){
            out=3;
        }else if (op == '-'){
            out=4;
        }else if (op == '+'){
            out=5;
        }else if (op == ':'){
            out=6;
        }else if (op == '\''){
            out=7;
        }else if (op == '('){
            out=8;
        }else if (op == ')'){
            out=9;
        }else if (op == '<'){
            out=10;
        }else if (op == '>'){
            out=11;
        }else if (op == '@'){
            out=12;
        }else if (op == '$'){
            out=13;
        }else if (op == '{'){
            out=14;
        }else if (op == '}'){
            out=15;
        }

        if (print){
            TextmodeCell* cell = &textmodeGrid[xPos+yPos*20];
            cell->bg_color = bg_color;
            cell->fg_color = fg_color;
            cell->character = out;
            if(++xPos>=20){
                xPos = leftMargin;
                if(++yPos>=20){
                    return;
                }
            }
        }

        i++;
    }
}

void clearGrid(int col) {
    for (int i = 0; i<400; i++) {
        textmodeGrid[i].character = 0;
        textmodeGrid[i].bg_color = col;
        textmodeGrid[i].fg_color = 3;
    }

    textmodeGrid[89].character = 33;
    textmodeGrid[90].character = 34;
}

// Function to initialize SDL components
void InitializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow(
        "Konpacto",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    screen = SDL_CreateRGBSurface(
        0,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        DEPTH,
        0, 0, 0, 0
    );

    font = SDL_LoadBMP("assets/chunkfont.bmp");//LoadImage(font, font_path);
    if (font == NULL) {
        printf("Font could not initialize! SDL_image Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_FillRect(
        screen,
        &screen->clip_rect,
        SDL_MapRGB(screen->format, 0x00, 0x00, 0x00)
    );

    intermediate = SDL_CreateRGBSurfaceWithFormat(0,6,6,8,SDL_PIXELFORMAT_RGB565);
    SDL_FillRect(
        intermediate,
        &intermediate->clip_rect,
        SDL_MapRGB(intermediate->format, 0x00, 0x00, 0x00)
    );

    clearGrid(2);
}

// Function to clean up SDL components
void CleanupSDL() {
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void RenderScreen() {

    SDL_Rect src_rect;
    src_rect.w = 6;
    src_rect.h = 6;
    SDL_Color colors[2];

    SDL_Rect dst_rect;
    dst_rect.w = 24;
    dst_rect.h = 24;

    for (int i = 0; i<400; i++){
        
        int character = textmodeGrid[i].character;
        src_rect.x = (character%16)*6;
        src_rect.y = (character/16)*6;

        colors[0] = palette[textmodeGrid[i].bg_color];
        colors[1] = palette[textmodeGrid[i].fg_color];

        int success = SDL_SetPaletteColors(font->format->palette,colors,0,2);
        if (success != 0) {
            printf("could not set all colors: %s\n",SDL_GetError());
        }

        SDL_BlitSurface(font,&src_rect,intermediate,NULL);

        dst_rect.x = 80+((i%20)*24);
        dst_rect.y = (i/20)*24;
        SDL_BlitScaled(intermediate,NULL,screen,&dst_rect);
    }

    PrintText("hi! @\nlol,0die",0,0);

    clearGrid(2);

    // Main loop continuation
    // Flip the backbuffer
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}