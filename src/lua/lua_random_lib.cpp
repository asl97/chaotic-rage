// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;


#include <stdio.h>
#include <math.h>
#include "../rage.h"

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
}


#define LUA_FUNC(name) static int name(lua_State *L)
#define LUA_REG(name) lua_register(L, #name, name)


/**
* Return a random number between min and max
*
* @return String: The currently selected unit type
**/
LUA_FUNC(random)
{
	float min = lua_tonumber(L, 1);
	float max = lua_tonumber(L, 2);
	
	lua_pushnumber(L, getRandomf(min,max));
	
	return 1;
}


/**
* Loads the library into a lua state
**/
void load_random_lib(lua_State *L)
{
	LUA_REG(random);
}
