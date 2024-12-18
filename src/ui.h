#ifndef WWUI_H_
#define WWUI_H_
/* ^^ these are the include guards */

/* Prototypes for the functions */
/* Sums two ints */
void RenderUI();

void InitializePages();

int clamp(int d, int min, int max);

int positive_modulo(int i, int n);

typedef enum {
    UINothing,
    UIPlace,
    UIDelete,
    UIChange,
    UIPageChange,
    UIMove,
    UIMoveRepeat
} UIEventType;

typedef struct UIEvent {
	UIEventType type;
	int horizontal;
	int vertical;
	int change;
}UIEvent;

typedef struct UIGrid {
	int width;
	int height;
	int xPos;
	int yPos;
	bool horizontalLink;
	void (*setPtr)(int, int, UIEvent);
	void (*drawPtr)(int, int, bool);
}UIGrid;

typedef struct UIPage {
	int index;
	int length;
	UIGrid* grids;
}UIPage;

#endif
