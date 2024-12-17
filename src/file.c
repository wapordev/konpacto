#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <stdbool.h>

#include "input.h"

#include "screen.h"

#include "tinydir.h"

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