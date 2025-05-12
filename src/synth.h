#ifndef WWSYNTH_H_
#define WWSYNTH_H_

#define CHANNELCOUNT 8

#include <stdint.h>

typedef struct KonAudioFormat {
	uint32_t frequency;
	uint32_t packetSize;		//signed int format size in bytes
	uint8_t channelCount;		//1 mono, 2 stereo (output channels, regardless of mix)
}KonAudioFormat;

typedef struct KonMacro {		//parameter/modulation data
	char name[16];
	uint8_t defaultValue;
	uint8_t speed;				//0 to 127 tick speed+1, -1 to -128, ms? maybe?
	uint8_t min;				
	uint8_t max;				//data output range.
	uint8_t flags;				//bit 0 = loops, bit 1,2 = interpolation mode 
	uint8_t loopStart;
	uint8_t loopEnd;
	uint8_t length;
	uint8_t selectedStep;		//editor value
	uint8_t* data;
}KonMacro;

typedef struct KonInstrument {		//storing synth identifier and macro list. (pitch offset, volume, etc) 
	char name[21];
	char selectedSynth[64];
	char synthEffect[64];
	uint8_t macroCount;
	uint8_t effectCount;
	KonMacro macros[64];
	uint8_t selectedMacro;			//editor value
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

// typedef struct KonGenerator {
// 	GeneratorPointer generatorPointer;
// 	uint8_t numOf
// }KonGenerator;

typedef struct KonChannel {
	uint8_t stepIndex;
	uint8_t stepAccumulator;		//if accum == length, play next step
	uint8_t stepLength;				//set by groove after current step played
	uint64_t tickCounter;			//info for the macros!
	int32_t on;
	KonStep synthData;				//synth instance created from instrument index
}KonChannel;

typedef struct KonGroove {			//list of ticks per step.
	uint8_t loops;
}KonGroove;

typedef struct KonTrack {
	uint8_t length;
	uint8_t grooveIndex;
	uint8_t temporaryLength; 	//mid-change, to be confirmed. so track data is not deleted accidentally
	KonStep* steps;
}KonTrack;

typedef struct KonArrangements {
	uint8_t trackIndexes[CHANNELCOUNT];	//track index
	uint8_t jumpIndex;					//jump to index
}KonArrangements;

typedef struct KonAudio {
	KonAudioFormat format;
	int notesInScale;
	double frequencies[254];
	KonGroove grooves[255];
	KonTrack tracks[255];				//	
	KonInstrument instruments[255];
	KonArrangements arrangements[256];	//
	KonChannel channels[CHANNELCOUNT];	//channel data
	double luaData[66];
	uint64_t tickrate;					//Unsigned Fixed24_40 calculated by some method, number of frames (including decimal) per tick
	uint64_t frameAcumulator; 			//Unsigned Fixed24_40 rolling error from tickrate
	uint8_t forceMono;					//0 off, 1 both left, 2 both right, 3 mix
	uint8_t playing;
	uint8_t looping;
	uint8_t arrangeIndex;				//currently playing arrangement
	char* songName;
	uint8_t beatsPerMinute;
}KonAudio;

extern KonAudio konAudio;

uint8_t konTrackIsEmpty(KonTrack* track);

double konGet(int index);

void setInstrument(int instrumentIndex, char* name);

void setEffect(int instrumentIndex, char* name);

//dangerous
void clearSong(KonAudio* konAudio);

void konOut(double left, double right);

void konStart(KonAudio* konAudio, uint8_t arrangeIndex);

void konStop(KonAudio* konAudio);

void konSetBPM(KonAudio* konAudio, uint64_t bpm, uint64_t beatsPerMeasure, uint64_t targetRate);

void konInit(KonAudio* konAudio, int frequency, int packetSize, int channelCount);

void konFill(KonAudio* konAudio, uint8_t* stream, int len);

void ChangeTrackLength(KonTrack* track);

void VerifyTrack(KonTrack* track);

#endif
