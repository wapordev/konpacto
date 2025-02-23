#include <stdio.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>


int main() {
   // setup lua
   lua_State *L = luaL_newstate();
   luaL_openlibs(L);

   // Run a lua string
   // luaL_dostring(L, "print(\"hello from lua\")");

   luaL_dofile(L, "test.lua");
   

   lua_close(L);

   return 0;
}