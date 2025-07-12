#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

#include "input.h"

#include "screen.h"

#include "tinydir.h"
#include "sound.h"
#include "synth.h"
#include "ui.h"

#include "file.h"

const uint32_t FILE_VERSION = 0;


char defaultSongPath[PATH_MAX] = "assets/songs";
char currentSongPath[PATH_MAX] = "";
char currentSongName[PATH_MAX] = "";

int clamp(int d, int min, int max) {
  const int t = d < min ? min : d;
  return t > max ? max : t;
}



double fclamp(double d, double min, double max) {
  const double t = d < min ? min : d;
  return t > max ? max : t;
}

int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strcasecmp(str + lenstr - lensuffix, suffix) == 0;
}


double referenceFrequency = 440.0;
int referenceNote = 69;


int configSampleRate = 44100;
int configBufferSize = 512;
char* configTheme = NULL;
char* configFont = NULL;

const char* format = "%i -SAMPLE RATE\n%i -BUFFER SIZE\n%s -THEME\n%s -FONT\n";

int32_t get32(FILE *fp)
{
    int32_t ret = 0;
    ret |= getc(fp);
    ret |= getc(fp) << 8;
    ret |= (uint32_t)getc(fp) << 16;
    ret |= (uint32_t)getc(fp) << 24;
    return ret;
}

/* write a 4-byte little-endian integer */
void put32(uint32_t val, FILE *fp)
{
    putc( val        & 0xff, fp);
    putc((val >>  8) & 0xff, fp);
    putc((val >> 16) & 0xff, fp);
    putc((val >> 24) & 0xff, fp);
}

const int segmentCount = 5;

void LoadSong(char* path){

	FILE *f = fopen(path, "rb");
	
	if (f == NULL)
	{
	    printf("Error opening file when loading song!\n");
	    return;
	}
	lockAudio();

	clearSong(&konAudio);

	uint32_t version = get32(f);
	//do version stuff

	uint8_t fileSegments = getc(f);

	uint32_t arrangeStartOffset = 0;
	uint32_t trackStartOffset = 0;
	uint32_t grooveStartOffset = 0;
	uint32_t instrumentStartOffset = 0;

	uint32_t segmentEnd = 0;

	if (fileSegments > 0)
		arrangeStartOffset = get32(f);
	if (fileSegments > 1)
		trackStartOffset = get32(f);
	if (fileSegments > 2)
		grooveStartOffset = get32(f);
	if (fileSegments > 3)
		instrumentStartOffset = get32(f);
	if (fileSegments > 4)
		segmentEnd = get32(f);

	if (fileSegments > segmentCount){
		fseek(f,4*(fileSegments-segmentCount-1),SEEK_CUR);
	}

	//read project info
	konAudio.bpm = get32(f);
	konAudio.ticksPerStep = getc(f);

	konSetBPM(&konAudio);

	//arrangements
	fseek(f,arrangeStartOffset,SEEK_SET);
	for(int i=0;i<256;i++){
		KonArrangements* arrangement = &konAudio.arrangements[i];
		for (int j=0;j<8;j++){
			arrangement->trackIndexes[j] = getc(f);
		}
		arrangement->jumpIndex = getc(f);
	}

	while(ftell(f)<grooveStartOffset){
		int i = getc(f);
		KonTrack* track = &konAudio.tracks[i];
		track->length = getc(f);
		track->grooveIndex = getc(f);
		track->steps = malloc((track->length+1)*sizeof(KonStep));
		track->temporaryLength = track->length;

		for(int j=0;j<track->length+1;j++){
			KonStep* step = &track->steps[j];
			step->note=getc(f);
			step->instrument=getc(f);
			step->velocity=getc(f);
			step->command=getc(f);
			step->param1=getc(f);
			step->param2=getc(f);
			step->param3=getc(f);
		}
	}

	//groove
	{
		int i = 0;
		while(ftell(f)<instrumentStartOffset){
			konAudio.grooves[i].length=getc(f);
			for(int j=0;j<konAudio.grooves[i].length;j++)
				konAudio.grooves[i].data[j]=getc(f);
			i++;
		}
	}

	while(ftell(f)<segmentEnd){
		int i = getc(f);
		KonInstrument* instrument = &konAudio.instruments[i];

		instrument->route=getc(f);
		instrument->wetDryMix=getc(f);

		for(int j=0;j<sizeof(instrument->name);j++){
			instrument->name[j]=getc(f);
		}
		for(int j=0;j<sizeof(instrument->selectedSynth);j++){
			instrument->selectedSynth[j]=getc(f);
		}
		

		int macroCount = getc(f);


		setInstrument(i,instrument->selectedSynth);

		for(int j=0;j<macroCount;j++){
			KonMacro* macro = &instrument->macros[j];

			
			macro->speed = getc(f);
			macro->loopStart = getc(f);
			macro->loopEnd = getc(f);
			macro->interpolationMode = getc(f);

			macro->min = getc(f);
			macro->max = getc(f);
			macro->oscillates = getc(f);
			

			macro->defaultValue = getc(f);
			macro->length = getc(f);

			macro->data = malloc(sizeof(uint8_t)*macro->length);

			for(int k=0;k<macro->length;k++){
				macro->data[k] = getc(f);
			}

		}

		//effect macros
		// for(int j=32;j<(effectCount+32);j++){
		// 	KonMacro* macro = &instrument->macros[j];
			
		// }

		


	}






	fclose(f);

	unlockAudio();
}

