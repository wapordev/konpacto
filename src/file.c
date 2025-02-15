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

void SetScale(KonAudio* konAudio, const char* scalePath){
	SDL_RWops* rwops = SDL_RWFromFile(scalePath, "r");
	if(!rwops){return;}

	const int64_t dataSize = SDL_RWsize(rwops);

	char* data = malloc(dataSize+2);
	if(!data){return;}

	data[dataSize+2] = '\n';
	data[dataSize+2] = '\0';

	SDL_RWread(rwops, data, dataSize, 1);

	SDL_RWclose(rwops);

	bool ignoreLine=false;
	bool ignoredDescription=false;

	bool interpretingNote=false;
	char* noteStart;

	long scaleSize=0;
	int scaleIndex=0;
	double* scaleNotes;

	//ouch
	for(int i=0; i<dataSize; i++){
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
			scaleNotes=malloc(sizeof(double)*scaleSize);
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

	for(int i=0; i<scaleSize; i++){
		printf("%f\n",scaleNotes[i]);
	}


	konAudio->frequencies[referenceNote]=referenceFrequency;
	scaleIndex=0;

	printf("frequencies!!: \n\n");
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
			scaleIndex=scaleSize-2;
		}
		scaleIndex--;
	}


	for(int i=0; i<254; i++){
		printf("%f\n",konAudio->frequencies[i]);
	}

	free(scaleNotes);
	

	unlockAudio();
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
	return nameListPtr;

	bail:
	PrintText("failed to load!",0,5);
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