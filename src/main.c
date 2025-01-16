#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "input.h"
#include "sound.h"
#include "screen.h"
#include "ui.h"
#include "pages.h"


int num = 0;

int main(int argc, char* argv[])
{
    // Initialization code
    InitializeScreen();
    //InitializeSound();
    InitializePages();

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
    //CleanupSound();

    return EXIT_SUCCESS;
}