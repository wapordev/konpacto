#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL2_gfxPrimitives.h>
#include <stdbool.h>

#include "input.h"

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

bool HandleInputs() {
    SDL_Event windowEvent;
    while (SDL_PollEvent(&windowEvent))
    {
        if (windowEvent.type == SDL_QUIT) {
            return true;
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
                return true;
                break;
            default:
                break;
            }
        }
    }
    return false;
}