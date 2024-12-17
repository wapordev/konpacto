#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ui.h"
#include "file.h"
#include "pages.h"
#include "screen.h"

#define CreateGrid(width, height, setPtr, drawPtr)  {width, height, 0, 0, width>1, setPtr, drawPtr}

int temp = 0;
void Set(int xPos, int yPos, int change) {
	temp=clamp(temp+change,0,255);
}

int fontCount=0;
int fontIndex=0;
char** nameList;

void SetFont(int xPos, int yPos, int change) {
	for(int i=0; i<fontCount; i++){
		free(nameList[i]);
	}
	free(nameList);
	nameList = ListPath("assets",".BMP",&fontCount);

	fontIndex=positive_modulo(fontIndex+change,fontCount);
	ChangeFont(nameList[fontIndex]);
}

void DrawFont(int xPos, int yPos, bool selected) {
	PrintSelected(nameList[fontIndex],5,2,selected,2,3,1,0);
}

void Draw(int xPos, int yPos, bool selected) {
	HexSelected(temp,6,1,selected,2,3,1,0);
}

void InitializePages(){
	nameList = ListPath("assets",".BMP",&fontCount);
	ChangeFont(nameList[0]);
}

UIPage projectPage = {0,2,(UIGrid[2]){CreateGrid(1,1,&Set,&Draw),CreateGrid(1,1,&SetFont,&DrawFont)} };