void SaveSong(char* path){
	if (path[0]=='\0'){
		SetContextPage(ContextSaveSong);
		return;
	}

	FILE *f = fopen(path, "wb");

	if (f == NULL)
	{
	    printf("Error opening file when saving song!\n");
	    return;
	}

	//just in case
	lockAudio();

	//saving! yay

	put32(FILE_VERSION,f);

	
	uint32_t arrangeStartOffset = 0;
	uint32_t trackStartOffset = 0;
	uint32_t grooveStartOffset = 0;
	uint32_t instrumentStartOffset = 0;
	uint32_t segmentEnd = 0;


	//number of segments
	putc(segmentCount,f);

	fseek(f,4*segmentCount,SEEK_CUR);

	//song information
	put32(konAudio.bpm,f);
	putc(konAudio.ticksPerStep,f);

	//arrangements
	arrangeStartOffset=ftell(f);

	for (int i=0;i<256;i++){
		KonArrangements* arrangement = &konAudio.arrangements[i];
		for (int j=0;j<8;j++){
			putc(arrangement->trackIndexes[j],f);
		}
		putc(arrangement->jumpIndex,f);
	}

	//tracks
	trackStartOffset=ftell(f);

	for (int i=0;i<255;i++){
		KonTrack* track = &konAudio.tracks[i];
		if(konTrackIsEmpty(track))
			continue;
		//track header
		putc(i,f);
		putc(track->length,f);
		putc(track->grooveIndex,f);

		for(int j=0;j<track->length+1;j++){
			KonStep* step = &track->steps[j];
			putc(step->note,f);
			putc(step->instrument,f);
			putc(step->velocity,f);
			putc(step->command,f);
			putc(step->param1,f);
			putc(step->param2,f);
			putc(step->param3,f);
		}
	}

	//grooves
	grooveStartOffset=ftell(f);

	for(int i=0;i<255;i++){
		int len = konAudio.grooves[i].length;
		putc(len,f);
		for(int j=0;j<len;i++)
			putc(konAudio.grooves[i].data[j],f);
	}

	//instruments
	instrumentStartOffset=ftell(f);

	for (int i=0;i<255;i++){
		KonInstrument* instrument = &konAudio.instruments[i];
		if(instrument->selectedSynth[0] == 0)
			continue;
		putc(i,f);
		putc(instrument->route,f);
		putc(instrument->wetDryMix,f);
		for(int j=0;j<sizeof(instrument->name);j++)
			putc(instrument->name[j],f);
		for(int j=0;j<sizeof(instrument->selectedSynth);j++)
			putc(instrument->selectedSynth[j],f);
		putc(instrument->macroCount,f);
		//macros

		for (int j=0;j<instrument->macroCount;j++){
			KonMacro* macro = &instrument->macros[j];
			
			putc(macro->speed,f);
			putc(macro->loopStart,f);
			putc(macro->loopEnd,f);
			putc(macro->interpolationMode,f);
			
			putc(macro->min,f);		
			putc(macro->max,f);
			putc(macro->oscillates,f);

			putc(macro->defaultValue,f);
			putc(macro->length,f);

			for(int k=0;k<macro->length;k++){
				putc(macro->data[k],f);
			}
		}

	}

	segmentEnd = ftell(f);

	//write offsets
	fseek(f,5,SEEK_SET);

	put32(arrangeStartOffset,f);
	put32(trackStartOffset,f);
	put32(grooveStartOffset,f);
	put32(instrumentStartOffset,f);
	put32(segmentEnd,f);

	fclose(f);

	unlockAudio();
}

