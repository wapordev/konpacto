#ifndef WWLUA_H_
#define WWLUA_H_
#include "synth.h"

void InitializeLua();

void StopLua();

void LoadLuaFile(char* filePath);

void TickLuaChannel(double* leftOut, double* rightOut, KonChannel channel);

#endif
