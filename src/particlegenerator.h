// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#pragma once
#include <iostream>
#include <SDL.h>
#include "rage.h"

using namespace std;

#define MAX_SPEWERS 32

class ParticleGenerator : public Entity
{
	public:
		virtual EntityClass klass() const { return PGENERATOR; }
		
	public:
		ParticleGenType* type;
		int age;
		int spewdelay[MAX_SPEWERS];
		
	public:
		ParticleGenerator(ParticleGenType* type, GameState *st);
		virtual ~ParticleGenerator();
		
	public:
		virtual void update(int delta);
		virtual AnimPlay* getAnimModel();
		virtual Sound* getSound();
};
