#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ui.h"
#include "file.h"
#include "pages.h"
#include "screen.h"

#define CreateGrid(width, height, setPtr, drawPtr)  {width, height, 0, 0, width>1 && height>1, setPtr, drawPtr}


int themeCount=0;
int themeIndex=0;
char** themeList;

int fontCount=0;
int fontIndex=0;
char** fontList;

void SetTheme(int xPos, int yPos, UIEvent event) {
	strcpy(helpString,"4x1png assets/themes");
	if(event.type!=UIChange){return;}
	int change = event.change;
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

	if(strcmp(themeList[themeIndex],"Kleahpacto.png")==0){
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

void SetFont(int xPos, int yPos, UIEvent event) {
	strcpy(helpString,"16x8chr assets/fonts");
	if(event.type!=UIChange){return;}
	int change = event.change;
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

int noFile[15]={0x2E,0x27,0x27,0x1A,0x26,0x1E,0x1D,0,0x1F,0x22,0x25,0x1E,0x5};
int fileName[15];
int lastChar = 0x1A;

void SetFile(int xPos, int yPos, UIEvent event){
	if(event.type == UIDelete){
		for(int i=xPos; i<15; i++){
			fileName[i]=0;
		}
	}else {
		if(event.type == UIPlace){
			if(fileName[xPos] == 0){
				if(fileName[0]==0){
					lastChar=0x1A;
				}
				fileName[xPos] = lastChar;
				for(int i=0; i<xPos; i++){
					if(fileName[i]==0){
						fileName[i]=4;
					}
				}
			}else{
				lastChar = fileName[xPos];
			}
			fileName[xPos] = positive_modulo(fileName[xPos]-1,0x33)+1;
		}

		if(event.type == UIChange){
			fileName[xPos] += event.change;
			if(fileName[xPos]!=0){
				lastChar = fileName[xPos];
			}
			fileName[xPos] = positive_modulo(fileName[xPos]-1,0x33)+1;
		}
	}
}

void DrawFile(int xPos, int yPos, bool selected){
	int* chr = &fileName[0];
	if(fileName[0]==0){
		chr = &noFile[0];
	}
	PokeSelected(65+xPos,chr[xPos],selected,2,3,1,0);
}

void ScrollGrid(int* scroll, int* yPos, UIEvent event){
	if(event.type==UIMove || event.type==UIMoveRepeat || event.type==UIPageChange){
		int ch = 1;
		if(event.type==UIPageChange){ch=16;}
		if(*yPos==0 && *scroll>0){
			ch=clamp(ch,0,*scroll);
			*scroll-=ch;
			*yPos = 1;
		}
		if(*yPos>7 && *scroll<0xf0){
			ch=clamp(ch,0,0xf0-*scroll);
			*scroll+=ch;
			*yPos = 7;
		}
		return;
	}
}

void SetArrange(int xPos, int yPos, UIEvent event){
	
	if(arrangePage.grids[0].xPos<8){
		char string[20] = "channel 0 track";
		string[8]='1'+arrangePage.grids[0].xPos;
		strcpy(helpString,string);
	}else{
		strcpy(helpString,"jump to: pattern 0");
	}
	
	ScrollGrid(&arrangeScroll,&arrangePage.grids[0].yPos,event);
	
	
}

void DrawArrange(int xPos, int yPos, bool selected){
	PrintSelected(". ",xPos*2+2,yPos+3,selected,2,3,1,0);
}

void SetTrackInfo(int xPos, int yPos, UIEvent event){
	switch(xPos){
	case 0:
		strcpy(helpString,"track number");
		break;
	case 1:
		strcpy(helpString,"track speed");
		break;
	case 2:
		strcpy(helpString,"track length");
		break;
	}
}

void DrawTrackInfo(int xPos, int yPos, bool selected){
	HexSelected(0,xPos*7+4,1,selected,2,3,1,0);
}

void SetTrackData(int xPos, int yPos, UIEvent event){
	switch(xPos){
	case 0:
		strcpy(helpString,"note");
		break;
	case 1:
		strcpy(helpString,"instrument");
		break;
	case 2:
		strcpy(helpString,"volume");
		break;
	case 3:
		strcpy(helpString,"command");
		break;
	case 4:
		strcpy(helpString,"parameter 1");
		break;
	case 5:
		strcpy(helpString,"parameter 2");
		break;
	case 6:
		strcpy(helpString,"parameter 3");
		break;
	}
	ScrollGrid(&trackScroll,&trackPage.grids[1].yPos,event);
}

int trackDataWidths[7] = {0,3,6,9,11,13,15};
void DrawTrackData(int xPos, int yPos, bool selected){
	PrintSelected(". ",trackDataWidths[xPos]+2,yPos+3,selected,2,3,1,0);
}

UIPage projectPage = {0,3,(UIGrid[3]){CreateGrid(1,1,&SetTheme,&DrawTheme),CreateGrid(1,1,&SetFont,&DrawFont),CreateGrid(15,1,&SetFile,&DrawFile)} };

UIPage arrangePage = {0,1,(UIGrid[1]){CreateGrid(9,16,&SetArrange,&DrawArrange)}};

UIPage trackPage = {0,2,(UIGrid[2]){CreateGrid(3,1,&SetTrackInfo,&DrawTrackInfo),CreateGrid(7,16,&SetTrackData,&DrawTrackData)}};
