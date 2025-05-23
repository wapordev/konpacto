#ifndef WWFILE_H_
#define WWFILE_H_

#include "synth.h"

#include <stdbool.h>

void SaveSong(char* path);

void LoadSong(char* path);

void LoadConfig();

void SaveConfig();

int clamp(int d, int min, int max);

double lerp(double a, double b, double f);

double fclamp(double d, double min, double max);

char* IntToChar(int BMPstring[], char* out, int length);

char* CharToBMP(char* string, bool filter);

char** ListPath(char* path, char* filter, int* outLength);

int FindStringInList(char** list, int count, char* string);

void SetScale(KonAudio* konAudio, const char* scalePath);

void FreePath();

extern int configSampleRate;
extern int configBufferSize;
extern char* configTheme;
extern char* configFont;

#endif
