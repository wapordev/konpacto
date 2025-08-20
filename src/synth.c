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
	if(index<0 || index > 35){
		return 0;
	}
	if(index==32)
		if(konAudio.luaData.bankSelect-1>=0)
		{
			LuaDatabank* bank = &konAudio.luaData.banks[konAudio.luaData.bankSelect-1];
			return bank->outLeft * bank->data[1];
		}
	if(index==33)
		if(konAudio.luaData.bankSelect-1>=0)
		{
			LuaDatabank* bank = &konAudio.luaData.banks[konAudio.luaData.bankSelect-1];
			return bank->outRight * bank->data[1];
		}
	if(index==34)
		return konAudio.luaData.carryLeft;
	if(index==35)
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

static inline void resetChannelMacros(KonChannel* channel, int force){
	for (int i=0; i<32; i++){
			KonMacroProcess* macro = &channel->synthMacros[i];
			if(!macro->macro.oscillates || force){
				macro->stepIndex=0;
				macro->nextStep=0;
				macro->percentage=1;
			}

			macro = &channel->routeMacros[i];
			if(!macro->macro.oscillates || force){
				macro->stepIndex=0;
				macro->nextStep=0;
				macro->percentage=1;
			}
		}
}

//0, 1 if track ended
static inline uint8_t channelTick(KonAudio* konAudio, KonChannel* channel, uint8_t trackIndex, uint8_t channelIndex){
	if(trackIndex==0){return 1;}

	KonTrack currentTrack = konAudio->tracks[trackIndex-1];
		
	if(currentTrack.steps==NULL){
		channel->on=ChOff;
		return channelIndex==0;
	}

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

			//TO DO, SYNTH INIT

			KonInstrument* instrument = &konAudio->instruments[currentStep.instrument-1];

			char* effectName = NULL;

			if(instrument->route){
				effectName = konAudio->instruments[instrument->route-1].selectedSynth;
			} 

			SetLuaInstrument(instrument->selectedSynth,effectName,channelIndex);

			for(int i=0;i<instrument->macroCount;i++){
				channel->synthMacros[i].macro=instrument->macros[i].macro;
			}
			if(instrument->route){
				KonInstrument* routeInstrument = &konAudio->instruments[instrument->route-1];
				for(int i=0;i<routeInstrument->macroCount;i++){
					channel->routeMacros[i].macro=routeInstrument->macros[i].macro;
				}
			}
		}

		if(currentStep.velocity){
			channel->synthData.velocity = currentStep.velocity;
		}

		if(currentStep.note!=0){
			if(currentStep.note == 255){
				//macro loop off
				channel->on =  ChRel;
			}else{
				channel->on =  ChJust;
				channel->synthData.note = currentStep.note;
				resetChannelMacros(channel,0);
			}
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

		channel->on=ChOff;
	}
}

void konStop(KonAudio* konAudio){
	lockAudio();
	konStopInternal(konAudio);
	unlockAudio();
}

void konResetChannels(KonAudio* konAudio, int force){
	for(int i=0;i<CHANNELCOUNT;i++){
		KonChannel* channel = &konAudio->channels[i];

		channel->on=ChOff;
		channel->stepIndex=0;
		channel->stepAccumulator=0;
		channel->stepLength=0;

		resetChannelMacros(channel,force);

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
			KonMacro* macro = &instrument->macros[i].macro;
			if(macro->length){
				macro->length=0;
				free(macro->data);
				macro->data=NULL;
			}
		}
	}

	if (params>instrument->macroCount){
		for(int i=0;i<params;i++){
			KonMacro* macro = &instrument->macros[i].macro;
			if (i==1) {
				macro->max = 64;
			}else {
				macro->max = 255;
			}
		}
	}

	instrument->macroCount=params;

	strcpy(instrument->macros[0].name,"pitch");
	strcpy(instrument->macros[1].name,"volume");
	strcpy(instrument->macros[2].name,"panning");
	instrument->macros[0].macro.defaultValue=64;
	instrument->macros[1].macro.defaultValue=255;
	instrument->macros[2].macro.defaultValue=128;

	instrument->macros[0].macro.loopStart=1;
	instrument->macros[1].macro.loopStart=1;
	instrument->macros[2].macro.loopStart=1;

	for(int i=3;i<params;i++){
		int defaultValue = GetLuaParam(path,i-3,instrument->macros[i].name);
		if(defaultValue>=0){
			instrument->macros[i].macro.defaultValue = defaultValue;
			if(instrument->macros[i].macro.length==0){
				instrument->macros[i].macro.loopStart=1;
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
	konResetChannels(konAudio,1);
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
			KonMacroUI* macroShell = &instrument->macros[j];
			KonMacro* macro = &macroShell->macro;
			macroShell->name[0]='\0';
			if (macro->length) {
				macro->length=0;
				free(macro->data);
				macro->data = NULL;
			}
		}
	}
}

