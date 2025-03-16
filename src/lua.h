#ifndef WWLUA_H_
#define WWLUA_H_
#include "synth.h"

void InitializeLua();

void StopLua();

void LoadLuaFile(char* filePath);

void SetLuaInstrument(char* path, int index);

void TickLuaChannel(int index);

int CountLuaParam(char* filePath);

int GetLuaParam(char* filePath, int paramIndex, char* dest);

#endif
