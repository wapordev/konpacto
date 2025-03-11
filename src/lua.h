#ifndef WWLUA_H_
#define WWLUA_H_
#include "synth.h"

void InitializeLua();

void StopLua();

void LoadLuaFile(char* filePath);

void SetLuaInstrument(char* path, int index);

void TickLuaChannel(int index);

#endif
