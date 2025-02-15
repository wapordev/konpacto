#ifndef WWSYNTH_H_
#define WWSYNTH_H_

#define CHANNELCOUNT 8

#include <stdint.h>

uint32_t (*SynthPointer)(KonStep,void*,KonInstrument);

typedef struct KonAudioFormat {
	uint32_t frequency;
	uint32_t packetSize;		//signed int format size in bytes
	uint8_t channelCount;		//1 mono, 2 stereo (output channels, regardless of mix)
}KonAudioFormat;



typedef struct KonSynth {
	SynthPointer synthPointer;
	void* synthStruct;
	int32_t out;				//output!
	int32_t on;					//0 == off, 1 == held, 3 == just pressed
}KonSynth;

typedef struct KonInstrument {		//storing synth identifier and macro list. (pitch offset, volume, etc) 

}KonInstrument;

typedef struct KonStep {
	uint8_t note;					//
	uint8_t instrument;				//literal index into loaded instruments
	uint8_t velocity;					
	uint8_t command;
	uint8_t param1;
	uint8_t param2;
	uint8_t param3;
}KonStep;

typedef struct KonChannel {
	uint8_t stepIndex;
	uint8_t stepAccumulator;		//if accum == length, play next step
	uint8_t stepLength;				//set by groove after current step played
	KonStep synthData;				
	KonSynth synth;					//synth instance created from instrument index
}KonChannel;

typedef struct KonGroove {			//list of ticks per step.
	uint8_t loops;
}KonGroove;

typedef struct KonTrack {
	uint8_t grooveIndex;
	uint8_t length;
	uint8_t temporaryLength; 	//mid-change, to be confirmed. so track data is not deleted accidentally
	KonStep* steps;
}KonTrack;

typedef struct KonArrangements {
	uint8_t trackIndexes[CHANNELCOUNT];	//track index
	uint8_t jumpIndex;					//jump to index
}KonArrangements;

typedef struct KonAudio {
	KonAudioFormat format;
	KonGroove grooves[255];
	KonTrack tracks[255];				//	
	KonArrangements arrangements[256];	//
	KonChannel channels[CHANNELCOUNT];	//channel data
	uint64_t tickrate;					//Unsigned Fixed24_40 calculated by some method, number of frames (including decimal) per tick
	uint64_t frameAcumulator; 			//Unsigned Fixed24_40 rolling error from tickrate
	uint8_t forceMono;					//0 off, 1 both left, 2 both right, 3 mix
	uint8_t playing;
	uint8_t looping;
	uint8_t arrangeIndex;				//currently playing arrangement
}KonAudio;

extern KonAudio konAudio;

void konStart(KonAudio* konAudio, uint8_t arrangeIndex);

void konStop(KonAudio* konAudio);

void konSetBPM(KonAudio* konAudio, uint64_t bpm, uint64_t beatsPerMeasure, uint64_t targetRate);

void konInit(KonAudio* konAudio, int frequency, int packetSize, int channelCount);

void konFill(KonAudio* konAudio, uint8_t* stream, int len);

void ChangeTrackLength(KonTrack* track);

void VerifyTrack(KonTrack* track);

#endif
