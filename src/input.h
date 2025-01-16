#ifndef WWINPUT_H_
#define WWINPUT_H_
/* ^^ these are the include guards */

/* Prototypes for the functions */
/* Sums two ints */
bool HandleInputs();

bool IsPressed(int key);

bool IsJustPressed(int key, int repeats);

typedef struct KeyDef {
	int scancode;
	bool repeatable;
	int pressed;
	int repeatTimer;
	int repeatCount;
}KeyDef;

#endif
