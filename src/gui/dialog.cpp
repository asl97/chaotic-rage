// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#include <iostream>
#include <SDL.h>
#include <math.h>
#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include <guichan/opengl.hpp>
#include <guichan/opengl/openglsdlimageloader.hpp>
#include "../rage.h"
#include "dialog.h"


using namespace std;


/**
* Constructor for "new game" dialog
**/
DialogNewGame::DialogNewGame(int num_local)
{
	this->num_local = num_local;
}

/**
* Setup routine for the "New Game" dialog
**/
gcn::Container * DialogNewGame::setup()
{
	gcn::Button* button;
	
	if (this->num_local == 1) {
		c = new gcn::Window("Single Player");
	} else if (this->num_local > 1) {
		c = new gcn::Window("Split Screen");
	}
	
	c->setDimension(gcn::Rectangle(0, 0, 200, 200));
	
	
	this->map = new gcn::DropDown(new VectorListModel(&this->m->maps));
	this->map->setPosition(10, 30);
	c->add(this->map);
	
	this->gametype = new gcn::DropDown(new VectorListModel(&this->m->gametypes));
	this->gametype->setPosition(10, 10);
	c->add(this->gametype);
	
	this->unittype = new gcn::DropDown(new VectorListModel(&this->m->unittypes));
	this->unittype->setPosition(10, 50);
	c->add(this->unittype);
	
	this->viewmode = new gcn::DropDown(new VectorListModel(&this->m->viewmodes));
	this->viewmode->setPosition(10, 70);
	c->add(this->viewmode);
	
	
	button = new gcn::Button("Start Game");
	button->setPosition(120, 150);
	button->addActionListener(this);
	c->add(button);
	
	return c;
}

/**
* Button click processing for the "New Game" dialog
**/
void DialogNewGame::action(const gcn::ActionEvent& actionEvent)
{
	this->m->startGame(
		this->m->maps[this->map->getSelected()],
		this->m->gametypes[this->gametype->getSelected()],
		this->m->unittypes[this->unittype->getSelected()],
		this->viewmode->getSelected(),
		this->num_local
	);
}



/**
* Setup routine for the NULL dialog
**/
gcn::Container * DialogNull::setup()
{
	gcn::Label* label;

	c = new gcn::Window("Argh");
	c->setDimension(gcn::Rectangle(0, 0, 300, 100));
	
	label = new gcn::Label("This action is not available at this time");
	c->add(label, 22, 20);
	
	return c;
}


/**
* Setup routine for the NULL dialog
**/
gcn::Container * DialogControls::setup()
{
	string controls[] = {
		"Move", "W A S D",
		"Aim", "Mouse",
		"Fire", "Left Click",
		"Melee", "Right Click",
		"Change Weapon", "Scroll",
		"Use", "E",
		"Lift", "Q",
		"Special", "T",
		""
	};
	
	gcn::Label* label;
	
	c = new gcn::Window("Controls");
	c->setDimension(gcn::Rectangle(0, 0, 300, 300));
	
	int y = 20;
	int i = 0;
	do {
		if (controls[i].length() == 0) break;
		
		label = new gcn::Label(controls[i]);
		c->add(label, 22, y);
		i++;
		
		label = new gcn::Label(controls[i]);
		label->setAlignment(gcn::Graphics::RIGHT);
		c->add(label, 300 - 22 - label->getWidth(), y);
		i++;
		
		y += 20;
	} while(1);
	
	c->setHeight(y + 30);
	
	return c;
}



/**
* Quit dialog is for in-game quitting
**/
DialogQuit::DialogQuit(GameState *st)
{
	this->st = st;
}


/**
* Setup routine for the NULL dialog
**/
gcn::Container * DialogQuit::setup()
{
	gcn::Button* button;
	
	c = new gcn::Window("Quit? Really?");
	c->setDimension(gcn::Rectangle(0, 0, 300, 100));
	
	button = new gcn::Button("Yes");
	button->setPosition(30, 30);
	button->addActionListener(this);
	c->add(button);
	
	button = new gcn::Button("No");
	button->setPosition(100, 30);
	button->addActionListener(this);
	c->add(button);
	
	return c;
}

/**
* Handle a button click
**/
void DialogQuit::action(const gcn::ActionEvent& actionEvent)
{
	this->st->remDialog(this);
	this->st->running = false;
}


