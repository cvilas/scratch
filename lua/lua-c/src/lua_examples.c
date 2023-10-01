#include <stdio.h>
#include "../lib/lua/src/lua.h"
#include "../lib/lua/src/lualib.h"
#include "../lib/lua/src/lauxlib.h"
#include <math.h>

void lua_example_dofile(void) {
  lua_State* L = luaL_newstate();
  luaL_openlibs(L); 
  luaL_dofile(L, "./scripts/factorial.lua");
  lua_close(L);
}

void lua_example_getvar(void) {
  lua_State* L = luaL_newstate();
  luaL_dostring(L, "some_var = 486"); // A lua code
  lua_getglobal(L, "some_var"); // access the variable and put it in stack
  lua_Number some_var_in_c = lua_tonumber(L, -1); // pop it from stack
  printf("The value of some_var is %d\n", (int)some_var_in_c);
  lua_close(L);
}

void lua_example_stack(void) {
  lua_State* L = luaL_newstate();
  lua_pushnumber(L, 286); // stack [1] [-3]
  lua_pushnumber(L, 386); // stack [2] [-2]
  lua_pushnumber(L, 486); // stack [3] [-1]
  lua_Number element;
  element = lua_tonumber(L, -1);
  printf("The last element added to stack is %d\n", (int)element);

  lua_remove(L, 2); // remove the second element from stack
  element = lua_tonumber(L, 2);
  printf("Position 2 of the stack is no: %d\n", (int)element);
  lua_close(L);
}

void lua_example_call_lua_function(void) {
  lua_State* L = luaL_newstate();
  luaL_openlibs(L); // opens lua libraries so we have access to math.sqrt() and print() from the lua file
  if(LUA_OK != luaL_dofile(L, "./scripts/pythagoras.lua")) {
    luaL_error(L, "Error %d: %s\n", __LINE__, lua_tostring(L, -1)); // any errors will be on the stack
  }
  lua_getglobal(L, "pythagoras");
  if(lua_isfunction(L, -1)) {
    lua_pushnumber(L, 3); // first argument 'a' for function pushed to stack
    lua_pushnumber(L, 4); // second argument 'b' for function pushed to stack
    const int NUM_ARGS = 2;
    const int NUM_RETURNS = 1;
    if(LUA_OK != lua_pcall(L, NUM_ARGS, NUM_RETURNS, 0/*msg handler*/)) { // call the lua function. It will take arguments from the stack
      luaL_error(L, "Error: %d: %s\n", __LINE__, lua_tostring(L, -1));
    }
    lua_Number pythagoras_result = lua_tonumber(L, -1); // get results back from lua function from stack
    printf("The pythagoras result is %f\n", (float)pythagoras_result);
  }
  lua_close(L);
}

int native_pythagoras(lua_State* L) { // returns the number of results
  lua_Number b = lua_tonumber(L, -1); // get parameters in the opposite order to push
  lua_Number a = lua_tonumber(L, -2);
  lua_Number result = sqrt(a*a + b*b);
  lua_pushnumber(L, result);
  return 1; // returns how many values the function is returning
}

void lua_example_call_c_function(void) {
  lua_State* L = luaL_newstate();
  lua_pushcfunction(L, native_pythagoras);
  lua_setglobal(L, "native_pythagoras"); // exposing a variable for lua to access
  luaL_dofile(L, "./scripts/pythagoras-native.lua");
  lua_getglobal(L, "pythagoras");
  if(lua_isfunction(L, -1)) {
    const int a = 3;
    const int b = 4;
    lua_pushnumber(L, a);
    lua_pushnumber(L, b); // push arguments
    const int NUM_ARGS = 2;
    const int NUM_RETURNS = 1;
    lua_pcall(L, NUM_ARGS, NUM_RETURNS, 0);
    lua_Number pyth_result = lua_tonumber(L, -1);
    printf("Native pythagoras = %f\n", (float)pyth_result); 
  }
  lua_close(L);
}

typedef struct rectangle2d {
  int x;
  int y;
  int width;
  int height;
} rectangle;

int create_rectangle(lua_State* L) {
  rectangle* rect = (rectangle*)lua_newuserdata(L, sizeof(rectangle));
  rect->x = 0;
  rect->y = 0;
  rect->width = 0;
  rect->height = 0;
  return 1; // return our own type as user data so Lua can access it
}

int change_rectangle_size(lua_State* L) {
  rectangle* rect = (rectangle*)lua_touserdata(L, -3);
  rect->width = lua_tonumber(L, -2);
  rect->height = lua_tonumber(L, -1);
  return 0; // does not return anything back to the stack
}

void lua_example_userdata(void) {
  lua_State* L = luaL_newstate();

  lua_pushcfunction(L, create_rectangle); // expose C native function
  lua_setglobal(L, "create_rectangle");

  lua_pushcfunction(L, change_rectangle_size);
  lua_setglobal(L, "change_rectangle_size");

  luaL_dofile(L, "./scripts/rectangle.lua");
  lua_getglobal(L, "square");
  if(lua_isuserdata(L, 1)) {
    rectangle* r = (rectangle*)lua_touserdata(L,-1);
    printf("We got back a rectangle from lua, width: %d, height: %d\n", r->width, r->height);
  } else {
    printf("We did not get a rectangle user data");
  }

  lua_close(L);
}

void lua_example_table(void) {
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  if(luaL_dofile(L, "./scripts/configtable.lua") == LUA_OK) {
    lua_getglobal(L, "config_table");
    if(lua_istable(L, -1)){
      lua_getfield(L, -1, "window_height");
      printf("Window width defined in the lua table is %s\n", lua_tostring(L, -1));
    }
  } else {
    luaL_error(L, "Error: %d: %s\n", __LINE__, lua_tostring(L, -1));
  }
  lua_close(L);
}

int main(int argc, char* argv[]) {
  //lua_example_dofile();
  //lua_example_getvar();
  //lua_example_stack();
  //lua_example_call_lua_function();
  //lua_example_call_c_function();
  //lua_example_userdata();
  lua_example_table();
  return 0;
}

