#include <stdio.h>
#include <string.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "synth.h"
#include "sound.h"
#include "file.h"

#include "luajit.h"

lua_State* lua;

int test = 0;

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

void InitializeLua(){
	lua = luaL_newstate();
	luaL_openlibs(lua);

	int result = luaL_dostring(lua,
	"\n\
	--jit.opt.start('maxtrace=8000', 'maxrecord=16000', 'minstitch=3', 'maxmcode=40960') \n\
	sampleRate = 44100 \n\
	local ffi = require('ffi') \n\
	ffi.cdef'double konGet(int index);' \n\
	ffi.cdef'void konOut(double left, double right);' \n\
	ffi.cdef'void bankSwitch();' \n\
	local channelData = {} \n\
	local loadedSynths = {} \n\
	local synthHash = {} \n\
	local channelSynths = {0,0,0,0,0,0,0,0} \n\
	print('hello world, lua lives!\\n') \n\
-- Load a synth from filepath \n\
	function _kLoad(filePath) \n\
		--[[if(loadedSynths[filePath]) then \n\
			return \n\
		end ]] \n\
		print('trying to load: '..filePath..' ...') \n\
		local untrusted, message = loadfile(filePath,'t') \n\
		if (not untrusted) then \n\
			print('failed to load! '..message) \n\
			return \n\
		end \n\
-- Allowed API \n\
		local env = { \n\
			print = print, \n\
			clamp = math.clamp, \n\
			sin = math.sin, \n\
			pi = math.pi, \n\
			C = ffi.C, \n\
			sampleRate = sampleRate \n\
		} \n\
		setfenv(untrusted,env) \n\
		local result, message = pcall(untrusted) \n\
		if (not result) then \n\
			print('failed to run! '..message) \n\
			return \n\
		end \n\
-- Check if properly formatted \n\
		if(not env._defaultParams or type(env._defaultParams)~='table') then \n\
			print('failed to load! missing _defaultParams.') return \n\
		end \n\
		if(#env._defaultParams>0)then \n\
			for i=1,#env._defaultParams do \n\
				if(type(env._defaultParams[i])~='table' or #env._defaultParams[i]<2)then \n\
					print('failed to load! all in _defaultParams must be a table of PARAMNAME, DEFAULTVAL') return \n\
				end \n\
				if(type(env._defaultParams[i][1])~='string' or type(env._defaultParams[i][2])~='number') then \n\
					print('failed to load! PARAMNAME must be a string, and DEFAULTVAL must be a number') return \n\
				end \n\
			end \n\
		else \n\
			print('failed to load! _defaultParams empty') return \n\
		end \n\
		if(not env._audioFrame or type(env._audioFrame)~='function') then \n\
			print('failed to load! missing _audioFrame.') return \n\
		end \n\
		local index = synthHash[filePath] or #loadedSynths+1 \n\
		synthHash[filePath] = index \n\
		loadedSynths[index]=env \n\
		for i=1,8 do \n\
			if(channelSynths[i]==index)then \n\
				channelData[i]=env._init() \n\
			end \n\
		end \n\
	end \n\
-- Tick Channel \n\
	function _kTick() \n\
		for i=1,8 do \n\
			local synth = loadedSynths[channelSynths[i]] \n\
			if synth then synth._audioFrame(channelData[i]) end \n\
			ffi.C.bankSwitch() \n\
			ffi.C.bankSwitch() \n\
		end \n\
	end \n\
	function _kProf() \n\
		require('jit.p').start('l-3vfsi4m0')\n\
	end \n\
-- Initialize Synth \n\
	function _kInit(path,index) \n\
		path='assets/synths/'..path \n\
		local synthIndex = synthHash[path] \n\
		if(not synthIndex)then return end \n\
		local synth = loadedSynths[synthIndex] \n\
		if(not synth)then return end \n\
		channelSynths[index] = synthIndex \n\
		channelData[index] = synth._init() \n\
	end \n\
-- Get Param Count \n\
	function _kParamCount(path) \n\
		local synthIndex = synthHash[path] \n\
		if(not synthIndex)then return 0 end \n\
		local synth = loadedSynths[synthIndex] \n\
		if(not synth)then return 0 end \n\
		return #synth._defaultParams \n\
	end \n\
-- Get Param Details \n\
	function _kParamGet(path,paramIndex) \n\
		local synthIndex = synthHash[path] \n\
		if(not synthIndex)then return 'error',1 end \n\
		local synth = loadedSynths[synthIndex] \n\
		if(not synth)then return 'error',2 end \n\
		if(not synth._defaultParams[paramIndex])then return 'error',3 end \n\
		return synth._defaultParams[paramIndex][1], synth._defaultParams[paramIndex][2] \n\
	end \n\
	"
	);

	CheckLuaError(result*2,0,"hi");

	lua_pushinteger(lua,configSampleRate);

	lua_setglobal(lua,"sampleRate");
}

void StopLua(){
	lua_close(lua);
}

void SetLuaInstrument(char* path, int index){
	if(path==NULL || path[0]=='\0'){
		return;
	}

	lua_getglobal(lua,"_kInit");

	lua_pushstring(lua,path);

	lua_pushnumber(lua,index+1);


	int result = lua_pcall(lua,2,0,0);

	CheckLuaError(result,index,NULL);
}

void TickLuaChannels(){
	//temp
	if(test==0){
		lua_getglobal(lua,"_kProf");
		int result = lua_pcall(lua,0,0,0);
		CheckLuaError(result,0,NULL);
		test=1;
	}

	lua_getglobal(lua,"_kTick");

	int result = lua_pcall(lua,0,0,0);

	CheckLuaError(result,0,NULL);
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

void LoadLuaFile(char* filePath){
	if(filePath[0]=='\0'){return;}
	lockAudio();

	lua_getglobal(lua,"_kLoad");
	lua_pushstring(lua,filePath);
	int result = lua_pcall(lua,1,0,0);
	CheckLuaError(result,0,filePath);

	luaJIT_setmode(lua, 0, LUAJIT_MODE_ENGINE|LUAJIT_MODE_FLUSH);

	unlockAudio();
}