#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ui.h"
#include "pages.h"
#include "screen.h"

#define CreateGrid(width, height, setPtr, drawPtr)  (UIGrid){width, height, 0, 0, width>1, setPtr, drawPtr}

double clamp(int d, int min, int max) {
  const int t = d < min ? min : d;
  return t > max ? max : t;
}

int temp = 0;
void Set(int xPos, int yPos, int change) {
	temp=clamp(temp+change,0,255);
}

void Draw(int xPos, int yPos, bool selected) {
	HexSelected(temp,6,1,selected,2,3,1,0);
}

void Draw2(int xPos, int yPos, bool selected) {
	PrintHex(temp,6,3,2,3);
}

UIPage projectPage = {0,2,(UIGrid[2]){CreateGrid(1,1,&Set,&Draw),CreateGrid(1,1,&Set,&Draw2)} };


