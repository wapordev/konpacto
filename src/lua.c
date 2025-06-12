#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "synth.h"
#include "sound.h"
#include "file.h"

#include "luajit.h"

lua_State* lua;

void InitializeLua(){
	lua = luaL_newstate();
	luaL_openlibs(lua);

	luaL_dostring(lua,
	"\
	sampleRate = 44100 \
	local ffi = require('ffi') \
	ffi.cdef'double konGet(int index);' \
	ffi.cdef'void konOut(double left, double right);' \
	local channelSynths = {} \
	print('hello world, lua lives!\\n') \
-- Load a synth from filepath \n\
	function _kLoad(filePath) \
		--[[if(loadedSynths[filePath]) then \
			return -1 \
		end ]] \
		print('trying to load: '..filePath..' ...') \
		local untrusted, message = loadfile(filePath,'t') \
		if (not untrusted) then \
			print('failed to load! '..message) \
			return -1 \
		end \
-- Allowed API \n\
		local env = { \
			print = print, \
			clamp = math.clamp, \
			sin = math.sin, \
			pi = math.pi, \
			C = ffi.C, \
			sampleRate = sampleRate \
		} \
		setfenv(untrusted,env) \
		local result, message = pcall(untrusted) \
		if (not result) then \
			print('failed to run! '..message) \
			return -1 \
		end \
-- Check if properly formatted \n\
		if(not env._defaultParams or type(env._defaultParams)~='table') then \
			print('failed to load! missing _defaultParams.') return -1 \
		end \
		if(#env._defaultParams>0)then \
			for i=1,#env._defaultParams do \
				if(type(env._defaultParams[i])~='table' or #env._defaultParams[i]<2)then \
					print('failed to load! all in _defaultParams must be a table of PARAMNAME, DEFAULTVAL') return -1 \
				end \
				if(type(env._defaultParams[i][1])~='string' or type(env._defaultParams[i][2])~='number') then \
					print('failed to load! PARAMNAME must be a string, and DEFAULTVAL must be a number') return -1 \
				end \
			end \
		else \
			print('failed to load! _defaultParams empty') return -1 \
		end \
		if(not env._audioFrame or type(env._audioFrame)~='function') then \
			print('failed to load! missing _audioFrame.') return -1 \
		end \
		returnParams={}\
		for i=1,#env._defaultParams do \
			returnParams[i*2-1]=env._defaultParams[i][1] \
			returnParams[i*2]=env._defaultParams[i][2] \
		end \
		return env \
	end \
	function _kCheck(filePath) \
		local env = _kLoad(filePath) \
		if env ~= -1 then \
			returnParams[#env._defaultParams*2+1]=#env._defaultParams \
			return unpack(returnParams) \
		end \
	end \
-- Tick Channel \n\
	function _kTick(index) \
		local synth = channelSynths[index] \
		if(not synth)then return 0,0 end \
		return synth._audioFrame() \
	end \
-- Initialize Synth \n\
	function _kInit(index) \
		local synth = channelSynths[index] \
		if(not synth)then return end \
		synth._init() \
	end \
-- Set Synth \n\
	function _kSet(filePath,index) \
		local env = _kLoad(filePath) \
		if env ~= -1 then \
			channelSynths[index] = env \
		end \
	end \
-- Get Param Count \n\
	function _kParamCount(path) \
		local synth = channelSynths[index] \
		if(not synth)then return end \
		return #synth._defaultParams \
	end \
-- Get Param Details \n\
	function _kParamGet(path,paramIndex) \
		local synth = channelSynths[index] \
		if(not synth)then return 'error',2 end \
		if(not synth._defaultParams[paramIndex])then return 'error',3 end \
		return synth._defaultParams[paramIndex][1], synth._defaultParams[paramIndex][2] \
	end \
	"
	);


	lua_pushinteger(lua,configSampleRate);

	lua_setglobal(lua,"sampleRate");
}

