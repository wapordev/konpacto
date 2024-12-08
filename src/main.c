#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL2_gfxPrimitives.h>

#define PREFIX "[SDL2 TestApp] " 
const int w = 320;
const int h = 240;
const int bpp = 32;
static SDL_Window *window = NULL;
static SDL_Surface *screen = NULL;
static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Rect rt = {0};

static void flip(void)
{
    SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

static void fill_color(void)
{
    printf(PREFIX"%s\n", __func__);
    SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0xff, 0x00, 0x00));

    rt.x = 50;
    rt.y = 50;
    rt.w = 30;
    rt.h = 30;
    SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0x00, 0xff, 0x00));
 
    rt.x = 100;
    rt.y = 100;
    rt.w = 50;
    rt.h = 100;
    SDL_FillRect(screen, &rt, SDL_MapRGB(screen->format, 0x00, 0x00, 0xff));

    flip();
    SDL_Delay(3000);
}

static void draw_gfx(void)
{
    printf(PREFIX"%s\n", __func__);
    SDL_RenderClear(renderer);
    boxRGBA(renderer, 50, 100, 100, 150, 0xff, 0x00, 0x00, 0xff);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);
}

static void load_png(void)
{
    printf(PREFIX"%s\n", __func__);
    SDL_Surface *png = IMG_Load("bg.png");
    SDL_BlitSurface(png, NULL, screen, NULL);
    SDL_FreeSurface(png);

    flip();
    SDL_Delay(3000);
}

static void run_screentear(void)
{
    int cc = 300;
    uint32_t col[]={0xff0000, 0xff00, 0xff};

    printf(PREFIX"%s\n", __func__);
    while (cc--) {
        SDL_FillRect(screen, &screen->clip_rect, col[cc % 3]);
        flip();
        SDL_Delay(1000 / 60);
    }
}

static void play_wav(void)
{
    printf(PREFIX"%s\n", __func__);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Music *music = Mix_LoadMUS("nokia.wav");
    Mix_PlayMusic(music, 1);
    SDL_Delay(26000);
    Mix_HaltMusic();
    Mix_FreeMusic(music);
    Mix_CloseAudio();
}

static void draw_text(void)
{
    printf(PREFIX"%s\n", __func__);
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("font.ttf", 24);
    SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
 
    int ww = 0, hh = 0;
    SDL_Color col = {0, 255, 0};
    SDL_Rect rt = {0, 100, 0, 0};
    const char *cc = "SDL2 TestApp by 司徒";
 
    TTF_SizeUTF8(font, cc, &ww, &hh);
    rt.x = (w - ww) / 2;
    SDL_Surface *msg = TTF_RenderUTF8_Solid(font, cc, col);
    SDL_BlitSurface(msg, NULL, screen, &rt);
    SDL_FreeSurface(msg);

    flip();
    SDL_Delay(3000);

    TTF_CloseFont(font);
    TTF_Quit();
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    window = SDL_CreateWindow("main", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    screen = SDL_CreateRGBSurface(0, w, h, bpp, 0, 0, 0, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, w, h);

    fill_color();
    draw_gfx();
    load_png();
    draw_text();
    play_wav();
    run_screentear();
 
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
