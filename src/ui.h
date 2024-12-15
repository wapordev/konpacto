#ifndef WWUI_H_
#define WWUI_H_
/* ^^ these are the include guards */

/* Prototypes for the functions */
/* Sums two ints */
void RenderUI();

typedef struct UIGrid {
	int width;
	int height;
	int xPos;
	int yPos;
	bool horizontalLink;
	void (*setPtr)(int, int, int);
	void (*drawPtr)(int, int, bool);
}UIGrid;

typedef struct UIPage {
	int pointer;
	int length;
	UIGrid* grids;
}UIPage;

#endif
