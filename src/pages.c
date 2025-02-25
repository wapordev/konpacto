#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "ui.h"
#include "file.h"
#include "pages.h"
#include "screen.h"

#include "synth.h"

#include "lua.h"

#define CreateGrid(width, height, setPtr, drawPtr)  {width, height, 0, 0, width>1 && height>1, setPtr, drawPtr}

uint8_t noteSymbols[12]={28,53,29,54,30,31,55,32,56,26,52,27};
uint8_t noteNames[254][2];

int themeCount=0;
int themeIndex=0;
char** themeList;

int fontCount=0;
int fontIndex=0;
char** fontList;

int synthCount=0;
char** synthList;

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
	synthList = ListPath("assets/synths",".LUA",&synthCount);

	uint8_t octaveNumber = 0;
	uint8_t notesThisOctave=0;
	for(int i=0;i<254;i++){
		char* noteName = noteNames[i];

		noteName[0]=noteSymbols[notesThisOctave];
		noteName[1]=15+octaveNumber;

		notesThisOctave++;
		octaveNumber+=notesThisOctave/12;
		notesThisOctave%=12;
	}
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

void ScrollGrid(int* scroll, int* yPos, int length, UIEvent event){
	if(event.type==UIMove || event.type==UIMoveRepeat || event.type==UIPageChange){
		int stoppingPoint=(length-0x10);
		int ch = 1;
		if(event.type==UIPageChange){ch=16;}
		if(*yPos==0 && *scroll>0){
			ch=clamp(ch,0,*scroll);
			*scroll-=ch;
			*yPos = 1;
		}
		if(*yPos>7 && *scroll<stoppingPoint){
			ch=clamp(ch,0,stoppingPoint-*scroll);
			*scroll+=ch;
			*yPos = 7;
		}
		return;
	}
}

uint8_t lastArrangeInput=1;
void SetArrange(int xPos, int yPos, UIEvent event){
	
	if(arrangePage.grids[0].xPos<8){
		char string[20] = "channel 0 track";
		string[8]='1'+arrangePage.grids[0].xPos;
		strcpy(helpString,string);
	}else{
		strcpy(helpString,"jump to: pattern 0");
	}

	uint8_t idx = arrangeScroll + yPos;
	uint8_t* row = (uint8_t*)&konAudio.arrangements[idx];

	if( event.type == UIChange ){
		row[xPos]=clamp(row[xPos]+event.change,1,255);
		if(xPos<8){
			lastArrangeInput=clamp(row[xPos],1,255);
		}
	}else if( event.type == UIPlace){
		if(row[xPos]==0){
			if(xPos<8){
				row[xPos]=lastArrangeInput;
			}else{
				row[xPos]=clamp(idx+1,1,255);
			}
		}
	}else if ( event.type == UIDelete ){
		row[xPos]=0;
	}

	ScrollGrid(&arrangeScroll,&arrangePage.grids[0].yPos,256,event);
}

void DrawArrange(int xPos, int yPos, bool selected){
	uint8_t idx = arrangeScroll + yPos;

	uint8_t num = konAudio.arrangements[idx].trackIndexes[xPos];

	if (num) {
		num--;
		int oddColumn = xPos%2;
		HexSelected(num,xPos*2+2,yPos+3,selected,2+oddColumn,3-oddColumn,1,0);
	} else {
		PrintSelected(". ",xPos*2+2,yPos+3,selected,2,3,1,0);
	}
}

uint8_t selectedTrack = 0;
void SetTrackInfo(int xPos, int yPos, UIEvent event){
	switch(xPos){
	case 0:
		strcpy(helpString,"track number");
		if(event.type == UIChange){
			selectedTrack=clamp(selectedTrack+event.change,0,254);
		}
		break;
	case 1:
		if(event.type == UIChange){
			KonTrack* track = &konAudio.tracks[selectedTrack];
			track->temporaryLength=clamp(track->temporaryLength+event.change,0,255);
		}
		strcpy(helpString,"track length");
		break;
	case 2:
		strcpy(helpString,"groove");
		break;
	}
	KonTrack* track = &konAudio.tracks[selectedTrack];

	trackPage.grids[1].height=clamp(track->temporaryLength+1,1,16);

	VerifyTrack(track);
}

