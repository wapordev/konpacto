#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "input.h"

SDL_AudioDeviceID deviceId;


int32_t x = 0;

void callback(void *userdata, Uint8 * stream, int len){
	if ( len == 0 )
    return;


	//printf("whell %i",len);
	//exit(0);
	int32_t* pointer = (int32_t*)stream;

	for(int i=0; i<len/4; i+=2){
		if(IsPressed(4)){
			x+=20967296;
		}else{
			x+=10967296;
		}
		
		int32_t sample = x*.0625;
		pointer[i]=sample;
		pointer[i+1]=sample;
	}
}

void _InitializeSound(){
	SDL_AudioSpec idealSpec =
	{
		44100,  				//44.1khz
		AUDIO_S32,   		//32 S float
		2,
		0,
		1, 					//buffer size
		0,
		0,
		callback,
		NULL,

	};

	SDL_AudioSpec returnedSpec;

	deviceId = SDL_OpenAudioDevice(NULL, 0, &idealSpec, &returnedSpec, 0);
	printf("Font could not initialize! SDL_image Error: %s\n", SDL_GetError());
	printf("Font could not initialize! SDL_image Error: %i\n", deviceId);
	printf("freq: %i\n", returnedSpec.freq);
	printf("ch: %i\n", returnedSpec.channels);
	printf("pref form: %i\n", AUDIO_S32);
	printf("form: %i\n", returnedSpec.format);
	printf("samp: %i\n", returnedSpec.size);

	SDL_PauseAudioDevice(deviceId,0);
}

void _CleanupSound(){

}