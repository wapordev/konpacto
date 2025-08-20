#ifndef WWLUA_H_
#define WWLUA_H_
#include "synth.h"

void InitializeLua();

void StopLua();

void LoadLuaFile(char* filePath);

void SetLuaInstrument(char* iPath, char* ePath, int index);

void TickLuaChannels();

int CountLuaParam(char* filePath);

int GetLuaParam(char* filePath, int paramIndex, char* dest);

#endif
