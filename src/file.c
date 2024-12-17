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

void ListPath(){

	//PrintText(CharToBMP(fontNames[0],true),0,5);
	tinydir_dir dir;
  	tinydir_open_sorted(&dir, "assets");
  	int y=0;
  	for(int i=0;i<dir.n_files;i++){
  		tinydir_file file;
		tinydir_readfile_n(&dir, &file, i);
		if(EndsWith(file.name,".BMP")){
			PrintText(file.name,0,5+y);
			y++;
		}
	}
	tinydir_close(&dir);
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