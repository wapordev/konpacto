#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <stdbool.h>
#include <math.h>

#include "input.h"

#include "screen.h"

#include "tinydir.h"
#include "sound.h"
#include "synth.h"

#include "file.h"

int clamp(int d, int min, int max) {
  const int t = d < min ? min : d;
  return t > max ? max : t;
}

double lerp(double a, double b, double f) 
{
    return (a * (1.0 - f)) + (b * f);
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
	printf("Loaded scale %s, %i notes read\n",scalePath,scaleSize);

	konAudio->notesInScale=scaleSize;
	
	bail:

	free(scaleNotes);

	unlockAudio();
	
}

int FindStringInList(char** list, int count, char* string){
	for(int i=0;i<count;i++){
		if(strcmp(list[i],string)==0){return i;}
	}
	return 0;
}

//
char** ListPath(char* path, char* filter, int* outLength){



	//PrintText(CharToBMP(fontNames[0],true),0,5);
	tinydir_dir dir;
  	if (tinydir_open_sorted(&dir, path) == -1)
	{
		goto bail;
	}
	
	char** nameListPtr;
  	nameListPtr = (char**) malloc(dir.n_files * sizeof(int*));

  	int y=0;

  	for(int i=0;i<dir.n_files;i++){
  		tinydir_file file;
		if (tinydir_readfile_n(&dir, &file, i) == -1)
		{
			for (int j=0;j<dir.n_files;j++){
				if(nameListPtr[j] != NULL){
					free(nameListPtr[j]);
				}
			}
			free(nameListPtr);
			goto bail;
		}
		if(EndsWith(file.name,filter)){
			nameListPtr[y] = malloc( (strlen(file.name)+1) * sizeof(char) );

			strcpy(nameListPtr[y],file.name);

			
			y++;
		}
	}
	tinydir_close(&dir);
	*outLength = y;
	printf("Listing %s for %s, found %i entries\n",path,filter,y);
	return nameListPtr;

	bail:
	printf("failed to load: %s\nEnsure that the path < 260 characters, and includes only ASCII.\n",path);
	return NULL;
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