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

#if defined _WIN32 || defined __CYGWIN__
   #define DLL_PUBLIC __declspec(dllexport)
#else
   #define DLL_PUBLIC
#endif


KonAudio konAudio;

float rollingMS[64];
int rollingIndex = 0;


const uint64_t ERRORSTEP = (uint64_t)1<<40;


//sooooooooo
//till now I've been structuring this file with the idea that the KonAudio instance is not a singleton
//not that I need it that way nor is it implemented that way, but just for flexibility later.
//The only way I can think to have this work here is by having a lua light userdata
//which requires more typing for the end user, or a lua wrapper which is an unnecesary function call
//and possibly having the danger of accessing bad memory through lua?
DLL_PUBLIC double konGet(int index){
	if(index<0 || index > 63){
		return 0;
	}
	return konAudio.luaData[index];
}

DLL_PUBLIC void konOut(double left, double right){
	konAudio.luaData[64] = left;
	konAudio.luaData[65] = right;
}

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
	if(trackIndex==0){return 1;}

	KonTrack currentTrack = konAudio->tracks[trackIndex-1];
		
	if(currentTrack.steps==NULL){
		channel->on=0;
		return channelIndex==0;
	}

	channel->tickCounter+=1;

	int ending = 0;

	if(channel->stepAccumulator == channel->stepLength){



		//first track continues, others loop
		if(channel->stepIndex>currentTrack.length){
			ending = 1;
			channel->stepIndex=0;
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

			channel->tickCounter=0;

			//TO DO, SYNTH INIT

			SetLuaInstrument(konAudio->instruments[currentStep.instrument-1].selectedSynth,channelIndex);
		}

		if(currentStep.velocity){
			channel->synthData.velocity = currentStep.velocity;
		}

		if(currentStep.note!=0){
			if(currentStep.note == 255){
				//macro loop off
				//channel->on =  0;
			}else{
				channel->on =  3;
				channel->synthData.note = currentStep.note;
			}
			channel->tickCounter=0;
		}

		channel->stepIndex++;
		channel->stepAccumulator=1;
		channel->stepLength=6; //replace with groove later

	}else{
		channel->stepAccumulator++;
	}

	return ending;
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

		channel->on=0;
		channel->stepIndex=0;
		channel->stepAccumulator=0;
		channel->stepLength=0;
		channel->tickCounter=0;
	}
}

void setInstrument(int instrumentIndex, char* name){
	KonInstrument* instrument = &konAudio.instruments[instrumentIndex];

	char path[64]="assets/synths/";

	strcat(path,name);

	LoadLuaFile(path);

	int params = clamp(CountLuaParam(path),0,28)+3;

	if(params<instrument->macroCount){
		for(int i=params;i<instrument->macroCount;i++){
			if(instrument->macros[i].length){
				free(instrument->macros[i].data);
				instrument->macros[i].length=0;
			}
		}
	}

	instrument->macroCount=params;

	strcpy(instrument->macros[0].name,"pitch");
	strcpy(instrument->macros[1].name,"volume l");
	strcpy(instrument->macros[2].name,"volume r");
	instrument->macros[0].defaultValue=64;
	instrument->macros[1].defaultValue=255;
	instrument->macros[2].defaultValue=255;

	instrument->macros[0].loopStart=1;
	instrument->macros[1].loopStart=1;
	instrument->macros[2].loopStart=1;

	for(int i=3;i<params;i++){
		int defaultValue = GetLuaParam(path,i-3,instrument->macros[i].name);
		if(defaultValue>=0){
			instrument->macros[i].defaultValue = defaultValue;
			if(instrument->macros[i].length==0){
				instrument->macros[i].loopStart=1;
			}
		}
	}
	instrument->selectedMacro=0;
}


//im so sorry for the konaudio* inconsistency.
//i never settled on a standard
//i wanted it to be instanceable, and whatnot
//but that just makes the project structure more confusing

