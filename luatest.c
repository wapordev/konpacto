#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#if defined _WIN32 || defined __CYGWIN__
   #define DLL_PUBLIC __declspec(dllexport)
#else
   #define DLL_PUBLIC
#endif

DLL_PUBLIC double CFunction(){
   return 777;
}

int main() {
   printf("int main(): hello test build\n\n");

   lua_State *lua = luaL_newstate();
   luaL_openlibs(lua);
   luaL_dofile(lua, "test.lua");

   lua_getglobal(lua,"luaFunction");
   int result = lua_pcall(lua,0,0,0);

   lua_close(lua);

   return 0;
}