#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "input.h"
#include "ui.h"
#include "screen.h"



int page=0;

char pageNames[5][14] = {"project","arrange","compose","track edit","operators"};

char helpString[20];

void ProjectPage() {
	PrintColor("theme\nfont\ncolors",0,1,2,1);

	//Fox
	DrawFox(7,9,0,1,2,3);
	PrintText("kon,1pacto\n\n,0portable",6,15);
	PokeScreen(329,0x74,2,0);
    PokeScreen(330,0x75,2,0);
}

void ArrangePage() {

}

void ComposePage() {

}

void TrackPage() {

}

void OperatorsPage() {

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
	case 0: ProjectPage(); break;
	case 1: ArrangePage(); break;
	case 2: ComposePage(); break;
	case 3: TrackPage(); break;
	case 4: OperatorsPage(); break;
	}
	
	PrintText(";1,0X",15+page,19);
}