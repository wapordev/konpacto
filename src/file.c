#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <stdbool.h>

#include "input.h"

#include "screen.h"

char* GetPath(){
	return SDL_GetBasePath();
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

char* CharToBMP(char* string, bool filter){

	char filterEnd = filter ? 'Z' : 'O';

	for(int i=0;i<128;i++){
		char current = string[i];
		if(current=='\0'){break;}
		if(current >= 'a' && current <= 'z'){continue;}
		if(current >= 'A' && current <= filterEnd){
			if(filter){string[i]=current-'A'+'a';}  
			continue;
		}
		if(current=='~'){string[i] = 'A'; continue;}
		if(current=='!'){string[i] = 'B'; continue;}
		if(current=='/'){string[i] = 'C'; continue;}
		if(current=='\\'){string[i] = 'C'; continue;}
		if(current=='-'){string[i] = 'D'; continue;}
		if(current=='.'){string[i] = 'E'; continue;}
		if(current==':'){string[i] = 'F'; continue;}
		if(current=='\''){string[i] = 'G'; continue;}
		if(current=='('){string[i] = 'H'; continue;}
		if(current==')'){string[i] = 'I'; continue;}
		if(current=='<'){string[i] = 'J'; continue;}
		if(current=='>'){string[i] = 'K'; continue;}
		//if(current==''){ continue;}
		//if(current==''){ continue;}
		//if(current==''){ continue;}
		//if(current==''){ continue;}
		string[i] = ' ';
	}
	return string;
}