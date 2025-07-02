#ifndef WWFILE_H_
#define WWFILE_H_

#include "synth.h"

#include <stdbool.h>

typedef struct FileStructure {
	char* path;
	char* name;
	int isDir;
}FileStructure;

typedef struct FileResults {
	FileStructure* files;
	int count;
}FileResults;

void SaveSong(char* path);

void LoadSong(char* path);

void LoadConfig();

void SaveConfig();

int clamp(int d, int min, int max);

double lerp(double a, double b, double f);

double fclamp(double d, double min, double max);

char* IntToChar(int BMPstring[], char* out, int length);

char* CharToBMP(char* string, bool filter);

void ListPath(char* path, char* filter, FileResults* result, int includeDirectories);

int FindStringInList(FileResults* list, char* string);

void SetScale(KonAudio* konAudio, const char* scalePath);

void FreePath();


extern char defaultSongPath[PATH_MAX];
extern char currentSongPath[PATH_MAX];
extern char currentSongName[PATH_MAX];

extern int configSampleRate;
extern int configBufferSize;
extern char* configTheme;
extern char* configFont;

#endif