int CheckLuaError(int result, int index, char* synthPath){

	if (result == LUA_ERRRUN) {
	    // get the error object (message)
	    const char *err = lua_tostring(lua,-1); // "Nil argument"

	    char* name;
	    if(synthPath=NULL){
	    	KonChannel channel = konAudio.channels[index];
	    	name = konAudio.instruments[channel.synthData.instrument-1].selectedSynth;
	    }else{
	    	name=synthPath;
	    }
	  
	    printf("lua error in %s: %s\n",name,err);
	    // pop the error object
	    lua_pop(lua,1);
	    return 0;
	}
	else if (result == LUA_ERRMEM) {
	    printf("lua out of memory!!!\n");
	    lua_pop(lua,1);
	    return 0;
	}else if (result != 0){
		printf("catastrophic lua error!\n");
		lua_pop(lua,1);
		return 0;
	}
	return 1;
}

void StopLua(){
	lua_close(lua);
}

void SetLuaInstrument(char* path, int index){
	if(path==NULL || path[0]=='\0'){
		return;
	}

	lua_getglobal(lua,"_kSet");

	lua_pushstring(lua,path);

	lua_pushnumber(lua,index+1);


	int result = lua_pcall(lua,2,0,0);

	CheckLuaError(result,index,NULL);

	luaJIT_setmode(lua, 0, LUAJIT_MODE_ENGINE|LUAJIT_MODE_FLUSH);
}

void TickLuaChannel(int index){
	lua_getglobal(lua,"_kTick");

	lua_pushnumber(lua,index+1);

	int result = lua_pcall(lua,1,0,0);

	CheckLuaError(result,index,NULL);
}

int CountLuaParam(char* filePath){
	lua_getglobal(lua,"_kParamCount");

	lua_pushstring(lua,filePath);

	int result = lua_pcall(lua,1,0,0);

	if(CheckLuaError(result,0,filePath)){
		return lua_tointeger(lua,0);
	}else{
		return 0;
	}
}

int GetLuaParam(char* filePath, int paramIndex, char* dest){
	lua_getglobal(lua,"_kParamGet");

	lua_pushstring(lua,filePath);

	lua_pushnumber(lua,paramIndex+1);

	int result = lua_pcall(lua,2,2,0);

	if(CheckLuaError(result,0,filePath)){
		const char* string = lua_tolstring(lua,-2,NULL);
		if(string!=NULL){
			strncpy(dest,string,15);
			int defaultVal = lua_tointeger(lua,-1);
			defaultVal=clamp(defaultVal,0,255);
			lua_pop(lua,2);
			return defaultVal;
		}else{
			lua_pop(lua,2);
			return -1;
		}
	}else{
		return -1;
	}
}

void LuaConfigInstrument(KonInstrument* instrument, char* filePath){
	if(filePath[0]=='\0'){return;}
	lockAudio();

	lua_getglobal(lua,"_kCheck");
	lua_pushstring(lua,filePath);
	lua_call(lua,1,LUA_MULTRET);

	int paramCount = lua_tointeger(lua,-1);
	lua_pop(lua,1);
	printf("%i\n",paramCount);

	if(paramCount+3<instrument->macroCount){
		for(int i=paramCount+3;i<instrument->macroCount;i++){
			if(instrument->macros[i].length){
				free(instrument->macros[i].data);
				instrument->macros[i].length=0;
			}
		}
	}

	instrument->macroCount=paramCount+3;

	for(int i=paramCount;i>0;i--){
		int macroDefaultValue = lua_tointeger(lua,-1);
		lua_pop(lua,1);
		const char* macroName = lua_tolstring(lua,-1,NULL);
		lua_pop(lua,1);

		int macroIndex = i+2;
		KonMacro* macro = &instrument->macros[macroIndex];

		strcpy(macro->name,macroName);
		macro->defaultValue = macroDefaultValue;

		if(macro->length==0){
			macro->loopStart=1;
		}
	}


	luaJIT_setmode(lua, 0, LUAJIT_MODE_ENGINE|LUAJIT_MODE_FLUSH);

	unlockAudio();

	return;
}