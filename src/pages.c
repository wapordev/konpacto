#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#endif

#include "ui.h"
#include "file.h"
#include "pages.h"
#include "screen.h"
#include "sound.h"

#include "synth.h"



#include "lua.h"

#define CreateGrid(width, height, setPtr, drawPtr)  {width, height, 0, 0, width>1 && height>1, setPtr, drawPtr}

uint8_t noteSymbols[12]={28,53,29,54,30,31,55,32,56,26,52,27};
uint8_t noteNames[254][2];

int themeIndex=0;
FileResults themeList;

int fontIndex=0;
FileResults fontList;

FileResults synthList;

void MatchTheme(char* themeName){
	ListPath("assets/palettes",".PNG",&themeList,0);
	for(int i=0;i<themeList.count;i++){
		if(strcmp(themeList.files[i].name,themeName)==0){
			themeIndex=i;
			return;
		}
	}
}

void SetTheme(int xPos, int yPos, UIEvent event) {
	strcpy(helpString,"4x1png assets/themes");
	if(event.type!=UIChange){return;}
	int change = event.change;

	MatchTheme(configTheme);

	themeIndex=positive_modulo(themeIndex+change,themeList.count);
	ChangeTheme(themeList.files[themeIndex].name);

	free(configTheme);
	int length=strlen(themeList.files[themeIndex].name);
	configTheme = malloc(length+1);
	strcpy(configTheme,themeList.files[themeIndex].name);
	configTheme[length]='\0';

	SaveConfig();
}