void DrawTrackInfo(int xPos, int yPos, bool selected){
	uint8_t num = 0;

	KonTrack* track = &konAudio.tracks[selectedTrack];

	if(xPos==2){

	}else if (xPos){
		num=track->temporaryLength;
	}else{
		num=selectedTrack;
	}

	HexSelected(num,xPos*7+4,1,selected,2,3,1,0);
}

uint8_t lastNote=70;
uint8_t lastInstrument=1;
uint8_t lastVolume=255;
uint8_t lastCommand=1;
uint8_t lastParam=0;
void SetTrackData(int xPos, int yPos, UIEvent event){
	KonTrack* track = &konAudio.tracks[selectedTrack];
	uint8_t scrollPosition=yPos+trackScroll;

	VerifyTrack(track);
	ChangeTrackLength(track);

	uint8_t* step = (uint8_t*)&track->steps[scrollPosition];

	if(event.type == UIChange && event.change){
		int new = step[xPos];
		if(xPos==0){

			if(step[xPos]==0 && event.change<0){
				new = 255;
			}else if(step[xPos]==255){
				if(event.change>0){
					new = 0+event.change;
				}else if(event.change<0){
					new = 255+event.change;
				}
			}else{
				new=clamp(new+event.horizontal+event.vertical*12,1,254);
			}
			//printf("freq: %f\n",konAudio.frequencies[new-1]);
		}else if (xPos==2){
			new=clamp(new+event.horizontal,(new/16)*16,(new/16+1)*16);
			new+=event.vertical*16;
		}else{
			new+=event.change;
		}
		uint8_t clamped = clamp(new,0,255);
		step[xPos] = clamped;

		if(xPos==0){
			if(clamped!=0 && clamped != 255){
				lastNote = clamped;
			}
		}else if(xPos==1){
			lastInstrument=clamp(new,1,255);
		}else if(xPos==2){
			lastVolume=clamp(new,1,255);
		}else if(xPos==3){
			lastCommand=clamp(new,1,255);
		}else {
			lastParam=clamped;
		}

	}else if(event.type == UIDelete){
		uint8_t new=0;
		if(xPos==0 && step[xPos]==0){
			new=255;
		}
		step[xPos] = new;
	}else if(event.type == UIPlace){
		uint8_t new=step[xPos];
		if(step[xPos]==0){
			if(xPos==0){
				new = lastNote;
			}else if(xPos==1){
				new=lastInstrument;
			}else if(xPos==2){
				new=lastVolume;
			}else if(xPos==3){
				new=lastCommand;
			}else {
				new=lastParam;
			}
			step[xPos] = new;
		}else{
			if(xPos==0){
				if(new!=255){lastNote = new;}
			}else if(xPos==1){
				lastInstrument = new;
			}else if(xPos==2){
				lastVolume = new;
			}else if(xPos==3){
				lastCommand = new;
			}else {
				lastParam = new;
			}
		}
	}

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
	ScrollGrid(&trackScroll,&trackPage.grids[1].yPos,track->length+1,event);
}

