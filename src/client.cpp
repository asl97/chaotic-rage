// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#include <string>
#include <SDL.h>
#include "rage.h"


using namespace std;



int main (int argc, char ** argv) {
	
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	
	seedRandom();
	
	GameState *st = new GameState();
	
	new RenderOpenGL(st);
	new AudioSDLMixer(st);
	
	st->render->setScreenSize(900, 900, false);
	
	Mod * mod = new Mod(st, "data/cr/");
	if (! mod->load()) {
		reportFatalError("Unable to load data module 'cr'.");
	}
	
	Map *m = new Map(st);
	m->load("arena", st->render);
	st->curr_map = m;
	
	new GameLogic(st);
	
	GameType *gt = mod->getGameType(0);
	st->logic->execScript(gt->script);
	
	gameLoop(st, st->render);
	
	exit(0);
}




