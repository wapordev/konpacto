#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

KonAudio konAudio;

const uint64_t ERRORSTEP = (uint64_t)1<<40;

void VerifyTrack(KonTrack* track){
	if (track->steps == NULL) {
		track->length=15;
		track->temporaryLength=15;
		track->steps = calloc(track->length+1,sizeof(KonStep));
	}
}

void ChangeTrackLength(KonTrack* track){
	track->steps = realloc(track->steps,sizeof(KonStep)*(track->temporaryLength+1));

	if(track->temporaryLength > track->length){
		memset(&track->steps[track->length+1],0,sizeof(KonStep)*(track->temporaryLength-track->length));
	}

	track->length=track->temporaryLength;
}

void konSetBPM(KonAudio* konAudio, uint64_t bpm, uint64_t beatsPerMeasure, uint64_t targetRate){

	uint64_t freq = konAudio->format.frequency;

	uint64_t tickrate = (((freq*ERRORSTEP*60)/120)/beatsPerMeasure)/targetRate;

	konAudio->tickrate=tickrate;
}

void konInit(KonAudio* konAudio, int frequency, int packetSize, int channelCount){ 

	konAudio->format.frequency = frequency;
	konAudio->format.packetSize = packetSize;
	konAudio->format.channelCount = channelCount;
	konSetBPM(konAudio,120,4,1);
}


void konFill(KonAudio* konAudio, uint8_t* stream, int len){
	if ( len == 0 )
    return;
	
	int packetSize = konAudio->format.packetSize;
	int channelCount = konAudio->format.channelCount;
	
	int32_t* pointer32 = (int32_t*)stream;
	int16_t* pointer16 = (int16_t*)stream;

	

	for(int i=0; i<len/packetSize; i+=channelCount){
		KonSynth* synth = &konAudio->channels[0].synth;

		konAudio->frameAcumulator += ERRORSTEP;
		if(konAudio->frameAcumulator>konAudio->tickrate){
			konAudio->frameAcumulator -= konAudio->tickrate;

			synth->on = !synth->on;
			
		}



		synth->phase+=10967296;

		int32_t sample = synth->on ? synth->phase : 0;
		
		int32_t sampleLeft = sample*.0625;
		int32_t sampleRight = sampleLeft;

		if(packetSize==4){
			pointer32[i]=sampleLeft;
			pointer32[i+1]=sampleRight;
		}else{
			sampleLeft=sampleLeft>>((4 - packetSize)*8);
			sampleRight=sampleRight>>((4 - packetSize)*8);
			pointer16[i]=sampleLeft;
			pointer16[i+1]=sampleRight;
		}
	}
}