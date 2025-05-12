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
//software scaling, since the external libraries bilinear filter by default
#ifdef MMIYOO
SDL_Surface* intermediate = NULL;
#endif 
SDL_Texture* texture = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* font = NULL;

void ToggleFullscreen() {
    Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
    bool IsFullscreen = SDL_GetWindowFlags(window) & FullscreenFlag;
    SDL_SetWindowFullscreen(window, IsFullscreen ? 0 : FullscreenFlag);
    SDL_ShowCursor(IsFullscreen);
}


void PokeScreen(int pos, int chr, int bg, int fg) {
    textmodeGrid[pos].character = chr;
    textmodeGrid[pos].bg_color = bg;
    textmodeGrid[pos].fg_color = fg;
}

void PlaceScreen(int xPos, int yPos, int chr, int bg, int fg) {
    PokeScreen(xPos+yPos*20,chr,bg,fg);
}

void PlaceSelected(int chr, int xPos, int yPos, bool selected, int col0, int col1, int col2, int col3) {
    PokeSelected(xPos+yPos*20,chr,selected,col0,col1,col2,col3);
}

void PokeSelected(int pos, int chr, bool selected, int col0, int col1, int col2, int col3) {
    int bg_color = col0;
    int fg_color = col1;
    if(selected){
        bg_color = col2;
        fg_color = col3;
    }
    PokeScreen(pos,chr,bg_color,fg_color);
}

void DrawBar(int xPos, int yPos, int height, int bg, int fg){
    height-=1;
    for(int i=yPos;i>yPos-(height/6);i--){
        PlaceScreen(xPos,i,0x0,fg,bg);
    }
    int remainder=height%6;
    PlaceScreen(xPos,yPos-height/6,0x3A+remainder,bg,fg);
}

void BarSelected(int xPos, int yPos, int height, bool selected, int col0, int col1, int col2, int col3){
    int bg_color = col0;
    int fg_color = col1;
    if(selected){
        bg_color = col2;
        fg_color = col3;
    }
    DrawBar(xPos,yPos,height,bg_color,fg_color);
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

void PrintColor(char string[], int xPos, int yPos, int bg_color, int fg_color, bool wrap) {
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
        }else if (op == '~'){
            out=0x01;
        }else if (op == '!'){
            out=0x02;
        }else if (op == '/'){
            out=0x03;
        }else if (op == '\\'){
            out=0x03;
        }else if (op == '-'){
            out=0x04;
        }else if (op == '.'){
            out=0x05;
        }else if (op == ':'){
            out=0x06;
        }else if (op == '\''){
            out=0x07;
        }else if (op == '('){
            out=0x08;
        }else if (op == ')'){
            out=0x09;
        }else if (op == '<'){
            out=0x0A;
        }else if (op == '>'){
            out=0x0B;
        }

        if (print){
            TextmodeCell* cell = &textmodeGrid[xPos+yPos*20];
            cell->bg_color = bg_color;
            cell->fg_color = fg_color;
            if(!colorOnly){cell->character = out;}
            if(++xPos>=20){
                if(!wrap){return;}
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
    PrintColor(string, xPos, yPos, 2, 3, true);
}

void PrintSelected(char string[], int xPos, int yPos, bool selected, int col0, int col1, int col2, int col3){
    int bg_color = col0;
    int fg_color = col1;
    if(selected){
        bg_color = col2;
        fg_color = col3;
    }
    PrintColor(string, xPos, yPos, bg_color, fg_color, selected);
}

void PrintDec(int input, int xPos, int yPos, int bg_color, int fg_color){
    for(int i=100;i>0;i/=10){
        if(input/i){
            PlaceScreen(xPos,yPos,input/i+0x10,bg_color,fg_color);
            xPos+=1;
            input%=i;
        }
    }
}

void DecSelected(int input, int xPos, int yPos, bool selected, int col0, int col1, int col2, int col3){
    int bg_color = col0;
    int fg_color = col1;
    if(selected){
        bg_color = col2;
        fg_color = col3;
    }
    PrintDec(input,xPos,yPos,bg_color,fg_color);
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
    int screenW = fontW*20;
    int screenH = fontH*20;

    if(screen != NULL){SDL_FreeSurface(screen);}
    if(texture != NULL){SDL_DestroyTexture(texture);}

    #ifdef MMIYOO
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        480,
        480
    );
    #else
    //SDL_RenderSetLogicalSize(renderer, screenW, screenH);
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB565,
        SDL_TEXTUREACCESS_STREAMING,
        screenW,
        screenH
    );
    #endif
    screen = SDL_CreateRGBSurface(
        0,
        screenW,
        screenH,
        DEPTH,
        0, 0, 0, 0
    );
    
    SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
    SDL_FillRect(
        screen,
        &screen->clip_rect,
        SDL_MapRGB(screen->format, 0x00, 0x00, 0x00)
    );
}

