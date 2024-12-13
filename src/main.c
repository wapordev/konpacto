#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "input.h"
#include "screen.h"

#include "ui.h"


int num = 0;

int main(int argc, char* argv[])
{
    // Initialization code
    InitializeScreen();

    // Main loop
    while (true) {
        if(HandleInputs()){
            break;
        }

        RenderUI();

        RenderScreen();

    }
    // Cleanup code
    CleanupScreen();

    return EXIT_SUCCESS;
}