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
	if(index<0 || index > 33){
		return 0;
	}
	if(index==32)
		return konAudio.luaData.carryLeft;
	if(index==33)
		return konAudio.luaData.carryRight;
	return konAudio.luaData.banks[konAudio.luaData.bankSelect].data[index];
}

DLL_PUBLIC void konOut(double left, double right){
	konAudio.luaData.banks[konAudio.luaData.bankSelect].outLeft = left;
	konAudio.luaData.banks[konAudio.luaData.bankSelect].outRight = right;
}

DLL_PUBLIC void bankSwitch(){
	if(konAudio.luaData.bankSelect%2==1){
		konAudio.luaData.carryLeft = konAudio.luaData.banks[konAudio.luaData.bankSelect].outLeft;
		konAudio.luaData.carryRight = konAudio.luaData.banks[konAudio.luaData.bankSelect].outRight;
	}
	if (konAudio.luaData.bankSelect < 15)
		konAudio.luaData.bankSelect++;
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

void konSetBPM(KonAudio* konAudio){

	uint64_t freq = konAudio->format.frequency;

	uint64_t tickrate = ((freq*ERRORSTEP*15)/konAudio->bpm)/konAudio->ticksPerStep;

	konAudio->tickrate=tickrate;
}

void konInit(KonAudio* konAudio, int frequency, int packetSize, int channelCount){ 

	konAudio->format.frequency = frequency;
	konAudio->format.packetSize = packetSize;
	konAudio->format.channelCount = channelCount;
	konAudio->bpm=120;
	konAudio->ticksPerStep=6;
	konSetBPM(konAudio);
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

			KonInstrument* instrument = &konAudio->instruments[currentStep.instrument-1];

			SetLuaInstrument(instrument->selectedSynth,channelIndex);

			for(int i=0;i<instrument->macroCount;i++){
				channel->synthMacros[i]=instrument->macros[i];
			}
			if(instrument->route){
				KonInstrument* routeInstrument = &konAudio->instruments[instrument->route-1];
				for(int i=0;i<routeInstrument->macroCount;i++){
					channel->routeMacros[i]=routeInstrument->macros[i];
				}
			}
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

		//groove
		channel->stepLength=konAudio->ticksPerStep;
		if (currentTrack.grooveIndex)
		{
			KonGroove* groove = &konAudio->grooves[currentTrack.grooveIndex-1];

			if (groove->length)
				channel->stepLength= groove->data[channel->stepIndex%groove->length];
		}

		channel->stepIndex++;
		channel->stepAccumulator=1;

		

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
			if (macro->length) {
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

static inline double lerp(double a, double b, double f) 
{
    return (a * (1.0 - f)) + (b * f);
}

static inline double macroProcess(KonAudio* konAudio, KonChannel* channel, KonMacro* macro){
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

static inline double noteProcess(KonAudio* konAudio, double note,int referenceNote){
	if(note>=128){
		note-=128;
	}else{
		note=-64+note;
		note+=referenceNote;
	}
	
	double freq=konAudio->frequencies[clamp((int)note,0,254)];

	double noteFloor=floorf(note);

	if((int)note<254 && noteFloor!=note){
		

		double freqB=konAudio->frequencies[(int)note+1];

		double interp=note-noteFloor;

		//pow is slow :3
		//powf(freqA,1-interp)*powf(freqB,interp);
		freq=lerp(freq,freqB,interp);

		//note lerp
	}

	return freq;
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
	LuaData* luaData = &konAudio->luaData;

	for(int i=0; i<len/packetSize; i+=channelCount){

		if(konAudio->playing){
			sequenceProcess(konAudio);
		}
		
		konAudio->luaData.bankSelect=0;

		for(int j=0;j<CHANNELCOUNT;j++){
			KonChannel* channel = &konAudio->channels[j];
			
			if(!channel->on || !channel->synthData.instrument || !channel->synthData.note)
				continue;

			KonInstrument* instrument = &konAudio->instruments[channel->synthData.instrument-1];
			LuaDatabank* synthBank = &konAudio->luaData.banks[j*2];
			
			
			int referenceNote = channel->synthData.note-1;

			for(int k=0;k<instrument->macroCount;k++)
				synthBank->data[k]=macroProcess(konAudio,channel,&channel->synthMacros[k]);

			synthBank->data[0] = noteProcess(konAudio,synthBank->data[0],referenceNote);

			if(!instrument->route)
				continue;
			LuaDatabank* routeBank = &konAudio->luaData.banks[j*2+1];

			KonInstrument* routeInstrument = &konAudio->instruments[instrument->route-1];

			for(int k=0;k<routeInstrument->macroCount;k++)
				routeBank->data[k]=macroProcess(konAudio,channel,&channel->routeMacros[k]);

			routeBank->data[0] = noteProcess(konAudio,routeBank->data[0],referenceNote);
		}

		TickLuaChannels();

		double mixLeft = 0;
		double mixRight = 0;

		for (int j=0;j<8;j++){
			KonChannel* channel = &konAudio->channels[j];

			if(!channel->on || !channel->synthData.instrument || !channel->synthData.note)
				continue;

			KonInstrument* instrument = &konAudio->instruments[channel->synthData.instrument-1];
			LuaDatabank* synthBank = &konAudio->luaData.banks[j*2];

			double velocity=(channel->synthData.velocity/16)/15.0;
			double synthVolume = synthBank->data[1]/255;
			

			//double panning=(channel->synthData.velocity%16)/15.0;

			double synthOutLeft = synthBank->outLeft*synthVolume*velocity;
			double synthOutRight = synthBank->outRight*synthVolume*velocity;

			double outLeft = synthOutLeft;
			double outRight = synthOutRight;

			if(instrument->route){
				LuaDatabank* routeBank = &konAudio->luaData.banks[j*2+1];
				double routeVolume = routeBank->data[1]/255;

				double routeOutLeft = routeBank->outLeft*routeVolume*velocity;
				double routeOutRight = routeBank->outRight*routeVolume*velocity;

				double outLeft = lerp(routeOutLeft,synthOutLeft,((double)instrument->wetDryMix)/255.0);
				double outRight = lerp(routeOutRight,synthOutRight,((double)instrument->wetDryMix)/255.0);
			}

			if(isfinite(outLeft) && isfinite(outRight)){
				mixLeft+=fclamp(outLeft,-1.,1.);
				mixRight+=fclamp(outRight,-1.,1.);
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