int ChangeFont(char* fontName) {
    char* path;

    char* pathPrefix = "assets/fonts/";

    path = malloc(strlen(pathPrefix)+strlen(fontName)+1);
    path[0] = '\0';
    strcat(path,pathPrefix);
    strcat(path,fontName);

    font = SDL_LoadBMP(path);
    free(path);

    if (font == NULL) {
        printf("Font could not initialize! SDL_image Error: %s\n", SDL_GetError());
        return 0;
    }

    charWidth = font->w/16;
    charHeight = font->h/8;

    ScreenResize(charWidth,charHeight);

    return 1;
}

int ChangeTheme(char* themeName) {
    char* path;

    char* pathPrefix = "assets/palettes/";

    path = malloc(strlen(pathPrefix)+strlen(themeName)+1);
    path[0] = '\0';
    strcat(path,pathPrefix);
    strcat(path,themeName);

    SDL_Surface* theme;
    theme = IMG_Load(path);
    free(path);

    if (theme == NULL) {
        printf("Theme could not initialize! SDL_image Error: %s\n", SDL_GetError());
        return 0;
    }
    for(int i=0;i<4;i++){

                int bpp = theme->format->BytesPerPixel;
                Uint8 *pixel = (Uint8*)theme->pixels + i * bpp;

                #if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    palette[i].r = pixel[2]; 
                    palette[i].g = pixel[1]; 
                    palette[i].b = pixel[0];
                #else
                    palette[i].r = pixel[0]; 
                    palette[i].g = pixel[1]; 
                    palette[i].b = pixel[2];
                #endif

    }
    SDL_FreeSurface(theme);

    return 1;
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

    #ifdef MMIYOO
    intermediate = SDL_CreateRGBSurface(
        0,
        480,
        480,
        DEPTH,
        0, 0, 0, 0
    );
    #endif

    SDL_SetWindowResizable(window,true);

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    //SDL_RenderSetIntegerScale(renderer, true);

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }

    clearGrid(2);
    SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY,"nearest",SDL_HINT_OVERRIDE);
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

    int windowW;
    int windowH;

    SDL_GetWindowSize(window, &windowW, &windowH);

    bool charsTooBig = (charWidth*20 > windowW || charHeight*20 > windowH);

    //perhaps overly intensive, but.
    if(charsTooBig){
        SDL_RenderSetLogicalSize(renderer, windowH*((float)charWidth/(float)charHeight), windowH);
        SDL_RenderSetIntegerScale(renderer, false);
    }else{
        SDL_RenderSetLogicalSize(renderer, charWidth*20, charHeight*20);
        SDL_RenderSetIntegerScale(renderer, true);
    }

    

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
    #ifdef MMIYOO
    SDL_BlitScaled(screen, NULL, intermediate, NULL);
    SDL_UpdateTexture(texture, NULL, intermediate->pixels, intermediate->pitch);
    #else
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    #endif
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}