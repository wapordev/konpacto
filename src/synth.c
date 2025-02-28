#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synth.h"

#include <math.h>
#include <SDL.h>

#include "sound.h"
#include "lua.h"
#include "file.h"

KonAudio konAudio;

float rollingMS[64];
int rollingIndex = 0;


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
	konSetBPM(konAudio,120,4,6);
}

//0, 1 if track ended
static inline uint8_t channelTick(KonAudio* konAudio, KonChannel* channel, uint8_t trackIndex, uint8_t channelIndex){
	if(trackIndex==0){return channelIndex==0;}

	KonTrack currentTrack = konAudio->tracks[trackIndex-1];
		
	if(currentTrack.steps==NULL){
		channel->on=0;
		return channelIndex==0;
	}

	if(channel->stepAccumulator == channel->stepLength){



		//first track continues, others loop
		if(channel->stepIndex>currentTrack.length){
			if(channelIndex==0){
				return 1;
			}else{
				channel->stepIndex=0;
			}
		}

		KonStep currentStep = currentTrack.steps[channel->stepIndex];

		//STEP PROCESSING

		if(currentStep.command){
			channel->synthData.command = currentStep.command;
			channel->synthData.param1 = currentStep.param1;
			channel->synthData.param2 = currentStep.param2;
			channel->synthData.param3 = currentStep.param3;
		}

		if(currentStep.instrument){
			channel->synthData.instrument = currentStep.instrument;

			//TO DO, SYNTH INIT

			SetLuaInstrument(konAudio->instruments[currentStep.instrument-1].selectedSynth,channelIndex);
		}

		if(currentStep.velocity){
			channel->synthData.velocity = currentStep.velocity;
		}

		if(currentStep.note!=0){
			if(currentStep.note == 255){
				channel->on =  0;
			}else{
				channel->on =  3;
				channel->synthData.note = currentStep.note;
			}
		}

		channel->stepIndex++;
		channel->stepAccumulator=1;
		channel->stepLength=6; //replace with groove later

	}else{
		channel->stepAccumulator++;
	}

	return 0;
}

void konStopInternal(KonAudio* konAudio){
	konAudio->playing=0;
	for(int i=0;i<CHANNELCOUNT;i++){
		KonChannel* channel = &konAudio->channels[i];

		channel->on=0;
	}
}

void konStop(KonAudio* konAudio){
	lockAudio();
	konStopInternal(konAudio);
	unlockAudio();
}

void konResetChannels(KonAudio* konAudio){
	for(int i=0;i<CHANNELCOUNT;i++){
		KonChannel* channel = &konAudio->channels[i];

		channel->stepIndex=0;
		channel->stepAccumulator=0;
		channel->stepLength=0;
	}
}

void konStart(KonAudio* konAudio, uint8_t arrangeIndex){
	lockAudio();	//ensure no conflicts. future proofing

	konAudio->arrangeIndex=arrangeIndex;

	konResetChannels(konAudio);

	konAudio->frameAcumulator=0;
	konAudio->playing=1;

	unlockAudio();
}

uint8_t konArrangementIsEmpty(KonArrangements* arrangement){
	uint8_t isEmpty = 1;
	for(int i=0;i<CHANNELCOUNT;i++){

		uint8_t trackIndex = arrangement->trackIndexes[i];
		if(trackIndex){
			isEmpty=0;
			break;
		}

	}

	return isEmpty;
}


static inline void sequenceProcess(KonAudio* konAudio){
	konAudio->frameAcumulator += ERRORSTEP;
	if(konAudio->frameAcumulator<=konAudio->tickrate){return;}

	konAudio->frameAcumulator -= konAudio->tickrate;

	KonArrangements arrangement = konAudio->arrangements[konAudio->arrangeIndex];

	uint8_t arrangementEnded = 0;

	for(int i=0;i<CHANNELCOUNT;i++){
		uint8_t trackIndex = arrangement.trackIndexes[i];
		KonChannel* channel = &konAudio->channels[i];

		arrangementEnded = channelTick(konAudio, channel, trackIndex, i);
		if(arrangementEnded){break;}
	}

	if(arrangementEnded){

		uint8_t jump = arrangement.jumpIndex;

		uint8_t stopping = 0;

		if(jump){
			konAudio->arrangeIndex=jump-1;
			arrangement = konAudio->arrangements[konAudio->arrangeIndex];
			if( konArrangementIsEmpty(&arrangement) ){
				stopping=1;
			}
		}else{
			if(konAudio->arrangeIndex==255){
				stopping=1;
			}else{
				konAudio->arrangeIndex+=1;
			}
		}

		if(stopping){
			konStopInternal(konAudio);
		}else{
			konResetChannels(konAudio);
			arrangement = konAudio->arrangements[konAudio->arrangeIndex];
			for(int i=0;i<CHANNELCOUNT;i++){
				uint8_t trackIndex = arrangement.trackIndexes[i];
				KonChannel* channel = &konAudio->channels[i];

				channelTick(konAudio, channel, trackIndex, i);
			}
		}
	}
}

void konFill(KonAudio* konAudio, uint8_t* stream, int len){
	if ( len == 0 )
    return;
	
	uint64_t start = SDL_GetPerformanceCounter();

	int packetSize = konAudio->format.packetSize;
	int channelCount = konAudio->format.channelCount;
	
	int32_t* pointer32 = (int32_t*)stream;
	int16_t* pointer16 = (int16_t*)stream;

	double freqMultiplier = ((double)UINT32_MAX/(double)konAudio->format.frequency);	

	for(int i=0; i<len/packetSize; i+=channelCount){

		if(konAudio->playing){
			sequenceProcess(konAudio);
		}


		double mixLeft = 0;
		double mixRight = 0;

		for(int i=0;i<CHANNELCOUNT;i++){
			KonChannel* channel = &konAudio->channels[i];
			
			//SYNTH HANDLING

			double outLeft=0;
			double outRight=0;

			TickLuaChannel(&outLeft,&outRight,konAudio->channels[i],i);

			double volumeLeft=(channel->synthData.velocity/16)/15.0;
			double volumeRight=(channel->synthData.velocity%16)/15.0;

			if(isfinite(outLeft) && isfinite(outRight)){
				mixLeft+=fclamp(outLeft*volumeLeft,-1.,1.);
				mixRight+=fclamp(outRight*volumeRight,-1.,1.);
			}
		}


		
		int32_t sampleLeft = (int32_t)fclamp((mixLeft*((double)INT32_MAX/CHANNELCOUNT)),INT32_MIN,INT32_MAX)*.0625;
		int32_t sampleRight = (int32_t)fclamp((mixRight*((double)INT32_MAX/CHANNELCOUNT)),INT32_MIN,INT32_MAX)*.0625;

		//printf("%i\n",sampleLeft);

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

	uint64_t end = SDL_GetPerformanceCounter();

	float elapsedMS = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

	rollingMS[rollingIndex]=elapsedMS;
	rollingIndex=(rollingIndex+1)%64;

	float result = 0;

	for(int i=0; i<64; i++){
		result += rollingMS[i];
	}

	result/=64;

	printf("elapsed time average: %f, true %f\n",result, elapsedMS);
}