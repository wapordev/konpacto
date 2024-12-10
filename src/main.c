#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL2_gfxPrimitives.h>
#include <stdbool.h>

#include "input.h"
#include "screen.h"

int main(int argc, char* argv[])
{
    // Initialization code
    InitializeSDL();

    

    // Main loop
    bool quit = false;
    while (!quit) {
        quit = HandleInputs();

        RenderScreen();

    }
    // Cleanup code
    CleanupSDL();

    return EXIT_SUCCESS;
}