void konStart(KonAudio* konAudio, uint8_t arrangeIndex){
	lockAudio();	//ensure no conflicts. future proofing

	konAudio->arrangeIndex=arrangeIndex;

	konResetChannels(konAudio,1);

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
	if(!macro->length)
		return (double)macro->defaultValue;
	if(macro->length==1)
		return (double)macro->data[0];

	return (double)macro->data[clamp(step,0,macro->length-1)];
}

static inline double lerp(double a, double b, double f) 
{
    return (a * (1.0 - f)) + (b * f);
}

static inline int macroNextStep(KonChannel* channel, KonMacroProcess* macroShell, KonMacro* macro){
	int step = macroShell->stepIndex;
	int next = step+1;

	if((channel->on != ChRel || macro->loopEnd == macro->length-1) && macro->loopEnd >= macro->loopStart && next > macro->loopEnd){
		next=macro->loopStart;
	}

	if(macro->length && next>macro->length-1){
		next=macro->length-1;
	}

	return next;
} 

static inline double macroProcess(KonAudio* konAudio, KonChannel* channel, KonMacroProcess* macroShell){
	KonMacro* macro = &macroShell->macro;
	
	if(macroShell->percentage>=1.0){
		macroShell->percentage-=1.0;
		macroShell->stepIndex=macroShell->nextStep;
		macroShell->nextStep=macroNextStep(channel,macroShell,macro);
		macroShell->currentValue=macroDataGet(macro,macroShell->stepIndex);
	}

	int speed = macro->speed;
	if(!speed)
		speed=1;
	double stepRate=((double)ERRORSTEP)/konAudio->tickrate/speed;

	double out=macroShell->currentValue;

	if(macro->interpolationMode==IntLinear){
		double nextValue = macroDataGet(macro,macroShell->nextStep);

		double diff = nextValue - macroShell->currentValue;
		
		double stepsLeft = (1-macroShell->percentage)/stepRate;

		macroShell->currentValue += diff/stepsLeft;
	}

	macroShell->percentage += stepRate;

	if(macro->max!=0 || macro->min!=255){
		if(macro->max==macro->min){
			out=macro->max;
		}else{
			out*=(double)(macro->max-macro->min)/255;
			out+=macro->min;
		}
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
			konResetChannels(konAudio,0);
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

const double root2d2 = sqrt(2)/2;
const double pid4 = M_PI/4;

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

			for(int k=0;k<instrument->macroCount;k++){
				double macroOutput = macroProcess(konAudio,channel,&channel->synthMacros[k]);
				if(k && k!=2)
					macroOutput/=255;
				synthBank->data[k]=macroOutput;
			}

			synthBank->data[0] = noteProcess(konAudio,synthBank->data[0],referenceNote);

			if(!instrument->route)
				continue;
			LuaDatabank* routeBank = &konAudio->luaData.banks[j*2+1];

			KonInstrument* routeInstrument = &konAudio->instruments[instrument->route-1];

			for(int k=0;k<routeInstrument->macroCount;k++){
				double macroOutput = macroProcess(konAudio,channel,&channel->routeMacros[k]);
				if(k)
					macroOutput/=255;
				routeBank->data[k]=macroOutput;
			}

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
			double panVelocity=(channel->synthData.velocity%16);
			if(panVelocity)panVelocity--;
			panVelocity = panVelocity/14.0-.5;

			double synthVolume = synthBank->data[1];
			double synthPan = synthBank->data[2];
			if(synthPan)synthPan--;
			synthPan = synthPan/127-1;

			double panning = fclamp(synthPan+panVelocity,-1,1);

			double panAng = panning*pid4;
			double panCos = cos(panAng);
			double panSin = sin(panAng);
			double leftVolume = root2d2*(panCos - panSin);
			double rightVolume = root2d2*(panCos + panSin);
			
			//double panning=(channel->synthData.velocity%16)/15.0;

			double synthOutLeft = fclamp(synthBank->outLeft,-1,1)*synthVolume;
			double synthOutRight = fclamp(synthBank->outRight,-1,1)*synthVolume;

			double outLeft = synthOutLeft;
			double outRight = synthOutRight;

			if(instrument->route){
				LuaDatabank* routeBank = &konAudio->luaData.banks[j*2+1];
				double routeVolume = routeBank->data[1];

				double routeOutLeft = fclamp(routeBank->outLeft,-1,1)*routeVolume;
				double routeOutRight = fclamp(routeBank->outRight,-1,1)*routeVolume;

				outLeft = lerp(routeOutLeft,synthOutLeft,((double)instrument->wetDryMix)/255.0);
				outRight = lerp(routeOutRight,synthOutRight,((double)instrument->wetDryMix)/255.0);
			}

			outLeft*=velocity*leftVolume;
			outRight*=velocity*rightVolume;

			if(isfinite(outLeft) && isfinite(outRight)){
				mixLeft+=outLeft;
				mixRight+=outRight;
			}
		}
		
		int32_t sampleLeft = (int32_t)fclamp((mixLeft*((double)INT32_MAX/CHANNELCOUNT)),INT32_MIN,INT32_MAX);
		int32_t sampleRight = (int32_t)fclamp((mixRight*((double)INT32_MAX/CHANNELCOUNT)),INT32_MIN,INT32_MAX);

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