int trackDataWidths[7] = {0,3,6,9,11,13,15};
void DrawTrackData(int xPos, int yPos, bool selected){
	char out[3] = ". ";

	uint8_t num=0;
	uint8_t scrollPosition=yPos+trackScroll;
	KonTrack* track = &konAudio.tracks[selectedTrack];
	uint8_t commandSet=0;
	if(scrollPosition>track->length){
		num=0;
	}else{
		uint8_t* step = (uint8_t*)&track->steps[scrollPosition];

		commandSet = step[3]!=0;
		num=step[xPos];
	}

	if(xPos==0){
		if(num==255){
			num=0;
			strcpy(out,"--");
		}else if(num){
			num--;
			uint8_t color1 = selected ? 1 : 2;
			uint8_t color2 = selected ? 0 : 3;
			PokeScreen(trackDataWidths[xPos]+2+(yPos+3)*20,noteNames[num][0],color1,color2);
			PokeScreen(trackDataWidths[xPos]+3+(yPos+3)*20,noteNames[num][1],color1,color2);
			return;
		}
	}

	if(num==0 && !(commandSet && xPos>3)){
		PrintSelected(out,trackDataWidths[xPos]+2,yPos+3,selected,2,3,1,0);
	}else{
		if(xPos==1){num--;}

		int oddColumn = 1;

		if(xPos>3){
			oddColumn = xPos%2;
		}


		HexSelected(num,trackDataWidths[xPos]+2,yPos+3,selected,3-oddColumn,2+oddColumn,1,0);
	}
	
}


uint8_t instrumentIndex = 0;


void SetOpSynth(int xPos, int yPos, UIEvent event){
	KonInstrument* instrument = &konAudio.instruments[instrumentIndex];
	if(event.type == UIChange || event.type == UIPlace){
		if(event.change){
			free(synthList);
			synthList = ListPath("assets/synths",".LUA",&synthCount);
		}

		char* name = instrument->selectedSynth;
		int empty = name[0] == '\0';

		unsigned int synthIndex=0;
		if(!empty){
			synthIndex = FindStringInList(synthList,synthCount,name);
		}

		synthIndex=positive_modulo(synthIndex-event.change,synthCount);

		strcpy(instrument->selectedSynth,synthList[synthIndex]);

		char path[64]="assets/synths/";

		strcat(path,synthList[synthIndex]);

		LoadLuaFile(path);


	}else if(event.type == UIDelete){
		strcpy(instrument->selectedSynth,"");
	}
}

void DrawOpSynth(int xPos, int yPos, bool selected){
	KonInstrument* instrument = &konAudio.instruments[instrumentIndex];

	int empty = instrument->selectedSynth[0] == '\0';

	if(empty){
		PrintSelected("none",6,2,selected,2,3,1,0);
	}else{
		PrintSelected(instrument->selectedSynth,6,2,selected,2,3,1,0);
	}
}

void SetOpName(int xPos, int yPos, UIEvent event){
	if(event.type == UIChange){
		if(xPos==0){
			instrumentIndex=clamp(instrumentIndex+event.change,0,254);
		}
	}
}

void DrawOpName(int xPos, int yPos, bool selected){
	if(xPos==0){
		HexSelected(instrumentIndex,5,1,selected,2,3,1,0);
	}
}

void SetOpMacro(int xPos, int yPos, UIEvent event){
}

void DrawOpMacro(int xPos, int yPos, bool selected){
}

void SetOpFlags(int xPos, int yPos, UIEvent event){
}

void DrawOpFlags(int xPos, int yPos, bool selected){
}

void SetOpData(int xPos, int yPos, UIEvent event){
}

void DrawOpData(int xPos, int yPos, bool selected){
}

UIPage projectPage = {0,3,(UIGrid[3]){CreateGrid(1,1,&SetTheme,&DrawTheme),CreateGrid(1,1,&SetFont,&DrawFont),CreateGrid(15,1,&SetFile,&DrawFile)} };

UIPage arrangePage = {0,1,(UIGrid[1]){CreateGrid(9,16,&SetArrange,&DrawArrange)}};

UIPage trackPage = {0,2,(UIGrid[2]){CreateGrid(3,1,&SetTrackInfo,&DrawTrackInfo),CreateGrid(7,16,&SetTrackData,&DrawTrackData)}};

UIPage operatorPage = {0,5,(UIGrid[5]){CreateGrid(2,1,&SetOpName,&DrawOpName),CreateGrid(1,1,&SetOpSynth,&DrawOpSynth),CreateGrid(1,1,&SetOpMacro,&DrawOpMacro),CreateGrid(2,1,&SetOpFlags,&DrawOpFlags),CreateGrid(1,1,&SetOpData,&DrawOpData)}};