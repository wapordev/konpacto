#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdbool.h>

#include "screen.h"

// Window Settings
#define WINDOW_HEIGHT 480                   // window height in pixels
#define WINDOW_WIDTH 640                    // window width in pixels
#define DEPTH 16                            // window depth in pixels

int charWidth = 12;
int charHeight = 12;


TextmodeCell textmodeGrid[400];

SDL_Color palette[4] = {
    {131, 118, 156, 255},
    {255, 119, 168, 255},
    {255, 204, 170, 255},
    {255, 241, 232, 255}
};

SDL_Window* window = NULL;
SDL_Surface* screen = NULL;
SDL_Texture* texture = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* font = NULL;

void PokeScreen(int pos, int chr, int bg, int fg) {
    textmodeGrid[pos].character = chr;
    textmodeGrid[pos].bg_color = bg;
    textmodeGrid[pos].fg_color = fg;
}

//46
void DrawFox(int xPos, int yPos, int col0, int col1, int col2, int col3) {
    int pos=xPos+yPos*20;

    PokeScreen(pos,0x46,col2,col3);
    PokeScreen(pos+1,0x5A,col3,col2);
    PokeScreen(pos+2,0x39,col2,col1);
    PokeScreen(pos+3,0x39,col2,col1);
    PokeScreen(pos+4,0x5D,col3,col2);
    PokeScreen(pos+5,0x49,col2,col3);
    PokeScreen(pos+20,0x7E,col2,col3);
    PokeScreen(pos+21,0x56,col1,col3);
    PokeScreen(pos+22,0x00,col1,col3);
    PokeScreen(pos+23,0x00,col1,col3);
    PokeScreen(pos+24,0x59,col1,col3);
    PokeScreen(pos+25,0x7F,col2,col3);
    PokeScreen(pos+40,0x6F,col1,col2);
    PokeScreen(pos+41,0x66,col1,col0);
    PokeScreen(pos+42,0x67,col1,col0);
    PokeScreen(pos+43,0x68,col1,col0);
    PokeScreen(pos+44,0x69,col1,col0);
    PokeScreen(pos+45,0x6E,col1,col2);
    PokeScreen(pos+60,0x4E,col2,col1);
    PokeScreen(pos+61,0x76,col1,col0);
    PokeScreen(pos+62,0x57,col1,col3);
    PokeScreen(pos+63,0x58,col1,col3);
    PokeScreen(pos+64,0x79,col1,col0);
    PokeScreen(pos+65,0x4F,col2,col1);
    PokeScreen(pos+80,0x48,col2,col1);
    PokeScreen(pos+81,0x4D,col3,col2);
    PokeScreen(pos+82,0x77,col3,col0);
    PokeScreen(pos+83,0x78,col3,col0);
    PokeScreen(pos+84,0x4A,col3,col2);
    PokeScreen(pos+85,0x47,col2,col1);
    PokeScreen(pos+102,0x74,col2,col3);
    PokeScreen(pos+103,0x75,col2,col3);
}

