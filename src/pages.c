#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ui.h"
#include "file.h"
#include "pages.h"
#include "screen.h"

#define CreateGrid(width, height, setPtr, drawPtr)  {width, height, 0, 0, width>1, setPtr, drawPtr}


int themeCount=0;
int themeIndex=0;
char** themeList;

int fontCount=0;
int fontIndex=0;
char** fontList;

void SetTheme(int xPos, int yPos, int change) {
	for(int i=0; i<themeCount; i++){
		free(themeList[i]);
	}
	free(themeList);
	themeList = ListPath("assets/palettes",".PNG",&themeCount);

	themeIndex=positive_modulo(themeIndex+change,themeCount);
	ChangeTheme(themeList[themeIndex]);
}

void DrawTheme(int xPos, int yPos, bool selected) {
	PrintSelected(themeList[themeIndex],6,1,selected,2,3,1,0);

	if(strcmp(themeList[themeIndex],"leahpacto.png")==0){
		PokeScreen(187,0x46,2,1);
		PokeScreen(188,0x5A,1,2);

		PokeScreen(208,0x76,3,1);
		PokeScreen(211,0x77,1,3);

		PokeScreen(228,0x5C,3,1);
		PokeScreen(229,0x49,3,1);
		PokeScreen(230,0x7A,3,1);
		PokeScreen(231,0x5B,3,1);

		PokeScreen(248,0x57,1,0);
		PokeScreen(249,0x7F,3,1);
		PokeScreen(250,0x5F,1,3);
		PokeScreen(251,0x58,1,0);

		PokeScreen(268,0x4D,1,2);
		PokeScreen(269,0x68,3,0);
		PokeScreen(270,0x47,3,0);

		PokeScreen(289,0x0,2,2);
		PokeScreen(290,0x0,2,2);
	}
}

void SetFont(int xPos, int yPos, int change) {
	for(int i=0; i<fontCount; i++){
		free(fontList[i]);
	}
	free(fontList);
	fontList = ListPath("assets/fonts",".BMP",&fontCount);

	fontIndex=positive_modulo(fontIndex+change,fontCount);
	ChangeFont(fontList[fontIndex]);
}

void DrawFont(int xPos, int yPos, bool selected) {
	PrintSelected(fontList[fontIndex],5,2,selected,2,3,1,0);
}

void InitializePages(){
	themeList = ListPath("assets/palettes",".PNG",&themeCount);
	ChangeTheme(themeList[themeIndex]);
	fontList = ListPath("assets/fonts",".BMP",&fontCount);
	ChangeFont(fontList[0]);
}

int fileName[15] = {1};


void SetFile(int xPos, int yPos, int change){

}

void DrawFile(int xPos, int yPos, bool selected){
	PokeSelected(65+xPos,fileName[xPos],selected,2,3,1,0);
}

UIPage projectPage = {0,3,(UIGrid[3]){CreateGrid(1,1,&SetTheme,&DrawTheme),CreateGrid(1,1,&SetFont,&DrawFont),CreateGrid(15,1,&SetFile,&DrawFile)} };


