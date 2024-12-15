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

void PageProcess(UIPage page) {

	int horizontal = IsJustPressed(0) ? -1 : (IsJustPressed(1) ? 1 : 0);
	int vertical = IsJustPressed(2) ? -1 : (IsJustPressed(3) ? 1 : 0);
	int change = horizontal + vertical*16;

	for(int i = 0; i < page.length; i++) {
		UIGrid grid = page.grids[i];
		grid.drawPtr(grid.xPos, grid.yPos, i==page.pointer);
	}
	if(change){page.grids[0].setPtr(0,0,change);}
}

void ProjectDraw() {
	PrintColor("theme\nfont",0,1,2,1);

	PrintText(CharToBMP(GetPath(),true),0,5);

	//Fox
	DrawFox(7,9,0,1,2,3);
	PrintText("kon,1pacto\n\n,0portable",6,15);
	PokeScreen(329,0x74,2,0);
    PokeScreen(330,0x75,2,0);

    PageProcess(projectPage);
}

void ArrangeDraw() {

}

void ComposeDraw() {

}

void TrackDraw() {

}

void OperatorsDraw() {

}

void RenderUI() {
	clearGrid(2);

	if(IsJustPressed(0)){page--;}
	if(IsJustPressed(1)){page++;}
	

	strcpy(helpString,"gay people rock");

	PrintText(helpString,0,0);
	PrintText(";3,2XXXXXXXXXXXXXXXXXXXX",0,0);
	PrintText(pageNames[page],0,19);
	PrintText(";3,2XXXXXXXXXXXXXXX;0,3pacto",0,19);

	switch(page){
	case 0: ProjectDraw(); break;
	case 1: ArrangeDraw(); break;
	case 2: ComposeDraw(); break;
	case 3: TrackDraw(); break;
	case 4: OperatorsDraw(); break;
	}
	
	PrintText(";1,0X",15+page,19);
}