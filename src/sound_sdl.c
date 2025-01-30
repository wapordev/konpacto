#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "input.h"
#include "sound_sdl.h"
#include "synth.h"

SDL_AudioDeviceID deviceId;

void callback(void *userdata, Uint8 * stream, int len){
	if ( len == 0 )
    return;
	
	// Uint64 start = SDL_GetPerformanceCounter();

	AudioState* data = (AudioState*)userdata; 
	int32_t* pointer = (int32_t*)stream;

	for(int i=0; i<len/4; i+=2){
		// for(int j=0; j<200; j++){
		// 	data->dummy+=sin(data->phase);
		// }
		data->phase+=10967296;
		int32_t sample = data->phase*.0625;
		pointer[i]=sample;
		pointer[i+1]=sample;
	}

	// Uint64 end = SDL_GetPerformanceCounter();

	// float elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;


	// printf("well, it took %f",elapsedMS);
	// exit(0);

}

void _InitializeSound(){
	SDL_AudioSpec idealSpec =
	{
		44100,  				//44.1khz
		AUDIO_S32,   			//32 S int
		2,
		0,
		2048, 					//buffer size
		0,
		0,
		callback,
		audioState,

	};

	SDL_AudioSpec returnedSpec;

	deviceId = SDL_OpenAudioDevice(NULL, 0, &idealSpec, NULL, 0);
	printf("Font could not initialize! SDL_image Error: %s\n", SDL_GetError());
	printf("Font could not initialize! SDL_image Error: %i\n", deviceId);
	printf("freq: %i\n", returnedSpec.freq);
	printf("ch: %i\n", returnedSpec.channels);
	printf("pref form: %i\n", MIX_DEFAULT_FORMAT);
	printf("form: %i\n", returnedSpec.format);
	printf("samp: %i\n", returnedSpec.size);

	SDL_PauseAudioDevice(deviceId,0);
}

void _CleanupSound(){

}