void SaveConfig(){

	int size = snprintf(NULL,0,format,configSampleRate,configBufferSize,configTheme,configFont);
	char* buffer = malloc(sizeof(char)*(size+1));

	if(buffer){

		sprintf(buffer,format,configSampleRate,configBufferSize,configTheme,configFont);

		SDL_RWops* rwops = SDL_RWFromFile("config.txt", "w+");

		if(!rwops){
			free(buffer);
			printf("failed to open config to write");
			return;
		}


		SDL_RWwrite(rwops,buffer,sizeof(char),size);

		free(buffer);

		SDL_RWclose(rwops);

	}else{
		printf("failed to save config!");
		return;
	}

	
}

void LoadConfig(){
	printf("** CONFIG LOAD\n\n");
	SDL_RWops* rwops = SDL_RWFromFile("config.txt", "r");
	if(!rwops){
		printf("failed to open config");
		return;
	}

	const int64_t dataSize = SDL_RWsize(rwops);

	char* data = malloc(dataSize);
	if(!data){return;}

	int64_t readAmount = SDL_RWread(rwops, data, dataSize, 1);
	SDL_RWclose(rwops);

	if(readAmount==0){
		printf("failed to read config");
		free(data);
		return;
	}

	char* currentChar = &data[0];

	configSampleRate = strtol(currentChar,NULL,10);
	printf("sample rate: %i\n",configSampleRate);

	currentChar = strchr(currentChar,'\n')+1;

	configBufferSize = strtol(currentChar,NULL,10);
	printf("buffer size: %i\n",configBufferSize);

	for(int i=0;i<2;i++){
		currentChar = strchr(currentChar,'\n')+1;

		char* firstSpace = strchr(currentChar,' ');
		char* firstBreak = strchr(currentChar,'\n');
		if(firstSpace<firstBreak || firstBreak==NULL)
			firstBreak=firstSpace;
		firstBreak-=1;

		int length = firstBreak-currentChar;
		length+=1;
		if(length){
			char* dest = malloc(sizeof(char)*(length+1));
			strncpy(dest,currentChar,length);
			dest[length]='\0';

			if(configTheme==NULL){
				configTheme=dest;
			}else{
				configFont=dest;
			}

			printf("found: %s\n",dest);
		}else{
			printf("failed to find default theme and font\n");
		}
	}
	printf("\n");

	free(data);
}


