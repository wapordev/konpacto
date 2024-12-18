#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "input.h"
#include "ui.h"
#include "screen.h"
#include "pages.h"
#include "file.h"

int page=0;

char pageNames[5][14] = {"project","arrange","compose","track edit","operators"};

char helpString[20];

int clamp(int d, int min, int max) {
  const int t = d < min ? min : d;
  return t > max ? max : t;
}

inline int positive_modulo(int i, int n) {
    return (i % n + n) % n;
}

void PageProcess(UIPage* page, UIEvent* event) {
	
	if(event->type == UIMove || event->type == UIMoveRepeat){
		UIGrid *g = &page->grids[page->index];

		g->xPos += event->horizontal;
		g->yPos += event->vertical;

		
		if(g->yPos < 0 || g->yPos >= g->height){
			page->index=clamp(page->index-event->vertical,0,page->length-1);
		}
		//this shouldnt happen on key repeat. ill have to make another case for this
		if(event->type != UIMoveRepeat && g->horizontalLink && (g->xPos < 0 || g->xPos >= g->width)){
			page->index=0;
		}
		g->xPos = clamp(g->xPos,0,g->width-1);
		g->yPos = clamp(g->yPos,0,g->height-1);

		
	}

	UIGrid curGrid = page->grids[page->index];

	switch(event->type){
	case UIChange:
	case UIPlace:
	case UIDelete:
		curGrid.setPtr(curGrid.xPos, curGrid.yPos,*event);
		break;
	default: break;
	}

	for(int i = 0; i < page->length; i++) {
		if(i==page->index){continue;}
		UIGrid g = page->grids[i];
		for(int x=0;x<g.width;x++){
			for(int y=0;y<g.height;y++){
				g.drawPtr(x, y, false);
			}
		}
	}
	for(int x=0;x<curGrid.width;x++){
		for(int y=0;y<curGrid.height;y++){
			curGrid.drawPtr(x, y, (x==curGrid.xPos && y==curGrid.yPos));
		}
	}
	
}

void ProjectDraw(UIEvent* event) {
	PrintColor("theme\nfont\nfile",0,1,2,1,false);

	//Fox
	DrawFox(7,9,0,1,2,3);
	PrintText("kon,1pacto\n\n,0portable",6,15);
	PokeScreen(329,0x74,2,0);
    PokeScreen(330,0x75,2,0);

    PageProcess(&projectPage, event);
}

void ArrangeDraw(UIEvent* event) {

}

void ComposeDraw(UIEvent* event) {

}

void TrackDraw(UIEvent* event) {

}

void OperatorsDraw(UIEvent* event) {

}

void RenderUI() {
	clearGrid(2);

	strcpy(helpString,"gay people rock");

	PrintText(helpString,0,0);
	PrintText(";3,2XXXXXXXXXXXXXXXXXXXX",0,0);
	PrintText(pageNames[page],0,19);
	PrintText(";3,2XXXXXXXXXXXXXXX;0,3pacto",0,19);

	

	int horizontal = IsJustPressed(0) ? -1 : (IsJustPressed(1) ? 1 : 0);
	int vertical = IsJustPressed(2) ? -1 : (IsJustPressed(3) ? 1 : 0);
	int change = horizontal + vertical*16;

	UIEvent event = {UINothing,horizontal,vertical,change};

	bool sPress = IsPressed(6);
	bool zPress = IsPressed(4);
	bool xPress = IsPressed(5);
	bool xJPress = IsJustPressed(5);
	
	if(change!=0){
		event.type = UIMove;
		if (sPress){
			event.type = UIPageChange;
			if(IsJustPressed(0)){page--;}
			if(IsJustPressed(1)){page++;}
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
	}
	
	PrintText(";1,0X",15+page,19);
}