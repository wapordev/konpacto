#include <stdio.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "synth.h"
#include "sound.h"

lua_State* lua;

void InitializeLua(){
	lua = luaL_newstate();
	luaL_openlibs(lua);

	luaL_dostring(lua,
	"\
	local loadedSynths = {} \
	print('hello world') \
-- Load a synth from filepath \n\
	function _kLoad(filePath) \
		--[[if(loadedSynths[filePath]) then \
			return \
		end ]]\
		print('trying to load: '..filePath..' ...') \
		local untrusted, message = loadfile(filePath,'t') \
		if (not untrusted) then \
			print('failed to load! '..message) \
			return \
		end \
-- Allowed API \n\
		local env = { \
			print = print, \
			clamp = math.clamp \
		} \
		setfenv(untrusted,env) \
		local result, message = pcall(untrusted) \
		if (not result) then \
			print('failed to run! '..message) \
			return \
		end \
-- Check if properly formatted \n\
		if(not env._defaultParams or type(env._defaultParams)~='table') then \
			print('failed to load! missing _defaultParams.') return \
		end \
		if(#env._defaultParams>0)then \
			for i=1,#env._defaultParams do \
				if(type(env._defaultParams[i])~='table' or #env._defaultParams[i]<2)then \
					print('failed to load! all in _defaultParams must be a table of PARAMNAME, DEFAULTVAL') return \
				end \
				if(type(env._defaultParams[i][1])~='string' or type(env._defaultParams[i][2])~='number') then \
					print('failed to load! PARAMNAME must be a string, and DEFAULTVAL must be a number') return \
				end \
			end \
		else \
			print('failed to load! _defaultParams empty') return \
		end \
		if(not env._audioFrame or type(env._audioFrame)~='function') then \
			print('failed to load! missing _audioFrame.') return \
		end \
		loadedSynths[filePath]=env \
	end \
-- Tick Channel \n\
	function _kTick(path,data) \
		path='assets/synths/'..path \
		if (loadedSynths[path]) then \
			return loadedSynths[path]._audioFrame(data) \
		end \
	end \
	"
	);
}

void StopLua(){
	lua_close(lua);
}

void SetLuaInstrument(){

}

void TickLuaChannel(double* leftOut, double* rightOut, KonChannel channel){
	if(!channel.synthData.note){
		return;
	}
	if(!channel.synthData.instrument){
		return;
	}

	char* synthPath = konAudio.instruments[channel.synthData.instrument-1].selectedSynth;

	if(synthPath==NULL || synthPath[0]=='\0'){
		return;
	}

	lua_getglobal(lua,"_kTick");	

	lua_pushstring(lua,synthPath);

	lua_createtable(lua,3,0);

	lua_pushnumber(lua,1);
	lua_pushnumber(lua,channel.on);
	lua_settable(lua,-3);

	lua_pushnumber(lua,2);

	double note = konAudio.frequencies[channel.synthData.note-1];

	lua_pushnumber(lua,note);

	lua_settable(lua,-3);

	lua_pushnumber(lua,3);
	lua_pushnumber(lua,konAudio.format.frequency);
	lua_settable(lua,-3);


	int result = lua_pcall(lua,2,2,0);

	if (result == LUA_ERRRUN) {
	    // get the error object (message)
	    const char *err = lua_tostring(lua,-1); // "Nil argument"
	    printf("lua error: %s\n",err);
	    // pop the error object
	    lua_pop(lua,1);
	    return;
	}
	else if (result == LUA_ERRMEM) {
	    printf("lua out of memory!!!\n");
	    lua_pop(lua,1);
	    return;
	}else if (result != 0){
		printf("catastrophic lua error!\n");
		lua_pop(lua,1);
		return;
	}

	*rightOut = lua_tonumber(lua,-1);
	lua_pop(lua,1);
	*leftOut = lua_tonumber(lua,-1);
	lua_pop(lua,1);
}

void LoadLuaFile(char* filePath){
	if(filePath[0]=='\0'){return;}
	lockAudio();

	lua_getglobal(lua,"_kLoad");
	lua_pushstring(lua,filePath);
	lua_call(lua,1,0);

	unlockAudio();
}