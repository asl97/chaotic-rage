// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;


#include <stdio.h>
#include <math.h>
#include "lua_libs.h"
#include "../rage.h"
#include "../game_engine.h"
#include "../game_state.h"
#include "../mod/mod_manager.h"
#include "../mod/vehicletype.h"
#include "../mod/objecttype.h"
#include "../entity/vehicle.h"
#include "../entity/helicopter.h"
#include "../entity/object.h"

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
}


#define LUA_FUNC(name) static int name(lua_State *L)
#define LUA_REG(name) lua_register(L, #name, name)


/**
* Add a vehicle at the given coordinate
**/
LUA_FUNC(add_vehicle)
{
	Vehicle *v;
	
	VehicleType *vt = GEng()->mm->getVehicleType(*(new string(lua_tostring(L, 1))));
	if (vt == NULL) {
		return luaL_error(L, "Arg #1 is not an available vehicle type");
	}
	
	double * pos = get_vector3(L, 2);
	
	if (vt->helicopter) {
		v = new Helicopter(vt, getGameState(), pos[0], pos[2]);
	} else {
		v = new Vehicle(vt, getGameState(), pos[0], pos[2]);
	}
	
	getGameState()->addVehicle(v);
	
	return 0;
}


/**
* Add a vehicle at the given coordinate
**/
LUA_FUNC(add_object)
{
	Object *o;
	
	ObjectType *ot = GEng()->mm->getObjectType(*(new string(lua_tostring(L, 1))));
	if (ot == NULL) {
		return luaL_error(L, "Arg #1 is not an available Object type");
	}
	
	double * pos = get_vector3(L, 2);
	
	o = new Object(ot, getGameState(), pos[0], pos[1], pos[2], 0.0f);
	
	getGameState()->addObject(o);
	
	return 0;
}


/**
* Loads the library into a lua state
**/
void load_entity_lib(lua_State *L)
{
	LUA_REG(add_vehicle);
	LUA_REG(add_object);
}

