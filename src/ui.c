#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

//TEMP!!
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "input.h"
#include "ui.h"
#include "screen.h"
#include "pages.h"
#include "file.h"

#include "sound.h"
#include "synth.h"

int page=0;

char pageNames[6][15] = {"project 0.0.13","arrange","compose","track edit","operators","file select"};

char helpString[21];

inline int positive_modulo(int i, int n) {
    return (i % n + n) % n;
}

void PageProcess(UIPage* page, UIEvent* event) {
	
	if(event->type == UIMove || event->type == UIMoveRepeat || (event->type==UIPageChange && event->horizontal==0)){

		UIGrid *g = &page->grids[page->index];

		g->xPos += event->horizontal;

		if(event->type==UIPageChange){
			g->yPos -= event->vertical*16;
		}else {
			g->yPos -= event->vertical;



			if((g->height<2 || event->type==UIMove) && (g->yPos < 0 || g->yPos >= g->height)){
				page->index=clamp(page->index-event->vertical,0,page->length-1);
			}
			//this shouldnt happen on key repeat. ill have to make another case for this
			if(event->type == UIMove && g->horizontalLink && (g->xPos < 0 || g->xPos >= g->width)){
				page->index=0;
			}
		}
		
		g->xPos = clamp(g->xPos,0,g->width-1);
		g->yPos = clamp(g->yPos,0,g->height-1);

		
	}

	UIGrid* curGrid = &page->grids[page->index];

	curGrid->setPtr(curGrid->xPos, curGrid->yPos,*event);

	for(int i = 0; i < page->length; i++) {
		if(i==page->index){continue;}
		UIGrid g = page->grids[i];
		for(int x=0;x<g.width;x++){
			for(int y=0;y<g.height;y++){
				g.drawPtr(x, y, false);
			}
		}
	}
	//arrangePage.grids[0].yPos = 7;
	for(int x=0;x<curGrid->width;x++){
		for(int y=curGrid->height-1;y>=0;y--){
			curGrid->drawPtr(x, y, (x==curGrid->xPos && y==curGrid->yPos));
		}
	}
	
}

void ProjectDraw(UIEvent* event) {
	PrintColor("theme\nfont\nbpm\n-",0,1,2,1,false);
	
	//Fox
	DrawFox(7,9,0,1,2,3);

	char debug[21];

	// sprintf(debug, "%llu", konAudio.tickrate);
	// PrintText(debug,0,15);
	// sprintf(debug, "%llu", konAudio.frameAcumulator);
	// PrintText(debug,0,16);
	// sprintf(debug, "%u", konAudio.arrangeIndex);
	// PrintText(debug,0,17);

	PrintText("kon,1pacto\n\n,0portable",6,15);
	PokeScreen(329,0x74,2,0);
    PokeScreen(330,0x75,2,0);

    PageProcess(&projectPage, event);
}

int arrangeScroll = 0;
void ArrangeDraw(UIEvent* event) {
	PrintText("> ,11 2 3 4 5 6 7 8 jm",0,1);

	for(int i=0;i<16;i++){
		int idx = i+arrangeScroll;
		PrintHex(idx,0,3+i,(idx/4+1)%2,(idx/4)%2);
	}

	PageProcess(&arrangePage, event);
}

void ComposeDraw(UIEvent* event) {
}

int trackScroll = 0;
void TrackDraw(UIEvent* event) {
	PrintText(",1trk    len    grv",0,1);
	for(int i=0;i<16;i++){
		int idx = i+trackScroll;
		PrintHex(idx,0,3+i,(idx/4+1)%2,(idx/4)%2);
	}

	PageProcess(&trackPage, event);
}

void OperatorsDraw(UIEvent* event) {
	PrintText(",1inst    name\nsynth\nfx",0,1);

	for(int i=0;i<21;i++){
		PlaceScreen(i,5,0,0,1);
	}

	for(int i=1;i<20;i++){
		PlaceScreen(i,6,0x45,0,2);
	}
	PlaceScreen(0,6,0x4a,0,2);
	PlaceScreen(19,6,0x4d,0,2);

	for(int x=1;x<19;x++){
		for(int y=9;y<17;y++){
			PlaceScreen(x,y,0,0,3);
		}
	}
	PlaceScreen(1,9,0x6a,2,0); 
	PlaceScreen(18,9,0x6b,2,0); 

	PageProcess(&operatorPage, event);
}

FileContext fileContext = ContextSaveSong;

char lowerCasePath[PATH_MAX] = "";

void FileContextDraw(UIEvent* event) {

	// if(doNameEntry){
	// 	PrintText(",1file name\n;0XXXXXXXXXXXXXXXXXXXX;2-",0,1);
	// }else{
	// 	fileContextPage.index=2;
	// }

	strcpy(lowerCasePath,fileBrowserPath);
	char* p = lowerCasePath;
	for ( ; *p; ++p) *p = tolower(*p);
	int pathLength=strlen(lowerCasePath);
	int pathoffset = 0;
	if(pathLength>20)
		pathoffset = pathLength-20;
	

	switch (fileContext){
	case ContextSaveSong:
		for(int i=6;i<19;i++)
			PrintText(";0XXXXXXXXXXXXXXXXXXXX",0,i);
		for(int i=0;i<20;i++)
			PlaceScreen(i,4,0x55,2,1);
		PrintText(",1file name\n;0,3XXXXXXXXXXXXXXXXXXXX;2,1-",0,1);
		PrintColor(lowerCasePath+pathoffset,0,5,2,1,0);
		PrintColor(currentSongName,0,2,0,3,0);
		PageProcess(&saveFilePage, event);
		break;
	case ContextLoadSong:
		for(int i=4;i<19;i++)
			PrintText(";0XXXXXXXXXXXXXXXXXXXX",0,i);
		for(int i=0;i<20;i++)
			PlaceScreen(i,2,0x55,2,1);
		PrintText(",1-",0,1);

		PrintColor(lowerCasePath+pathoffset,0,3,2,1,0);
		PageProcess(&loadFilePage, event);
		break;
	}

	
}