void SetScale(KonAudio* konAudio, const char* scalePath){
	printf("** SCALE LOAD\n\n");

	SDL_RWops* rwops = SDL_RWFromFile(scalePath, "r");
	if(!rwops){
		printf("failed to open %s",scalePath);
		return;
	}

	const int64_t dataSize = SDL_RWsize(rwops);

	char* data = malloc(dataSize+2);
	if(!data){return;}

	data[dataSize+2] = '\n';
	data[dataSize+2] = '\0';

	int64_t readAmount = SDL_RWread(rwops, data, dataSize, 1);
	SDL_RWclose(rwops);

	if(readAmount==0){
		printf("failed to read %s",scalePath);
		free(data);
		return;
	}

	bool ignoreLine=false;
	bool ignoredDescription=false;

	bool interpretingNote=false;
	char* noteStart;

	long scaleSize=0;
	int scaleIndex=0;
	double* scaleNotes;

	//ouch
	for(int i=0; i<dataSize; i++){

		if(scaleSize && scaleNotes[scaleSize-1]){
			break;
		}

		char currentChar = data[i];
		if(currentChar>127){continue;}
		if(currentChar=='\n'){
			if(ignoreLine){
				ignoreLine=false;
			}else if(!ignoredDescription){
				ignoredDescription=true;
			}else{
				long number = strtol(noteStart,NULL,10);
				scaleNotes[scaleIndex]=(double)number;
				scaleIndex++;
			}
			interpretingNote=false;
			continue;
		}
		if(ignoreLine){continue;}
		if(currentChar=='!'){ignoreLine=true; continue;}
		if(currentChar<33){continue;}
		if(!ignoredDescription){continue;}

		if(!scaleSize){
			scaleSize = strtol(&data[i],NULL,10);
			scaleNotes=calloc(scaleSize,sizeof(double));
			ignoreLine=true;
		}else {

			if(!interpretingNote && currentChar>='0' && currentChar<='9'){
				interpretingNote=true;
				noteStart = &data[i];
			}else{
				if(currentChar=='.'){
					double cents = atof(noteStart);
					scaleNotes[scaleIndex]=pow(2,cents/1200);
					scaleIndex++;
					ignoreLine=true;
				}else if(currentChar=='/'){
					long divedend = strtol(noteStart,NULL,10);
					long divisor = strtol(&data[i+1],NULL,10);

					scaleNotes[scaleIndex]=(double)divedend/(double)divisor;
					scaleIndex++;
					ignoreLine=true;
				}
			}
		}
	}

	free(data);

	lockAudio(); //messing with audio internals

	if(scaleNotes[scaleSize-1]==0){
		printf("not enough notes in %s!\n",scalePath);
		goto bail;
	}

	konAudio->frequencies[referenceNote]=referenceFrequency;
	scaleIndex=0;

	int lastReference = referenceNote;
	for(int i=referenceNote+1; i<254; i++){
		konAudio->frequencies[i]=konAudio->frequencies[lastReference]*scaleNotes[scaleIndex];
		scaleIndex++;

		if(scaleIndex>=scaleSize){
			lastReference=i;
			scaleIndex=0;
		}
	}

	lastReference = referenceNote;
	scaleIndex=scaleSize-2;
	for(int i=referenceNote-1; i>=0; i--){
		double dividend = 1;
		if(scaleIndex>=0){
			dividend=scaleNotes[scaleIndex];
		}

		double ratio=dividend/scaleNotes[scaleSize-1];

		konAudio->frequencies[i]=konAudio->frequencies[lastReference]*ratio;

		if(scaleIndex<0){
			lastReference=i;
			scaleIndex=scaleSize-1;
		}
		scaleIndex--;
	}

	printf("Reference frequency: %f\n",referenceFrequency);
	printf("Reference note: %i\n",referenceNote);
	printf("Loaded scale %s, %i notes read\n\n",scalePath,scaleSize);

	konAudio->notesInScale=scaleSize;
	
	bail:

	free(scaleNotes);

	unlockAudio();
	
}

int FindStringInList(FileResults* list, char* string){
	for(int i=0;i<list->count;i++){
		if(strcmp(list->files[i].name,string)==0){return i;}
	}
	return 0;
}

void ListPath(char* path, char* filter, FileResults* result, int includeDirectories){

	if(result->files != NULL){
		for(int i=0; i<result->count; i++){
			free(result->files[i].name);
			free(result->files[i].path);
		}
		free(result->files);
		result->files=NULL;
	}
	result->count=0;

	tinydir_dir dir;
  	if (tinydir_open_sorted(&dir, path) == -1)
	{
		goto bail;
	}
	
  	result->files = (FileStructure*) calloc(dir.n_files,sizeof(FileStructure));

  	for(int i=0;i<dir.n_files;i++){
  		tinydir_file file;
		if (tinydir_readfile_n(&dir, &file, i) == -1)
		{
			goto bail;
		}
		if((includeDirectories && file.is_dir) || EndsWith(file.name,filter)){
			if (file.is_dir) {
				if (i==0) {
					continue;
				}
			}

			result->files[result->count].name = malloc( (strlen(file.name)+1) * sizeof(char) );
			strcpy(result->files[result->count].name,file.name);

			if (file.is_dir) {
				char* p = result->files[result->count].name;
				for ( ; *p; ++p) *p = tolower(*p);
			}

			result->files[result->count].path = malloc( (strlen(file.path)+1) * sizeof(char) );
			strcpy(result->files[result->count].path,file.path);

			
			result->files[result->count].isDir = file.is_dir;

			result->count++;
		}
	}
	tinydir_close(&dir);
	printf("Listing %s for %s, found %i entries\n",path,filter,result->count);
	return;

	bail:
	printf("failed to load: %s\nEnsure that the path < 260 characters, and includes only ASCII.\n",path);
	return;
}

char* IntToChar(int BMPstring[], char* out, int length){
	for(int i=0;i<length;i++){
		int BMPChar = BMPstring[i];
		char current = ' ';
		if(BMPChar>=1 && BMPChar <= 15){
			current = BMPChar-1+'A';
		}else if(BMPChar>=16 && BMPChar <= 26){
			current = BMPChar-16+'0';
		}else if(BMPChar>=27 && BMPChar <= 51){
			current = BMPChar-27+'a';
		}
	}
	return out;
}