#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "input.h"
#include "sound.h"
#include "synth.h"

#include "file.h"

SDL_AudioDeviceID deviceId;

#ifdef MMIYOO

#define AUDIO_TYPE int16_t
const int AUDIO_FORMAT = AUDIO_S16;

#else

#define AUDIO_TYPE int32_t
const int AUDIO_FORMAT = AUDIO_S32;

#endif

void lockAudio(){
	SDL_LockAudioDevice(deviceId);
}

void unlockAudio(){
	SDL_UnlockAudioDevice(deviceId);
}

void callback(void *userdata, Uint8 * stream, int len){
	if ( len == 0 )
    return;
	
	konFill((KonAudio*)userdata,stream,len);
}

void InitializeSound(){
	SDL_AudioSpec idealSpec =
	{
		configSampleRate,  				//44.1khz
		AUDIO_FORMAT,   			//32 S int
		2,
		0,
		configBufferSize, 					//buffer size 512 for miyoo!
		0,
		0,
		callback,
		&konAudio,

	};

	SDL_AudioSpec returnedSpec;

	deviceId = SDL_OpenAudioDevice(NULL, 0, &idealSpec, NULL, 0);

	const char* error = SDL_GetError();
	if(error[0]!='\0'){
		printf("Audio device error: %s\n",error);
	}
	

	returnedSpec = idealSpec;

	konInit(&konAudio,configSampleRate,sizeof(AUDIO_TYPE),2);

	SetScale(&konAudio,"assets/scales/12tet.scl");
	SDL_PauseAudioDevice(deviceId,0);
}


