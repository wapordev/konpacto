#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "input.h"
#include "screen.h"

int main(int argc, char* argv[])
{
    // Initialization code
    InitializeSDL();

    // Main loop
    while (true) {
        if(HandleInputs()){
            break;
        }

        RenderScreen();

    }
    // Cleanup code
    CleanupSDL();

    return EXIT_SUCCESS;
}