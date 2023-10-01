#include <SDL2/SDL.h>
#include "../lib/lua/src/lua.h"
#include "../lib/lua/src/lualib.h"
#include "../lib/lua/src/lauxlib.h"

#define FALSE 0
#define TRUE 1
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FPS 30
#define FRAME_TIME (1000/FPS)

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
lua_State* L;

struct player {
  float x;
  float y;
  float width;
  float height;
}player;

int initialise_window(void){
  if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "Error initialising SDL\n"); 
    return FALSE; 
  }
  window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS);
  if(window == NULL){
    fprintf(stderr, "Error creating window\n");
    return FALSE;
  }
  renderer = SDL_CreateRenderer(window, -1, 0);
  if(renderer == NULL) {
    fprintf(stderr, "Error creating renderer\n");
    return FALSE;
  }
  return TRUE;
}

void destroy_window() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

int is_running = TRUE;

void process_input(){
  SDL_Event event;
  SDL_PollEvent(&event);
  switch(event.type) {
    case SDL_QUIT: {
      is_running = FALSE;
      break;
    }
    case SDL_KEYDOWN: {
      if(event.key.keysym.sym == SDLK_ESCAPE){
        is_running = FALSE;
        break;
      }
    }
  }
}

void setup(){
  player.x = 20;
  player.y = 20;
  player.width = 10;
  player.height = 10;
}

int last_frame_time = 0;

void update() {
  while( !SDL_TICKS_PASSED(SDL_GetTicks(), last_frame_time + FRAME_TIME) ); 
  float delta_time = (SDL_GetTicks() - last_frame_time)/1000.0; // in seconds
  last_frame_time = SDL_GetTicks(); // time in ms since SDL_init
  
  lua_getglobal(L, "update");
  if(lua_isfunction(L, -1)) {
    const int NUM_ARGS = 1;
    const int NUM_RETURNS = 0;
    lua_pushnumber(L, delta_time);
    lua_pcall(L, NUM_ARGS, NUM_RETURNS, 0);
  }  
}

void render() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
  
  // draw the player at position x, y
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_Rect player_rect = {
    (int)player.x, 
    (int)player.y, 
    (int)player.width, 
    (int)player.height
  };
  SDL_RenderFillRect(renderer, &player_rect);
  SDL_RenderPresent(renderer); // swap buffers and show
}

int set_player_pos(lua_State* L) {
  lua_Number x = lua_tonumber(L, -2); 
  lua_Number y = lua_tonumber(L, -1);
  player.x = (int)x;
  player.y = (int)y;
  return 0;
}

int main(int argc, char* argv[]) {
  L = luaL_newstate();
  luaL_openlibs(L);
  if( luaL_dofile(L, "./scripts/player_movement.lua") != LUA_OK ) {
    luaL_error(L, "Error reading player movement lua: %d %s\n", __LINE__, lua_tostring(L,-1));
    return EXIT_FAILURE;
  }

  // TODO: Create a separate function to define all the lua-C bindings
  lua_pushcfunction(L, set_player_pos);
  lua_setglobal(L, "set_player_pos");

  if(initialise_window() == FALSE) {
    return -1;
  }

  setup();

  while(is_running) {
    process_input();
    update();
    render();
  }

  destroy_window();
  
  return 0;
}