void RenderUI() {
	clearGrid(2);

	PrintText(pageNames[page],0,19);
	PrintText(";3,2XXXXXXXXXXXXXXX;0,3pacto",0,19);

	

	int horizontal = IsJustPressed(0,0) ? -1 : (IsJustPressed(1,0) ? 1 : 0);
	int vertical = IsJustPressed(2,0) ? -1 : (IsJustPressed(3,0) ? 1 : 0);
	int change = horizontal + vertical*16;

	UIEvent event = {UINothing,horizontal,vertical,change};

	bool eJPress = IsJustPressed(7,0);
	bool sPress = IsPressed(6);
	bool zPress = IsPressed(4);
	bool xPress = IsPressed(5);
	bool xJPress = IsJustPressed(5,0);
	
	if(change!=0){
		event.type = UIMove;
		if(IsJustPressed(0,2) || IsJustPressed(1,2) || IsJustPressed(2,2) || IsJustPressed(3,2)){
			event.type = UIMoveRepeat;
		}
		if (sPress){
			event.type = UIPageChange;
			if (page!=5) {page=clamp(page+horizontal,0,4);}
		}else if(zPress){

		}else if(xPress){
			event.type = UIChange;
		}
	}else if(xJPress){
		event.type = UIPlace;
		if(zPress){
			event.type = UIDelete;
		}
	}

	switch(page){
	case 0: ProjectDraw(&event); break;
	case 1: ArrangeDraw(&event); break;
	case 2: ComposeDraw(&event); break;
	case 3: TrackDraw(&event); break;
	case 4: OperatorsDraw(&event); break;
	case 5: FileContextDraw(&event); break;
	}

	if (eJPress){
		if(konAudio.playing){
			konStop(&konAudio);
		}else{
			konStart(&konAudio,arrangeScroll+arrangePage.grids[0].yPos);
		}
	}

	PrintText(helpString,0,0);
	PrintText(";3,2XXXXXXXXXXXXXXXXXXXX",0,0);
	
	if(page==5){
		PrintColor("!file",15,19,1,0,false);
	} else {
		PrintText(";1,0X",15+page,19);
	}
}

void SetContextPage(FileContext newContext){
	fileContext = newContext;

	switch (newContext){
	case ContextSaveSong:
		strcpy(fileBrowserPath,defaultSongPath);
		ReloadFileBrowse(2);
		break;
	case ContextLoadSong:
		strcpy(fileBrowserPath,defaultSongPath);
		ReloadFileBrowse(0);
		break;
	default:
		break;
	}

	page=5;
}

void QuitContext(int succeeded){
	switch (fileContext){
	case ContextSaveSong:
		if(succeeded){
			strcpy(currentSongPath,selectedFile);
			strcpy(currentSongName,selectedFileName);
			SaveSong(currentSongPath);
			strcpy(defaultSongPath,fileBrowserPath);
		}
		page=0;
		break;
	case ContextLoadSong:
		if(succeeded){
			strcpy(currentSongPath,selectedFile);
			strcpy(currentSongName,selectedFileName);
			LoadSong(selectedFile);
			strcpy(defaultSongPath,fileBrowserPath);
		}
		page=0;
		break;
	default:
		break;
	}
}



char MapChar(char c){
	char mappedChar = 0;
	if(c == '.'){
		mappedChar = 5;
	}else if(c >= 65 && c <= 79){
		mappedChar = c-64;
	}else if (c >= 48 && c <= 57){
		mappedChar = c-32;
	}else if (c >= 97 && c <= 122){
		mappedChar = c-71;
	}

	return mappedChar;
}

char UnmapChar(char c){
	char unmappedChar = 40;
	if(c == 5){
		unmappedChar = '.';
	}else if(c >= 0 && c <= 15){
		unmappedChar = c+64;
	}else if (c >= 16 && c <= 25){
		unmappedChar = c+32;
	}else if (c >= 26 && c <= 56){
		unmappedChar = c+71;
	}

	return unmappedChar;
}

char lastUsedChar='a';

void HandleTextInput(char* text, int xPos, UIEvent event){
	PrintColor("X",xPos,2,1,0,0);

	if(event.type==UIPlace){
		int len = strlen(text);
		if(xPos>=len){
			text[xPos] = lastUsedChar;
			text[xPos+1] = '\0';
			for (int i=0; i<xPos; i++){
				if(i >= len || MapChar(text[i])==0){
					text[i]='D';
				}
			}
		}else{
			lastUsedChar = text[xPos];
		}

		

	}else if(event.type==UIChange){
		char mappedChar = MapChar(text[xPos]);
		
		text[xPos] = UnmapChar(positive_modulo(((int)mappedChar)+event.change,56));
	}else if(event.type==UIDelete){
		text[xPos]='\0';
	}
}