void PrintColor(char string[], int xPos, int yPos, int bg_color, int fg_color) {
    int leftMargin = xPos;
    int i = 0;
    while (string[i] != '\0')
    {
        bool print = true;
        bool colorOnly = false;
        char op = string[i];
        int out = 0;
        if(op >= 'a' && op <= 'z'){
            out = (int)(op-'a') + 26;
        }else if (op >= '0' && op <= '9'){
            out = (int)(op-'0') + 16;
        }else if (op == ';'){
            print=false;
            i++;
            bg_color = string[i] - '0';
        }else if (op == ','){
            print=false;
            i++;
            fg_color = string[i] - '0';
        }else if (op == '\n'){
            print=false;
            xPos = leftMargin;
            yPos += 1;
        }else if (op == 'X'){
            colorOnly=true;
        }else if (op == '='){
            print=false;
        }else if (op >= 'A' && op <= 'O'){
            out=(op-'A')+1;
        }

        if (print){
            TextmodeCell* cell = &textmodeGrid[xPos+yPos*20];
            cell->bg_color = bg_color;
            cell->fg_color = fg_color;
            if(!colorOnly){cell->character = out;}
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

void PrintText(char string[], int xPos, int yPos){
    PrintColor(string, xPos, yPos, 2, 3);
}

void PrintSelected(char string[], int xPos, int yPos, bool selected, int col0, int col1, int col2, int col3){
    int bg_color = col0;
    int fg_color = col1;
    if(selected){
        bg_color = col2;
        fg_color = col3;
    }
    PrintColor(string, xPos, yPos, bg_color, fg_color);
}

void PrintHex(int input, int xPos, int yPos, int bg_color, int fg_color){
    TextmodeCell* cell = &textmodeGrid[xPos+yPos*20];
    cell->bg_color = bg_color;
    cell->fg_color = fg_color;
    cell->character = (input/16)+16;

    cell = &textmodeGrid[(xPos+1)+yPos*20];
    cell->bg_color = bg_color;
    cell->fg_color = fg_color;
    cell->character = (input%16)+16;
};

void HexSelected(int input, int xPos, int yPos, bool selected, int col0, int col1, int col2, int col3){
    int bg_color = col0;
    int fg_color = col1;
    if(selected){
        bg_color = col2;
        fg_color = col3;
    }
    PrintHex(input,xPos,yPos,bg_color,fg_color);
}

void clearGrid(int col) {
    for (int i = 0; i<400; i++) {
        textmodeGrid[i].character = 0;
        textmodeGrid[i].bg_color = col;
        textmodeGrid[i].fg_color = 3;
    }
}

void ScreenResize(int fontW, int fontH) {
    if(screen != NULL){SDL_FreeSurface(screen);}
    if(texture != NULL){SDL_DestroyTexture(texture);}
    SDL_RenderSetLogicalSize(renderer, fontW, fontH);
    screen = SDL_CreateRGBSurface(
        0,
        fontW,
        fontH,
        DEPTH,
        0, 0, 0, 0
    );
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        fontW,
        fontH
    );
    SDL_FillRect(
        screen,
        &screen->clip_rect,
        SDL_MapRGB(screen->format, 0x00, 0x00, 0x00)
    );
}

// Function to initialize SDL components
void InitializeScreen() {
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

    SDL_SetWindowResizable(window,true);

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    SDL_RenderSetIntegerScale(renderer, true);
    ScreenResize(charWidth*20,charHeight*20);
    font = SDL_LoadBMP("assets/newbigfont.bmp");//LoadImage(font, font_path);
    if (font == NULL) {
        printf("Font could not initialize! SDL_image Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    clearGrid(2);
}

// Function to clean up SDL components
void CleanupScreen() {
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void RenderScreen() {

    SDL_Rect src_rect;
    src_rect.w = charWidth;
    src_rect.h = charHeight;
    SDL_Color colors[2];

    SDL_Rect dst_rect;
    dst_rect.w = charWidth;
    dst_rect.h = charHeight;

    for (int i = 0; i<400; i++){
        
        int character = textmodeGrid[i].character;
        src_rect.x = (character%16)*charWidth;
        src_rect.y = (character/16)*charHeight;

        colors[0] = palette[textmodeGrid[i].bg_color];
        colors[1] = palette[textmodeGrid[i].fg_color];

        int success = SDL_SetPaletteColors(font->format->palette,colors,0,2);
        if (success != 0) {
            printf("could not set all colors: %s\n",SDL_GetError());
        }


        dst_rect.x = ((i%20)*charWidth);
        dst_rect.y = (i/20)*charHeight;
        SDL_BlitSurface(font,&src_rect,screen,&dst_rect);
    }

    // Main loop continuation
    // Flip the backbuffer
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}