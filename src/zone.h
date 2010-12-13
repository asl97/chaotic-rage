// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#pragma once
#include <iostream>
#include <SDL.h>
#include "rage.h"

using namespace std;

#define NUM_FACTIONS 10

class Zone
{
	private:
		int x1;
		int y1;
		int x2;
		int y2;
		
	public:
		// Flags for the various jobs a zone may do
		// A zone may do multiple jobs at once
		// and may do the job for multiple teams
		// expect some (interesting) in-fighting when opposing
		// players are sent to the same prison for example...
		//
		// team 0 is for when not playing team games
		//
		bool spawn[NUM_FACTIONS];      // player spawn points
		bool prison[NUM_FACTIONS];     // team prisons
		bool collect[NUM_FACTIONS];    // team collectables - flags, suitcases, balls, etc.
		bool dest[NUM_FACTIONS];       // where collectables are to be taken
		bool nearbase[NUM_FACTIONS];   // somewhere near the team base - for generic use
	
	public:
		Zone(int x, int y, int width, int height);
		~Zone();
		
	public:
		int getRandomX();
		int getRandomY();
};

