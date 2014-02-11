// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#pragma once
#include "rage.h"

using namespace std;


namespace gcn {
	class Gui;
	class SDLInput;
	class Container;
}


/**
* Size of the frames-per-second ringbuffer
**/
#define FPS_SAMPLES 100


/**
* Contains pointers to all of the subsystems
*
* How to decide if it belongs here or in the GameState class:
*  - If it should be network-replicated, it belongs over there
*  - If it is local to this instance, it belongs here
**/
class GameEngine
{
	protected:
		// FPS ringbuffer + average
		int ticksum;
		int tickindex;
		int ticklist[FPS_SAMPLES];

		// Is the mouse currently "grabbed"?
		bool mouse_grabbed;

	public:
		// Is the game running?
		bool running;

		// Pointers to all of the subsystems
		Render* render;
		Audio* audio;
		GameLogic* logic;
		NetClient* client;
		NetServer* server;
		PhysicsBullet* physics;
		CommandLineArgs* cmdline;
		ClientConfig* cconf;
		ServerConfig* sconf;
		ModManager* mm;

		// Dialogs and guichan
		list<Dialog*> dialogs;
		gcn::Gui* gui;
		gcn::SDLInput* guiinput;
		gcn::Container* guitop;
		
	public:
		GameEngine();
		~GameEngine();
		
	private:
		// Disable copy-constructor and copy-assignment
		GameEngine(const GameEngine& that);
		GameEngine& operator= (GameEngine that);
		
	public:
		// Mouse grab
		void setMouseGrab(bool reset);
		bool getMouseGrab();

		// Init guichan
		void initGuichan();
		
		// Dialog management
		bool hasDialog(string name);
		void addDialog(Dialog * dialog);
		void remDialog(Dialog * dialog);
		bool hasDialogs();

		// Frames-per-second calcs
		void calcAverageTick(int newtick);
		float getAveTick();
};


/**
* Returns a pointer to the global GameEngine object
**/
GameEngine* GEng();