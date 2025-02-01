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
SDL_AudioSpec returnedSpec;

#ifdef MMIYOO

#define AUDIO_TYPE int16_t
const int AUDIO_FORMAT = AUDIO_S16;


#else

#define AUDIO_TYPE int32_t
const int AUDIO_FORMAT = AUDIO_S32;

#endif

const int AUDIO_SHIFT = (sizeof(int32_t)-sizeof(AUDIO_TYPE))*8;

void callback(void *userdata, Uint8 * stream, int len){
	if ( len == 0 )
    return;
	
	AudioState* data = (AudioState*)userdata; 
	AUDIO_TYPE* pointer = (AUDIO_TYPE*)stream;

	for(int i=0; i<len/sizeof(AUDIO_TYPE); i+=2){
		data->phase+=10967296;
		AUDIO_TYPE sample = (data->phase>>AUDIO_SHIFT)*.0625;
		pointer[i]=sample;
		pointer[i+1]=sample;
	}
}


void _InitializeSound(){
	SDL_AudioSpec idealSpec =
	{
		44100,  				//44.1khz
		AUDIO_FORMAT,   			//32 S int
		2,
		0,
		2048, 					//buffer size
		0,
		0,
		callback,
		audioState,

	};

	//SDL_AudioSpec returnedSpec;

	deviceId = SDL_OpenAudioDevice(NULL, 0, &idealSpec, NULL, 0);

	returnedSpec = idealSpec;

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