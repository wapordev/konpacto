#include <stdio.h>
#include <stdint.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#if defined _WIN32 || defined __CYGWIN__
   #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#else
   #define DLL_PUBLIC __attribute__ ((visibility ("default")))
#endif

typedef struct KonStep {
   uint8_t note;              //
   uint8_t instrument;           //literal index into loaded instruments
   uint8_t velocity;             
   uint8_t command;
   uint8_t param1;
   uint8_t param2;
   uint8_t param3;
}KonStep;

DLL_PUBLIC KonStep testFunc(int a){
   KonStep new = {
      a,
      a,
      a,
      a,
      a,
      a,
      a,
   };
   return new;
}

__declspec(dllexport) void testFunc2(){
   printf("heyhey");
}

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