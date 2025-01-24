#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdbool.h>

#include "input.h"

#include "screen.h"

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

Uint64 currentPC = 0;
Uint64 previousPC = 0;
double deltaTime = 0;

int initialRepeatDelay = 350;
int repeatDelays[6] = {100, 50, 50, 50, 50, 15};

//to replace with config file
#if defined(_WIN32) || defined(_WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(__WINDOWS__)
#define KPTA 27
#define KPTB 29
#define KPTS 225
#define KPTE 40
#else
#define KPTA 44
#define KPTB 224
#define KPTS 23
#define KPTE 40
#endif

KeyDef keys[8] = {
    {80,true,false,0,0}, //Left
    {79,true,false,0,0}, //Right
    {81,true,false,0,0}, //Down
    {82,true,false,0,0}, //Up
    {KPTB,false,false,0,0}, //B, del
    {KPTA,false,false,0,0}, //A, input
    {KPTS,false,false,0,0}, //page turn
    {KPTE,false,false,0,0} //Enter, play
};

bool IsScancodePressed(int key) {
    return (bool)SDL_GetKeyboardState(NULL)[key];
}

bool IsPressed(int key) {
    return IsScancodePressed(keys[key].scancode);
}

bool IsJustPressed(int key, int repeats) {
    switch(repeats){
    case 0:
        return keys[key].pressed>0;
    default:
        return keys[key].pressed==repeats;
    }
}

bool HandleInputs() {
    previousPC = currentPC;
    currentPC = SDL_GetPerformanceCounter();

    deltaTime = (double)((currentPC - previousPC)*1000 / (double)SDL_GetPerformanceFrequency() );

    for (int i = 0; i < 8; i++) {
        keys[i].pressed = 0;
    }

    SDL_Event windowEvent;
    while (SDL_PollEvent(&windowEvent))
    {
        if (windowEvent.type == SDL_QUIT) {return true;}
        else if (windowEvent.type == SDL_KEYDOWN) {
            if(windowEvent.key.keysym.scancode == 40 && IsScancodePressed(226))
            {
                ToggleFullscreen();
            }
            if(windowEvent.key.keysym.scancode == 41){return true;}
            if(windowEvent.key.repeat){continue;}
            for (int i = 0; i < 8; i++) {
                KeyDef* key = &keys[i];
                if (windowEvent.key.keysym.scancode != key->scancode) {continue;}
                key->pressed = 1;
                key->repeatCount = 0;
                key->repeatTimer = initialRepeatDelay;
            }
        }
    }

    for (int i = 0; i < 8; i++) {
        KeyDef* key = &keys[i];
        if(!key->repeatable || key->pressed){break;}
        if(IsPressed(i)){
            key->repeatTimer -= deltaTime;
            if(key->repeatTimer <= 0){
                if(key->repeatCount<80){key->repeatCount+=1;}
                key->repeatTimer = repeatDelays[key->repeatCount/16];
                key->pressed = 2;
            }
        }
    }

    return false;
}

