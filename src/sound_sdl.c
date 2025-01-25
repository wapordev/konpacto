#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <math.h>
#include <stdbool.h>

#include "input.h"

SDL_AudioDeviceID deviceId;


float x = 0;

void callback(void *userdata, Uint8 * stream, int len){
	if ( len == 0 )
    return;


	//printf("whell %i",len);
	//exit(0);
	float* pointer = (float*)stream;

	for(int i=0; i<len/4; i+=2){
		if(IsPressed(4)){
			x+=.0625;
		}else{
			x+=.125;
		}
		
		float sample = sin(x)/M_PI;
		pointer[i]=sample;
		pointer[i+1]=sample;
	}
}

void _InitializeSound(){
	SDL_AudioSpec idealSpec =
	{
		44100,  				//44.1khz
		AUDIO_F32,   		//32 S float
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
	printf("pref form: %i\n", AUDIO_F32);
	printf("form: %i\n", returnedSpec.format);
	printf("samp: %i\n", returnedSpec.size);

	SDL_PauseAudioDevice(deviceId,0);
}

void _CleanupSound(){

}