#ifndef WWFILE_H_
#define WWFILE_H_

#include "synth.h"

char* IntToChar(int BMPstring[], char* out, int length);

char* CharToBMP(char* string, bool filter);

char** ListPath(char* path, char* filter, int* outLength);

void SetScale(KonAudio* konAudio, const char* scalePath);

void FreePath();

#endif