void DrawTheme(int xPos, int yPos, bool selected) {
	PrintSelected(themeList.files[themeIndex].name,6,5,selected,2,3,1,0);

	if(strcmp(themeList.files[themeIndex].name,"Kleahpacto.png")==0){
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

void MatchFont(char* fontName){
	ListPath("assets/fonts",".BMP",&fontList,0);
	for(int i=0;i<fontList.count;i++){
		if(strcmp(fontList.files[i].name,fontName)==0){
			fontIndex=i;
			return;
		}
	}
}

void SetFont(int xPos, int yPos, UIEvent event) {
	strcpy(helpString,"16x8chr assets/fonts");
	if(event.type!=UIChange){return;}
	int change = event.change;

	MatchFont(configFont);

	fontIndex=positive_modulo(fontIndex+change,fontList.count);
	ChangeFont(fontList.files[fontIndex].name);

	free(configFont);
	int length=strlen(fontList.files[fontIndex].name);
	configFont = malloc(length+1);
	strcpy(configFont,fontList.files[fontIndex].name);
	configFont[length]='\0';

	SaveConfig();
}

void DrawFont(int xPos, int yPos, bool selected) {
	PrintSelected(fontList.files[fontIndex].name,5,6,selected,2,3,1,0);
}

void InitializePages(){
	printf("** FILE LISTS\n\n");

	MatchTheme(configTheme);
	ChangeTheme(themeList.files[themeIndex].name);
	//fontList = ListPath("assets/fonts",".BMP",&fontCount);
	MatchFont(configFont);
	ChangeFont(fontList.files[fontIndex].name);
	
	ListPath("assets/synths",".LUA",&synthList,0);

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
	printf("\n");
}

void SetBPM(int xPos, int yPos, UIEvent event){
	if (xPos == 0 ){
		strcpy(helpString,"beats per minute");
		if (event.type == UIChange)
			konAudio.bpm = clamp(konAudio.bpm+event.horizontal+(event.vertical*10),1,999);
	}else{
		strcpy(helpString,"ticks per step");
		if (event.type == UIChange)
			konAudio.ticksPerStep = clamp(konAudio.ticksPerStep+event.change,1,100);
	}
	if (event.type == UIChange)
		konSetBPM(&konAudio);
}

void DrawBPM(int xPos, int yPos, bool selected){
	if(xPos==0){
		DecSelected(konAudio.bpm,4,1,selected,2,3,1,0);
	}else{
		DecSelected(konAudio.ticksPerStep,8,1,selected,2,3,1,0);
	}
	
}

void SetSave(int xPos, int yPos, UIEvent event){
	
	if(xPos==0) {
		strcpy(helpString,"save song");
		if(event.type == UIPlace)
			SaveSong(currentSongPath);
	}else if (xPos==1) {
		strcpy(helpString,"save to new file");
		if(event.type == UIPlace)
			SetContextPage(ContextSaveSong);
	}else {
		strcpy(helpString,"load or clear song");
		if(event.type == UIPlace)
			SetContextPage(ContextLoadSong);
	}

}

void DrawSave(int xPos, int yPos, bool selected){
	if(xPos == 0){
		PrintSelected("save",1,3,selected,2,3,1,0);
	}else if (xPos == 1) {
		if(selected){
			PrintColor("XXXXXas",1,3,1,0,false);
		}else{
			PrintText("as",6,3);
		}
	}else{
		PrintSelected("load",9,3,selected,2,3,1,0);
	}
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
		KonTrack* track = &konAudio.tracks[selectedTrack];

		if(event.type == UIChange){
			track->grooveIndex=clamp(track->grooveIndex+event.change,0,254);
		}

		

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
		num=track->grooveIndex;
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

uint8_t lastGroove=6;

void SetTrackData(int xPos, int yPos, UIEvent event){
	KonTrack* track = &konAudio.tracks[selectedTrack];
	uint8_t scrollPosition=yPos+trackScroll;

	VerifyTrack(track);
	ChangeTrackLength(track);

	uint8_t* step = (uint8_t*)&track->steps[scrollPosition];

	//messy
	if(xPos == 7){
		if(track->grooveIndex){
			KonGroove* currentGroove = &konAudio.grooves[track->grooveIndex-1];

			if(event.type == UIPlace){
				if(scrollPosition >= currentGroove->length){
					currentGroove->length = scrollPosition+1;
					for(int i=0;i<scrollPosition;i++){
						if(currentGroove->data[i]==0)
							currentGroove->data[i]=konAudio.ticksPerStep;
					}
				}
				if(currentGroove->data[scrollPosition]==0){
					currentGroove->data[scrollPosition]=lastGroove;
				}else{
					lastGroove=currentGroove->data[scrollPosition];
				}
			}else if(event.type == UIChange){
				currentGroove->data[scrollPosition]=clamp(currentGroove->data[scrollPosition]+event.change,0,255);
			}else if(event.type == UIDelete){
				if(scrollPosition < currentGroove->length)
					currentGroove->length=scrollPosition;
			}
		}else{
			trackPage.grids[1].xPos=6;
		}
	}else{
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
					new=clamp(new+event.horizontal+event.vertical*konAudio.notesInScale,1,254);
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
	}

	switch(xPos){
	case 0:
		strcpy(helpString,"note");
		break;
	case 1:
		strcpy(helpString,"instrument ");
		int i = step[xPos];
		if (i){
			char* instrumentName = konAudio.instruments[i-1].name;
			strcpy(helpString+11,instrumentName);
		}
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
	case 7:
		strcpy(helpString,"groove");
		break;
	}
	ScrollGrid(&trackScroll,&trackPage.grids[1].yPos,track->length+1,event);
}

int trackDataWidths[8] = {0,3,6,9,11,13,15,16};
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

	if(num==0 && !(commandSet && xPos>3) && (xPos != 7)){
		PrintSelected(out,trackDataWidths[xPos]+2,yPos+3,selected,2,3,1,0);
	}else if (xPos != 7){
		if(xPos==1){num--;}

		int oddColumn = 1;

		if(xPos>3){
			oddColumn = xPos%2;
		}


		HexSelected(num,trackDataWidths[xPos]+2,yPos+3,selected,3-oddColumn,2+oddColumn,1,0);
	}else if (trackPage.grids[1].xPos == 7) {
		if(track->grooveIndex){
			KonGroove* currentGroove = &konAudio.grooves[track->grooveIndex-1];


			if(yPos==0){
				PrintColor("!!",18,2,2,0,false);
				if(track->length<currentGroove->length)
					PrintColor("..",18,4+track->length,2,0,false);
			}

			if(scrollPosition<currentGroove->length){
				HexSelected(currentGroove->data[scrollPosition],18,3+yPos,selected,0,3,1,0);
			}else{
				PrintSelected("  ",18,3+yPos,selected,0,2,1,2);
			}
		}
	}
	
}


uint8_t instrumentIndex = 0;


void SetOpSynth(int xPos, int yPos, UIEvent event){
	strcpy(helpString,"synth generator");

	KonInstrument* instrument = &konAudio.instruments[instrumentIndex];
	if(event.type == UIChange || event.type == UIPlace ){
		if(event.change){
			ListPath("assets/synths",".LUA",&synthList,0);
		}

		char* name = instrument->selectedSynth;
		int empty = name[0] == '\0';

		unsigned int synthIndex=0;
		if(!empty){
			synthIndex = FindStringInList(&synthList,name);
		}

		synthIndex=positive_modulo(synthIndex-event.change,synthList.count);

		strcpy(instrument->selectedSynth,synthList.files[synthIndex].name);

		setInstrument(instrumentIndex,synthList.files[synthIndex].name);
		
	}else if(event.type == UIDelete){
		strcpy(instrument->selectedSynth,"");
		for(int i=3;i<instrument->macroCount;i++){
			if(instrument->macros[i].length){
				instrument->macros[i].length=0;
				instrument->macros[i].data=NULL;
				free(instrument->macros[i].data);
			}
		}
		instrument->macroCount=0;
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

void SetOpEffect(int xPos, int yPos, UIEvent event){
	strcpy(helpString,"synth effect");

}

void DrawOpEffect(int xPos, int yPos, bool selected){
	PrintSelected("none",3,3,selected,2,3,1,0);
}

void SetOpName(int xPos, int yPos, UIEvent event){
	if(xPos==0){
		strcpy(helpString,"instrument index");
		if(event.type == UIChange)
			instrumentIndex=clamp(instrumentIndex+event.change,0,254);
	}else{
		KonInstrument* instrument = &konAudio.instruments[instrumentIndex];
		if(xPos==1){
			strcpy(helpString,"route to ");
			if (instrument->route){
				char* instrumentName = konAudio.instruments[instrument->route-1].name;
				strcpy(helpString+9,instrumentName);
			}
			if(event.type == UIChange)
				instrument->route=clamp(instrument->route+event.change,0,255);
		}else if(xPos==2){
			strcpy(helpString,"wet/dry");
			if(event.type == UIChange)
				instrument->wetDryMix=clamp(instrument->wetDryMix+event.change,0,255);
		}else{
			strcpy(helpString,"instrument name");
			HandleTextInput(instrument->name,xPos-3,event,13,1);
		}
	}
}

void DrawOpName(int xPos, int yPos, bool selected){
	if(xPos==0){
		HexSelected(instrumentIndex,2,1,selected,2,3,1,0);
	}else{
		KonInstrument* instrument = &konAudio.instruments[instrumentIndex];
		if(xPos==1){
			if(instrument->route==0){
				PrintSelected("no",5,1,selected,2,3,1,0);
			}else{
				HexSelected(instrument->route-1,5,1,selected,2,3,1,0);
			}
			
		}else if(xPos==2){
			HexSelected(instrument->wetDryMix,8,1,selected,2,3,1,0);
		}
	}
}


void SetOpMacro(int xPos, int yPos, UIEvent event){
	strcpy(helpString,"param modulation");
	
	KonInstrument* instrument = &konAudio.instruments[instrumentIndex];
	if(event.type == UIMove || event.type == UIMoveRepeat){
		instrument->selectedMacro=clamp(instrument->selectedMacro+event.horizontal,0,instrument->macroCount-1);
	}
}

void DrawOpMacro(int xPos, int yPos, bool selected){
	KonInstrument* instrument = &konAudio.instruments[instrumentIndex];
	KonMacro* macro = &instrument->macros[instrument->selectedMacro];

	if(instrument->macroCount){
		int max = instrument->macroCount-1;
		int len = strlen(macro->name);
		int x = clamp(instrument->selectedMacro,0,(instrument->selectedMacro==max ? 16 : 15)-len);
		PrintSelected(macro->name,2+x,4,selected,2,3,1,0);
		//HexSelected(macro->defaultValue,3,5,selected,2,3,1,0);
		if(x)
			PrintSelected("<",x,4,selected,0,3,0,1);
		if(instrument->selectedMacro!=max)
			PrintSelected(">",3+x+len,4,selected,0,3,0,1);
		PlaceSelected(0x4e,1+x,4,selected,0,2,0,1);
		PlaceSelected(0x7f,2+x+len,4,selected,0,2,0,1);
	}else{
		PrintSelected("no synth selected!",1,4,selected,2,3,1,0);
		PlaceSelected(0x4e,0,4,selected,0,2,0,1);
		PlaceSelected(0x7f,19,4,selected,0,2,0,1);
	}
	

	
}

//this one especially highlights the flaws of the UI system.
//sidenote; people say just use switch case instead of if chains, but its more of a structural issue?
//switch case is just a line longer for compiling to the same thing
void SetOpFlags(int xPos, int yPos, UIEvent event){
	KonInstrument* instrument = &konAudio.instruments[instrumentIndex];
	KonMacro* macro = &instrument->macros[instrument->selectedMacro];

	if(!instrument->macroCount)
		return;

	if(yPos==0){
		if(xPos==0){
			strcpy(helpString,"speed (in ticks)");
		}else if(xPos==1){
			strcpy(helpString,"loop begin point");
		}else if(xPos==2){
			strcpy(helpString,"loop end point");
		}else if(xPos==3){
			if(macro->interpolationMode){
				strcpy(helpString,"linear interpolation");
			}else{
				strcpy(helpString,"nearest neighbor");
			}
		}
	}else{
		if(xPos==0){
			strcpy(helpString,"not implemented");
		}else if(xPos==1){
			strcpy(helpString,"macro range minimum");
		}else if(xPos==2){
			strcpy(helpString,"macro range maximum");
		}else if(xPos==3){
			if(macro->oscillates==0){
				strcpy(helpString,"reset on note");
			}else{
				strcpy(helpString,"persist/oscillate");
			}
		}
	}

	if(event.type==UIChange){
		if(yPos==0){
			if(xPos==0){
				macro->speed=clamp(event.change+macro->speed,0,255);
			}else if(xPos==1){
				macro->loopStart=clamp(event.change+macro->loopStart,0,macro->length);
			}else if(xPos==2){
				macro->loopEnd=clamp(event.change+macro->loopEnd,0,macro->length);
			}else{
				macro->interpolationMode=clamp(event.change+macro->interpolationMode,0,1);
			}
		}else{
			if(xPos==0){
				
			}else if(xPos==1){
				macro->min=clamp(event.change+macro->min,0,255);
			}else if(xPos==2){
				macro->max=clamp(event.change+macro->max,0,255);
			}else{
				macro->oscillates=clamp(event.change+macro->oscillates,0,1);
			}
		}
	}
}

//ditto. would want more complex structure next time/in the future
//more modular with properties and code sharing per tile
void DrawOpFlags(int xPos, int yPos, bool selected){
	KonInstrument* instrument = &konAudio.instruments[instrumentIndex];
	KonMacro* macro = &instrument->macros[instrument->selectedMacro];

	if(!instrument->macroCount)
		return;

	if(yPos==0){
		if(xPos==0){
			HexSelected(macro->speed,3,6,selected,2,3,1,0);
		}else if(xPos==1){
			HexSelected(macro->loopStart,8,6,selected,2,3,1,0);
		}else if(xPos==2){
			HexSelected(macro->loopEnd,13,6,selected,2,3,1,0);
		}else{
			char* str = "n";
			if(macro->interpolationMode)
				str = "l";
			PrintSelected(str,18,6,selected,2,3,1,0);
		}
	}else{
		if(xPos==0){
			PrintText(",1s    b    e    i\n     m    x    o",1,6);
			PrintSelected("temp",1,7,selected,2,3,1,0);
		}else if(xPos==1){
			HexSelected(macro->min,8,7,selected,2,3,1,0);
		}else if (xPos==2){
			HexSelected(macro->max,13,7,selected,2,3,1,0);
		}else if (xPos==3){
			char* str = "x";
			if(macro->oscillates)
				str = "O";
			PrintSelected(str,18,7,selected,2,3,1,0);
		}
	}
}

void SetOpData(int xPos, int yPos, UIEvent event){
	KonInstrument* instrument = &konAudio.instruments[instrumentIndex];
	KonMacro* macro = &instrument->macros[instrument->selectedMacro];

	if(!instrument->macroCount)
		return;

	strcpy(helpString,"macro data");


	if(event.type==UIMove || event.type==UIMoveRepeat){
		macro->selectedStep=clamp(macro->selectedStep+event.horizontal,0,255);
	}

	int realPosition = macro->selectedStep;

	if(event.type == UIPlace){
		if(macro->length==0){
			macro->data = calloc(realPosition+1,sizeof(uint8_t));
			for(int i=0;i<realPosition+1;i++){
				macro->data[i] = macro->defaultValue;
			}
			macro->length=realPosition+1;
		}else if(realPosition>=macro->length){
			macro->data = realloc(macro->data,sizeof(uint8_t)*(realPosition+1));
			for(int i=macro->length;i<realPosition+1;i++){
				macro->data[i]=macro->data[macro->length-1];
			}
			macro->length=realPosition+1;
		}
	}

	if(event.type == UIDelete){
		if(macro->length==0)
			return;
		if(realPosition<macro->length){
			if(realPosition==0){
				macro->length=0;
				free(macro->data);
				return;
			}
			macro->data = realloc(macro->data, sizeof(uint8_t)*(realPosition));
			macro->length=realPosition;
		}
	}

	if(event.type == UIChange){
		if(macro->length==0)
			return;
		if(realPosition>=macro->length)
			return;
		int change = event.change;
		if(instrument->selectedMacro==0){
			change=event.horizontal+event.vertical*konAudio.notesInScale;
		}
		macro->data[realPosition]=clamp(macro->data[realPosition]+change,0,255);
	}
}

void DrawOpData(int xPos, int yPos, bool selected){
	KonInstrument* instrument = &konAudio.instruments[instrumentIndex];
	KonMacro* macro = &instrument->macros[instrument->selectedMacro];

	if(!instrument->macroCount)
		return;

	int opDataScroll=clamp(macro->selectedStep-8,0,238);
	int selectedStep = opDataScroll+xPos;
	selected=selectedStep==macro->selectedStep;

	int len=macro->length;

	int dataRaw=0;
	int data=0;


	if(selectedStep<len || selectedStep==0){
		if(len){
			dataRaw=macro->data[selectedStep];
		}else{
			dataRaw=macro->defaultValue;
		}
		data=(dataRaw*3)/16;
		data+=1;
	}

	if(data){
		BarSelected(xPos+1,16,data,selected,0,3,0,1);
	}else{
		PrintSelected(".",xPos+1,16,selected,0,3,0,1);
	}
	
	if(macro->loopEnd>=macro->loopStart){
		if(selectedStep==macro->loopStart){
			if(selectedStep==macro->loopEnd){
				PrintSelected("B",xPos+1,8,0,2,3,2,1);
			}else{
				PrintSelected("(",xPos+1,8,0,2,3,2,1);
			}
		}else if(selectedStep==macro->loopEnd){
			PrintSelected(")",xPos+1,8,0,2,3,2,1);
		}
	}

	if(opDataScroll){
		PrintSelected("<",0,16,0,2,3,2,1);
	}
	if(opDataScroll<macro->length-18){
		PrintSelected(">",19,16,0,2,3,2,1);
	}

	if( selected ){
		int x=macro->selectedStep-opDataScroll;

		if(data){
			HexSelected(dataRaw,clamp(x,0,16)+1,17,selected,3,0,1,0);
		}else{
			PrintSelected("nl",clamp(x,0,16)+1,17,selected,3,0,1,0);
		}
		
	}
}

void SetFileName(int xPos, int yPos, UIEvent event){
	HandleTextInput(currentSongName,xPos,event,0,2);
}

void DrawFileName(int xPos, int yPos, bool selected){
	
}

void SetFileConfirm(int xPos, int yPos, UIEvent event){
	if(event.type == UIPlace){
		if(xPos == 0){
			
			int len = strlen(fileBrowserPath);
			if(len<PATH_MAX-32){
				strcpy(selectedFile,fileBrowserPath);
				selectedFile[len]='\\';
				strncpy_s(selectedFile+len+1,PATH_MAX-len-1,currentSongName,PATH_MAX);

				char* lastSlash = strchr(selectedFile,'/');
				char* lastBackslash = strchr(selectedFile,'\\');
				if (lastBackslash > lastSlash)
					lastSlash = lastBackslash;
				char* lastDot = strchr(selectedFile,'.');
				if(lastDot< lastBackslash){
					len+=strlen(currentSongName)+1;
					strncpy_s(selectedFile+len,PATH_MAX-len,".kpt",PATH_MAX);
				}

				printf("Targetting %s to save.\n",selectedFile);

				strcpy(selectedFileName,currentSongName);
				

				QuitContext(1);
			}
			
		}else{
			QuitContext(0);
		}
	}
}

void DrawFileConfirm(int xPos, int yPos, bool selected){
	if(xPos==0){
		PrintSelected("confirm",1,3,selected,2,3,1,0);
	}else{
		PrintSelected("cancel",9,3,selected,2,3,1,0);
	}
}

void SetFileCancel(int xPos, int yPos, UIEvent event){
	if(event.type != UIPlace)
		return;
	if(xPos==0){
		lockAudio();
		clearSong(&konAudio);
		unlockAudio();
		QuitContext(0);
	}else{
		QuitContext(0);
	}
	
}

void DrawFileCancel(int xPos, int yPos, bool selected){
	if(xPos==0){
		PrintSelected("new",1,1,selected,2,3,1,0);
	}else{
		PrintSelected("cancel",5,1,selected,2,3,1,0);
	}
}


//stupid, rework later possibly. see next comment
int fileBrowserOffset=0;
char fileBrowserPath[PATH_MAX] = "assets/songs";
char selectedFile[PATH_MAX] = "";
char selectedFileName[PATH_MAX] = "";
char fileBrowserExtension[5] = ".KPT";
FileResults fileBrowserEntries;
int fileBrowserScroll = 0;

void ReloadFileBrowse(int offset){
	ListPath(fileBrowserPath,fileBrowserExtension,&fileBrowserEntries,1);
	strcpy(fileBrowserExtension,".KPT");
	fileBrowserOffset = offset;
}

void SetFileBrowse(int xPos, int yPos, UIEvent event){
	ScrollGrid(&fileBrowserScroll,&saveFilePage.grids[2].yPos,fileBrowserEntries.count,event);
	if (saveFilePage.grids[2].yPos >= fileBrowserEntries.count && fileBrowserEntries.count > 0){
		saveFilePage.grids[2].yPos = fileBrowserEntries.count-1;
	}
	//ok this is really stupid, but i had never considered before using an element for 2 pages.
	//there is not really the structure for this now
	ScrollGrid(&fileBrowserScroll,&loadFilePage.grids[1].yPos,fileBrowserEntries.count,event);
	if (loadFilePage.grids[1].yPos >= fileBrowserEntries.count && fileBrowserEntries.count > 0){
		loadFilePage.grids[1].yPos = fileBrowserEntries.count-1;
	}


	if(event.type != UIPlace)
		return;
	
	int realY = yPos+fileBrowserScroll;

	if(realY < fileBrowserEntries.count){
		if(fileBrowserEntries.files[realY].isDir){
			char newPath[PATH_MAX];
			realpath(fileBrowserEntries.files[realY].path,newPath);
			ListPath(newPath,fileBrowserExtension,&fileBrowserEntries,1);
			if(fileBrowserEntries.count==0){
				ListPath(fileBrowserPath,fileBrowserExtension,&fileBrowserEntries,1);
			}else{
				strcpy(fileBrowserPath,newPath);
				fileBrowserScroll=0;
			}
		}else{
			strcpy(selectedFile,fileBrowserEntries.files[realY].path);
			strcpy(selectedFileName,fileBrowserEntries.files[realY].name);
			QuitContext(1);
		}
	}

	if(fileBrowserEntries.files == NULL){
		ListPath(fileBrowserPath,fileBrowserExtension,&fileBrowserEntries,1);
	}
}

void DrawFileBrowse(int xPos, int yPos, bool selected){

	

	if(yPos+fileBrowserScroll < fileBrowserEntries.count){
		PrintSelected(fileBrowserEntries.files[yPos+fileBrowserScroll].name,0,4+yPos+fileBrowserOffset,selected,0,3,1,0);
	}
}

UIPage projectPage = {0,4,(UIGrid[5]){CreateGrid(2,1,&SetBPM,&DrawBPM),CreateGrid(3,1,&SetSave,&DrawSave),CreateGrid(1,1,&SetTheme,&DrawTheme),CreateGrid(1,1,&SetFont,&DrawFont)} };

UIPage arrangePage = {0,1,(UIGrid[1]){CreateGrid(9,16,&SetArrange,&DrawArrange)}};

UIPage trackPage = {0,2,(UIGrid[2]){CreateGrid(3,1,&SetTrackInfo,&DrawTrackInfo),CreateGrid(8,16,&SetTrackData,&DrawTrackData)}};

UIPage operatorPage = {0,5,(UIGrid[5]){CreateGrid(10,1,&SetOpName,&DrawOpName),CreateGrid(1,1,&SetOpSynth,&DrawOpSynth),CreateGrid(1,1,&SetOpMacro,&DrawOpMacro),CreateGrid(4,2,&SetOpFlags,&DrawOpFlags),CreateGrid(18,1,&SetOpData,&DrawOpData)}};

UIPage saveFilePage = {0,3,(UIGrid[3]){CreateGrid(20,1,&SetFileName,&DrawFileName),CreateGrid(2,1,&SetFileConfirm,&DrawFileConfirm),CreateGrid(1,13,&SetFileBrowse,&DrawFileBrowse)}};

UIPage loadFilePage = {0,2,(UIGrid[2]){CreateGrid(2,1,&SetFileCancel,&DrawFileCancel),CreateGrid(1,15,&SetFileBrowse,&DrawFileBrowse)}};