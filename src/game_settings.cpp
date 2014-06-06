// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#include "game_settings.h"
#include "game_engine.h"
#include "render_opengl/render_opengl.h"

using namespace std;


/**
* New, blank game settings
**/
GameSettings::GameSettings(int rounds)
{
	this->rounds = rounds;
}

/**
* New, blank per-faction game settings
**/
GameSettingsFaction::GameSettingsFaction()
{
	this->spawn_weapons_unit = true;
	this->spawn_weapons_gametype = true;
}

/**
* Switch view mode
**/
void GameSettings::switchViewMode()
{
	GEng()->render->viewmode = (GEng()->render->viewmode + 1) % nrOfViewModes;
}
