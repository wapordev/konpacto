#ifndef WWSCREEN_H_
#define WWSCREEN_H_
/* ^^ these are the include guards */

/* Prototypes for the functions */
/* Sums two ints */
bool HandleInputs();

void InitializeSDL();

void CleanupSDL();

void RenderScreen();

typedef struct TextmodeCell {
  int character;
  int bg_color;
  int fg_color;
};

#endif
