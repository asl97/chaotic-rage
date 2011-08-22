// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#pragma once
#include <iostream>
#include <SDL.h>
#include "rage.h"

using namespace std;


class ObjectTypeDamage {
	public:
		int health;
		AnimModel * model;
};

class ObjectType
{
	public:
		// from data file
		string name;
		bool stretch;
		bool wall;
		ObjectType * ground_type;
		
		vector <ObjectTypeDamage *> damage_models;
		vector <AudioPtr> walk_sounds;
		
		AnimModel * model;
		
		int check_radius;
		
		// dynamic
		int id;
		SpritePtr surf;
		
	public:
		ObjectType();
};


// Config file opts
extern cfg_opt_t objecttype_opts [];

// Item loading function handler
ObjectType* loadItemObjectType(cfg_t* cfg_item, Mod* mod);

