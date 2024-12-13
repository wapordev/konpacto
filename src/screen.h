#ifndef WWSCREEN_H_
#define WWSCREEN_H_
/* ^^ these are the include guards */

/* Prototypes for the functions */
/* Sums two ints */
bool HandleInputs();

void InitializeScreen();

void CleanupScreen();

void RenderScreen();

void clearGrid(int col);

//in this version her name is Reese
//in the pico-8 version her name (different fox) (they are sisters) is TBD
//I may never write this anywhere else
//only time will tell
void DrawFox(int xPos, int yPos, int col1, int col2, int col3, int col4);

void PokeScreen(int pos, int chr, int bg, int fg);

void PrintHex(int input, int bg_color, int fg_color, int xPos, int yPos);

void PrintSelected(char string[], int xPos, int yPos, bool selected, int col0, int col1, int col2, int col3);

void PrintColor(char string[], int xPos, int yPos, int bg_color, int fg_color);

void PrintText(char string[], int xPos, int yPos);

typedef struct TextmodeCell {
  int character;
  int bg_color;
  int fg_color;
}TextmodeCell;

#endif
