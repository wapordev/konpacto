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

#define paramSize 16

double params[paramSize];
clock_t startTime;


DLL_PUBLIC double get(int index){
   index--;
   if(index<0 || index>=paramSize){
      return 0;
   }
   return params[index];
}

static int cget(lua_State *L){
   lua_Integer index = lua_tointeger(L, 1);  /* get argument */
   
   index--;

   double output=0;

   if(index>=0 && index<paramSize){
      output=params[index];
   }

   lua_pushnumber(L, output);  /* push result */

   return 1;  /* number of results */
}



int main() {
   for(int i=0;i<paramSize;i++){
      params[i]=i;
   }

   lua_State *lua = luaL_newstate();
   luaL_openlibs(lua);

   lua_pushcfunction(lua, cget);
   lua_setglobal(lua, "cget");


   luaL_dofile(lua, "test.lua");
   
   startTime = clock();

   for (int tloop=0;tloop<10000000;tloop++){
      
      lua_getglobal(lua,"funcFFI");

      int result = lua_pcall(lua,0,0,0);

      if (result == LUA_ERRRUN) {
          // get the error object (message)
          const char *err = lua_tostring(lua,-1); // "Nil argument"
          printf("lua error in %s\n",err);
          // pop the error object
          lua_pop(lua,1);
          return 1;
      }
      else if (result == LUA_ERRMEM) {
          printf("lua out of memory!!!\n");
          lua_pop(lua,1);
          return 1;
      }else if (result != 0){
         printf("catastrophic lua error!\n");
         lua_pop(lua,1);
         return 1;
      }
   }

   printf("FFI call: %f\n",(float)(clock() - startTime) / CLOCKS_PER_SEC);

   startTime = clock();

   for (int tloop=0;tloop<10000000;tloop++){
      
      //full userdata
      
      lua_getglobal(lua,"funcC");

      int result = lua_pcall(lua,0,0,0);

      if (result == LUA_ERRRUN) {
          // get the error object (message)
          const char *err = lua_tostring(lua,-1); // "Nil argument"
          printf("lua error in %s\n",err);
          // pop the error object
          lua_pop(lua,1);
          return 1;
      }
      else if (result == LUA_ERRMEM) {
          printf("lua out of memory!!!\n");
          lua_pop(lua,1);
          return 1;
      }else if (result != 0){
         printf("catastrophic lua error!\n");
         lua_pop(lua,1);
         return 1;
      }
   }

   printf("C func wrapper seconds: %f\n",(float)(clock() - startTime) / CLOCKS_PER_SEC);

   startTime = clock();

   for (int tloop=0;tloop<10000000;tloop++){
      
      //full userdata
      
      lua_getglobal(lua,"funcLua");

      for(int i=0;i<paramSize;i++){
         lua_pushnumber(lua,params[i]);
      }

      int result = lua_pcall(lua,paramSize,0,0);

      if (result == LUA_ERRRUN) {
          // get the error object (message)
          const char *err = lua_tostring(lua,-1); // "Nil argument"
          printf("lua error in %s\n",err);
          // pop the error object
          lua_pop(lua,1);
          return 1;
      }
      else if (result == LUA_ERRMEM) {
          printf("lua out of memory!!!\n");
          lua_pop(lua,1);
          return 1;
      }else if (result != 0){
         printf("catastrophic lua error!\n");
         lua_pop(lua,1);
         return 1;
      }
   }

   printf("Passing %i parameters straight: %f\n",paramSize, (float)(clock() - startTime) / CLOCKS_PER_SEC);
   

   lua_close(lua);

   return 0;
}