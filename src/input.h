#ifndef WWINPUT_H_
#define WWINPUT_H_
/* ^^ these are the include guards */

/* Prototypes for the functions */
/* Sums two ints */
bool HandleInputs();

bool IsPressed(int key);

bool IsJustPressed(int key);

typedef struct KeyDef {
	int scancode;
	bool repeatable;
	bool pressed;
	int repeatTimer;
	int repeatCount;
}KeyDef;

#endif