//dangerous
void clearSong(KonAudio* konAudio){
	konAudio->arrangeIndex = 0;
	konAudio->looping = 0;
	konStopInternal(konAudio);
	konAudio->frameAcumulator = 0;
	konResetChannels(konAudio);
	for(int i=0;i<255;i++){
		KonTrack* track = &konAudio->tracks[i];
		track->grooveIndex=0;
		if (track->steps!=NULL) {
			free(track->steps);
			track->steps = NULL;
		}
		track->length=16;
		track->temporaryLength=16;
	}
	for(int i=0;i<256;i++){
		KonArrangements* arrangement = &konAudio->arrangements[i];
		for(int j=0;j<8;j++){
			arrangement->trackIndexes[j]=0;
		}
		arrangement->jumpIndex=0;
	}
	for(int i=0;i<255;i++){
		KonInstrument* instrument = &konAudio->instruments[i];
		instrument->name[0]='\0';
		instrument->selectedSynth[0]='\0';
		instrument->route=0;
		instrument->macroCount = 0;
		instrument->selectedMacro = 0;
		for(int j=0;j<64;j++){
			KonMacro* macro = &instrument->macros[j];
			macro->name[0]='\0';
			if (macro->data!=NULL) {
				free(macro->data);
				macro->data = NULL;
			}
		}
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

uint8_t konTrackIsEmpty(KonTrack* track){
	if (track->steps == NULL)
		return 1;
	uint8_t isEmpty = 1;
	for(int i=0;i<track->length;i++){
		if(track->steps[i].note ||
			track->steps[i].instrument ||
			track->steps[i].velocity ||
			track->steps[i].command ||
			track->steps[i].param1 ||
			track->steps[i].param2 ||
			track->steps[i].param3)
			isEmpty = 0;
	}

	return isEmpty;
}

static inline double macroDataGet(KonMacro* macro, int step){

	if(macro->loopEnd>=macro->loopStart){

	}

	return (double)macro->data[clamp(step,0,macro->length-1)];
}

static inline double macroProcess(KonAudio* konAudio, KonChannel* channel, KonInstrument* instrument, KonMacro* macro){
	double out=0;

	if(macro->length){
		if(macro->length==1){
			out=macro->data[0];
		}else{
			int speed = macro->speed;
			if(!speed)
				speed=1;

			int rawPosition = channel->tickCounter;

			out=macroDataGet(macro,rawPosition/speed);

			if(macro->interpolationMode==IntLinear){
				double positionOffset =  ((double)konAudio->frameAcumulator/konAudio->tickrate + (double)(rawPosition%speed)) /speed;

				if(positionOffset>0){
					double nextStep = macroDataGet(macro,rawPosition/speed+1);

					out=lerp(out,nextStep,positionOffset);
				}
			}

		}
	}else{
		out=macro->defaultValue;
	}

	return out;
}

static inline void sequenceProcess(KonAudio* konAudio){
	konAudio->frameAcumulator += ERRORSTEP;
	if(konAudio->frameAcumulator<=konAudio->tickrate){return;}

	konAudio->frameAcumulator -= konAudio->tickrate;

	KonArrangements arrangement = konAudio->arrangements[konAudio->arrangeIndex];

	uint8_t arrangementEnded = 0;

	uint8_t firstRealTrack=0;

	for(int i=0;i<CHANNELCOUNT;i++){
		uint8_t trackIndex = arrangement.trackIndexes[i];
		KonChannel* channel = &konAudio->channels[i];

		int ended = channelTick(konAudio, channel, trackIndex, i);

		if (trackIndex==0 && i==firstRealTrack){
			firstRealTrack++;
			if(firstRealTrack==CHANNELCOUNT)
				arrangementEnded=1;
		}
		if(i==firstRealTrack)
			arrangementEnded=ended;

		if(arrangementEnded){break;}
	}

	if(arrangementEnded){

		uint8_t jump = arrangement.jumpIndex;

		uint8_t stopping = 0;

		if(jump){
			konAudio->arrangeIndex=jump-1;
			
		}else{
			konAudio->arrangeIndex+=1;
		}

		arrangement = konAudio->arrangements[konAudio->arrangeIndex];
		if( konArrangementIsEmpty(&arrangement) ){
			stopping=1;
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
	double* luaData = konAudio->luaData;

	for(int i=0; i<len/packetSize; i+=channelCount){

		if(konAudio->playing){
			sequenceProcess(konAudio);
		}


		double mixLeft = 0;
		double mixRight = 0;

		
		for(int j=0;j<CHANNELCOUNT;j++){
			KonChannel* channel = &konAudio->channels[j];
			
			if(!channel->on || !channel->synthData.instrument || !channel->synthData.note)
				continue;

			KonInstrument* instrument = &konAudio->instruments[channel->synthData.instrument-1];

			for(int k=0;k<instrument->macroCount;k++){
				if(k==2 && !instrument->macros[k].length){
					luaData[k]=luaData[1];
					continue;
				}
				luaData[k]=macroProcess(konAudio,channel,instrument,&instrument->macros[k]);
			}

			//note processing!

			double trueNote=luaData[0];
			if(trueNote>=128){
				trueNote-=128;
			}else{
				trueNote=-64+trueNote;
				trueNote+=channel->synthData.note-1;
			}
			
			luaData[0]=konAudio->frequencies[clamp((int)trueNote,0,254)];

			double noteFloor=floorf(trueNote);

			if((int)trueNote<254 && noteFloor!=trueNote){
				

				double freqA=luaData[0];

				double freqB=konAudio->frequencies[(int)trueNote+1];

				double interp=trueNote-noteFloor;

				//pow is slow :3
				//powf(freqA,1-interp)*powf(freqB,interp);
				luaData[0]=lerp(freqA,freqB,interp);

				//note lerp
			}
			
			
			
			TickLuaChannel(j);

			double outLeft=luaData[64];
			double outRight=luaData[65];

			double volumeLeft=(channel->synthData.velocity/16)/15.0;
			double volumeRight=(channel->synthData.velocity%16)/15.0;

			volumeLeft*=luaData[1]/255;
			volumeRight*=luaData[2]/255;

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

	//printf("elapsed time average: %f, true %f\n",result, elapsedMS);
}