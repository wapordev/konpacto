#ifndef WWFILE_H_
#define WWFILE_H_

#include "synth.h"

#include <stdbool.h>

int clamp(int d, int min, int max);

double fclamp(double d, double min, double max);

char* IntToChar(int BMPstring[], char* out, int length);

char* CharToBMP(char* string, bool filter);

char** ListPath(char* path, char* filter, int* outLength);

int FindStringInList(char** list, int count, char* string);

void SetScale(KonAudio* konAudio, const char* scalePath);

void FreePath();

#endif
