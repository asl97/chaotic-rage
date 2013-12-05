// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#include <iostream>
#include <SDL.h>
#include <math.h>
#include "render_opengl/gl.h"

#include "guichan/guichan.hpp"
#include "guichan/sdl.hpp"
#include "guichan/opengl.hpp"
#include "guichan/opengl/openglsdlimageloader.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rage.h"
#include "menu.h"
#include "render_opengl/render_opengl.h"
#include "render/sprite.h"
#include "audio/audio.h"
#include "gamestate.h"
#include "gui/client_settings.h"
#include "gui/newcampaign.h"
#include "gui/newgame.h"
#include "gui/network.h"
#include "gui/mods.h"
#include "gui/controls.h"
#include "mod/campaign.h"
#include "net/net.h"
#include "net/net_client.h"
#include "net/net_server.h"
#include "net/net_gameinfo.h"
#include "util/obj.h"
#include "util/serverconfig.h"
#include "mod/mod_manager.h"
#include "map.h"
#include "lua/gamelogic.h"

using namespace std;


Menu::Menu(GameState *st)
{
	this->st = st;
	this->mm = st->mm;
	this->render = (RenderOpenGL*) st->render;
	this->dialog = NULL;
	this->mapreg = NULL;
	this->logo = NULL;
	this->bg = NULL;
	this->bgmesh = NULL;
}


/**
* Load stuff from the currently loaded mods into some arrays.
**/
void Menu::loadModBits(UIUpdate* ui)
{
	Mod* mod = st->mm->getSupplOrBase();

	// Logo
	delete(this->logo);
	this->logo = this->render->loadSprite("menu/logo.png", mod);
	if (!logo) {
		this->logo = this->render->loadSprite("menu/logo.png", st->mm->getBase());
	}

	if (ui) ui->updateUI();

	// Background
	delete(this->bg);
	this->bg = this->render->loadSprite("menu/bg.jpg", mod);
	if (!bg) {
		this->bg = this->render->loadSprite("menu/bg.jpg", st->mm->getBase());
	}

	if (ui) ui->updateUI();

	// Load maps
	delete(this->mapreg);
	this->mapreg = new MapRegistry();
	this->mapreg->find(mod);
	this->mapreg->find("maps");

	if (ui) ui->updateUI();

	// Gametypes
	this->gametypes.clear();
	{
		vector<GameType*> * ut = st->mm->getAllGameTypes();
		for (vector<GameType*>::iterator it = ut->begin(); it != ut->end(); it++) {
			this->gametypes.push_back((*it)->name);
		}
		delete(ut);
	}

	// Viewmodes
	this->viewmodes.clear();
	this->viewmodes.push_back("Behind player");
	this->viewmodes.push_back("Above player");
	this->viewmodes.push_back("First person");
	
	// Unittypes
	this->unittypes.clear();
	{
		vector<UnitType*> * ut = st->mm->getAllUnitTypes();
		for (vector<UnitType*>::iterator it = ut->begin(); it != ut->end(); it++) {
			if ((*it)->playable) this->unittypes.push_back((*it)->name);
		}
		delete(ut);
	}
	
	// Main menu items
	this->menuClear();
	int y = render->getHeight() - 60;
	
	this->menuAdd("Quit", 40, y, MC_QUIT);
	y -= 40;
	
	this->menuAdd("Help", 40, y, MC_HELP);
	y -= 40;
	
	this->menuAdd("Mods", 40, y, MC_MODS);
	y -= 40;
	
	this->menuAdd("Settings", 40, y, MC_SETTINGS);
	y -= 40;
	
	if (mod->hasArcade()) {
		this->menuAdd("Network Game", 40, y, MC_NETWORK);
		y -= 40;
		
		this->menuAdd("Split Screen", 40, y, MC_SPLITSCREEN);
		y -= 40;
		
		this->menuAdd("Single Player", 40, y, MC_SINGLEPLAYER);
		y -= 40;
	}
	
	if (mod->hasCampaign()) {
		this->menuAdd("Campaign", 40, y, MC_CAMPAIGN);
	}
}


/**
* Create the "background" mesh
* We save it in a WavefrontObj.
**/
void Menu::createBGmesh()
{
	if (bgmesh != NULL) return;
	
	Vertex v;
	TexUV t;
	Face f;
	
	bgmesh = new WavefrontObj();
	v.y = 0.0;
	
	v.x = -1.0f; v.z = -1.0f; t.x = 0.0f; t.y = 0.0f;
	bgmesh->vertexes.push_back(v);
	bgmesh->texuvs.push_back(t);
	
	v.x = -1.0f; v.z = 1.0f; t.x = 0.0f; t.y = 1.0f;
	bgmesh->vertexes.push_back(v);
	bgmesh->texuvs.push_back(t);
	
	v.x = 1.0f; v.z = 1.0f; t.x = 1.0f; t.y = 1.0f;
	bgmesh->vertexes.push_back(v);
	bgmesh->texuvs.push_back(t);
	
	v.x = 1.0f; v.z = -1.0f; t.x = 1.0f; t.y = 0.0f;
	bgmesh->vertexes.push_back(v);
	bgmesh->texuvs.push_back(t);
	
	v.x = 0.0f; v.y = 1.0f; v.z = 0.0f;
	bgmesh->normals.push_back(v);
	
	f.v1 = 1; f.v2 = 2; f.v3 = 3;
	f.t1 = 1; f.t2 = 2; f.t3 = 3;
	f.n1 = 1; f.n2 = 1; f.n3 = 1;
	bgmesh->faces.push_back(f);
	
	f.v1 = 1; f.v2 = 3; f.v3 = 4;
	f.t1 = 1; f.t2 = 3; f.t3 = 4;
	f.n1 = 1; f.n2 = 1; f.n3 = 1;
	bgmesh->faces.push_back(f);
}


/**
* Run the menu
* The UIUpdate is for the first-run on the menu, duing initial loading of the menu stuff
**/
void Menu::doit(UIUpdate* ui)
{
	Mod *mod = st->mm->getBase();
	
	
	// A few bits always load off base
	this->render->loadFont("orb.otf", mod);
	
	if (ui) ui->updateUI();

	this->input = new gcn::SDLInput();
	this->gui = new gcn::Gui();
	this->gui->setInput(input);
	this->render->initGuichan(gui, mod);
	
	if (ui) ui->updateUI();

	this->createBGmesh();
	this->bg_rot1_pos = -10.0f;
	this->bg_rot1_dir = 0.006f;
	this->bg_rot2_pos = 3.0f;
	this->bg_rot2_dir = -0.004f;
	
	if (ui) ui->updateUI();

	this->loadModBits(ui);
	this->setupGLstate();
	
	// Menu loop
	this->running = true;
	while (this->running) {
		this->updateUI();
	}
}


/**
* Set the OpenGL state just the way we like it.
**/
void Menu::setupGLstate()
{
	this->render->mainViewport(0, 0);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// Ortho for menu stuff
	render->ortho = glm::ortho<float>(0.0f, (float)render->real_width, (float)render->real_height, 0.0f, 0.0f, 10.0f);
	
	GLShader *shader = render->shaders["basic"];
	glUseProgram(shader->p());
	glUniformMatrix4fv(shader->uniform("uMVP"), 1, GL_FALSE, glm::value_ptr(render->ortho));
	
	this->st->setMouseGrab(false);
}


/**
* Update the UI for the menu
**/
void Menu::updateUI()
{
	int mousex, mousey;
	SDL_Event event;
	
	// Menu hover
	SDL_GetMouseState(&mousex, &mousey);
	this->menuHover(mousex, mousey);
	
	// Events
	MenuCommand cmd = MC_NOTHING;
	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_QUIT) {
			cmd = MC_QUIT;
			
		} else if (event.type == SDL_MOUSEBUTTONUP) {
			cmd = this->menuClick(mousex, mousey);
			
		} else if (event.type == SDL_KEYDOWN) {
			// Key press
			switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					if (this->dialog != NULL) {
						this->setDialog(NULL);
					} else {
						cmd = MC_QUIT;
					}
					break;

				case SDLK_PRINTSCREEN:
					{
						string filename = getUserDataDir();
						filename.append("menu_screenshot.bmp");
						render->saveScreenshot(filename);
					}
					break;

				default: break;
			}
			
		}

		input->pushInput(event);
	}
	
	// Handle main menu commands
	switch (cmd) {
		case MC_CAMPAIGN: this->doCampaign(); break;
		case MC_SINGLEPLAYER: this->doSingleplayer(); break;
		case MC_SPLITSCREEN: this->doSplitscreen(); break;
		case MC_NETWORK: this->doNetwork(); break;
		case MC_SETTINGS: this->doSettings(); break;
		case MC_MODS: this->doMods(); break;
		case MC_HELP: this->doHelp(); break;
		case MC_QUIT: this->doQuit(); break;
		default: break;
	}
	
	// Begin render
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Menu items
	this->menuRender();
	
	// Logo in top-left
	glUseProgram(render->shaders["basic"]->p());
	render->renderSprite(logo, 40, 40);
	
	// If a guichan dialog is set, render it and process events
	if (this->dialog != NULL) {
		gui->logic();
		gui->draw();
	}
	
	SDL_GL_SwapWindow(render->window);
}


/**
* Clear all of the current menu items
**/
void Menu::menuClear()
{
	for (vector<MenuItem*>::iterator it = this->menuitems.begin(); it != this->menuitems.end(); it++) {
		delete(*it);
	}
	this->menuitems.clear();
	this->menuitems.reserve(7);
}


/**
* Adds a menu item to the main menu
**/
void Menu::menuAdd(string name, int x, int y, MenuCommand cmd)
{
	MenuItem * nu = new MenuItem();
	this->menuitems.push_back(nu);
	
	nu->x1 = x;
	nu->x2 = x + 200;
	nu->y1 = y;
	nu->y2 = y + 20;
	nu->name = name;
	nu->cmd = cmd;
	nu->hover = false;
}


/**
* Renders the main menu
**/
void Menu::menuRender()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	
	for (unsigned int i = 0; i < this->menuitems.size(); i++) {
		MenuItem * m = this->menuitems.at(i);
		
		render->renderText(m->name, m->x1 + 1, m->y1 + 20 + 1, 0.1f, 0.1f, 0.1f);
		
		if (m->hover) {
			render->renderText(m->name, m->x1, m->y1 + 20, 161.0f/255.0f, 0.0f, 0.0f);
		} else {
			render->renderText(m->name, m->x1, m->y1 + 20, 1.0f, 1.0f, 1.0f);
		}
	}
}


/**
* Looks for mouse hovers on the main menu
**/
void Menu::menuHover(int x, int y)
{
	for (unsigned int i = 0; i < this->menuitems.size(); i++) {
		MenuItem * m = this->menuitems.at(i);
		
		if (x >= m->x1 and x <= m->x2 and y >= m->y1 and y <= m->y2) {
			m->hover = true;
		} else {
			m->hover = false;
		}
	}
}


/**
* Handles mouse clicks on the main menu, returning the appropriate MenuCommand
**/
MenuCommand Menu::menuClick(int x, int y)
{
	for (unsigned int i = 0; i < this->menuitems.size(); i++) {
		MenuItem * m = this->menuitems.at(i);
		
		if (x >= m->x1 and x <= m->x2 and y >= m->y1 and y <= m->y2) {
			return m->cmd;
		}
	}
	
	return MC_NOTHING;
}



/**
* Sets the current menu dialog, or NULL to hide the dialog
**/
void Menu::setDialog(Dialog * dialog)
{
	this->dialog = NULL;
	if (dialog == NULL) return;
	
	dialog->m = this;
	gcn::Container * c = dialog->setup();
	
	c->setPosition((this->render->real_width - c->getWidth()) / 2, (this->render->real_height - c->getHeight()) / 2);
	c->setBaseColor(gcn::Color(150, 150, 150, 200));
	
	gui->setTop(c);
	this->dialog = dialog;
}



void Menu::doCampaign()
{
	this->setDialog(new DialogNewCampaign(1, this->mm->getSupplOrBase()));
}

void Menu::doSingleplayer()
{
	this->setDialog(new DialogNewGame(1, this->st->mm));
}

void Menu::doSplitscreen()
{
	this->setDialog(new DialogNewGame(2, this->st->mm));
}

void Menu::doNetwork()
{
	this->setDialog(new DialogNetJoin());
}

void Menu::doSettings()
{
	this->setDialog(new DialogClientSettings(this->st));
}

void Menu::doMods()
{
	this->setDialog(new DialogMods(this->st, this->st->mm));
}

void Menu::doHelp()
{
	this->setDialog(new DialogControls());
}

void Menu::doQuit()
{
	this->running = false;
}



/**
* Plays through each game of a campaign
**/
void Menu::startCampaign(Campaign* c, string unittype, int viewmode, unsigned int num_local)
{
	for (vector<CampaignStage*>::iterator it = c->stages->begin(); it != c->stages->end();) {
		CampaignStage* stage = *it;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (stage->map != "") {
			///-- Game stage --
			MapReg* m = mapreg->get(stage->map);
			if (m == NULL) return;

			this->startGame(m, stage->gametype, unittype, 0, 1, false);

			// A result of 1 is success, 0 is failure, -1 is an error.
			int result = st->getLastGameResult();
			if (result == 1) {
				it++;
			} else if (result == -1) {
				return;			// error
			}
			

		} else if (stage->image_filename != "") {
			///-- Display image --

			SpritePtr img = this->render->loadSprite("campaigns/" + stage->image_filename, c->mod);
			if (img) {
				// We don't want the image to wrap
				glBindTexture(GL_TEXTURE_2D, img->pixels);
				// TODO GLES: glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				// TODO GLES: glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

				// Determine position...
				float width = render->getSpriteWidth(img), height = render->getSpriteHeight(img);
				float ratImg = width/height, ratScr = (float)render->real_width/(float)render->real_height;

				// ...using ratios and fit-with-black-bars system
				if (ratImg > ratScr) {
					width = render->real_width;
					height = render->real_height * ratScr/ratImg;
				} else if (ratImg < ratScr) {
					width = render->real_width * ratImg/ratScr;
					height = render->real_height;
				} else {
					width = render->real_width;
					height = render->real_height;
				}

				// Render image
				render->renderSprite(img, (render->real_width - width) / 2.0f, (render->real_height - height) / 2.0f, width, height);

				// Wait
				unsigned int start = SDL_GetTicks();
				bool running = true;
				SDL_Event event;
				while (running) {
					if ((SDL_GetTicks() - start) > stage->image_time) {
						running = false;
					}

					while (SDL_PollEvent(&event)) {
						if (event.type == SDL_QUIT) {
							this->render->freeSprite(img);
							return;			// window close = campaign exit
						}
					}

					SDL_GL_SwapWindow(((RenderOpenGL*)render)->window);
				}

				this->render->freeSprite(img);
			}
			it++;
			
		} else {
			assert(0);
		}
	}
}


/**
* Starts a game with the specified settings
*
* @param int viewmode The camera view mode for the local player(s)
* @param int num_local The number of local players; 1 for single, 2+ for splitscreen
* @param int host Set to 1 to host a network game, 0 for a local-only game
**/
void Menu::startGame(MapReg *map, string gametype, string unittype, int viewmode, unsigned int num_local, bool host)
{
	st->physics->init();
	
	if (host) {
		if (st->sconf == NULL) {
			st->sconf = new ServerConfig();
		}
		
		new NetServer(st);
	}
	
	// Load map
	Map *m = new Map(st);
	m->load(map->getName(), st->render, map->getMod());
	st->map = m;
	
	// Load gametype
	new GameLogic(st);
	GameType *gt = st->mm->getGameType(gametype);
	st->logic->selected_unittype = st->mm->getUnitType(unittype);
	st->logic->execScript(gt->script);
	
	// Reset client variables
	st->client = NULL;
	st->num_local = num_local;
	
	// Create players in GameState.
	for (unsigned int i = 0; i < num_local; i++) {
		st->local_players[i] = new PlayerState(st);
		st->local_players[i]->slot = i + 1;
	}

	st->render->viewmode = viewmode;

	// Begin!
	gameLoop(st, st->render);
	
	// Post game, we need to restore menu GL settings
	this->setupGLstate();
	
	if (host) delete(st->server);
}

	
/**
* Join a network game
**/
void Menu::networkJoin(string host)
{
	if (st->client) delete (st->client);
	new NetClient(st);

	st->physics->init();

	// Create local player
	st->num_local = 1;
	st->local_players[0] = new PlayerState(st);
	
	// Try to join the server
	NetGameinfo *gameinfo = st->client->attemptJoinGame(host, 17778, this);
	if (gameinfo == NULL) {
		delete (st->client);
		st->client = NULL;
		displayMessageBox("Unable to connect to server " + host);
		return;
	}

	// Try to find the map
	MapReg *map = this->mapreg->get(gameinfo->map);
	if (! map) displayMessageBox("Map not found: " + map->getName());

	// Load map
	Map *m = new Map(st);
	m->load(map->getName(), st->render, map->getMod());
	st->map = m;
	
	// Load gametype
	new GameLogic(st);
	st->logic->selected_unittype = st->mm->getUnitType("robot");

	st->render->viewmode = 0;
	
	// Download the gamestate
	// When this is done, a final message is sent to tell the server we are done.
	bool gotstate = st->client->downloadGameState();
	if (! gotstate) {
		st->audio->postGame();
		st->postGame();
		delete (st->client);
		st->client = NULL;
		displayMessageBox("Unable to download intial game state from server " + host);
		return;
	}

	// Begin!
	gameLoop(st, st->render);
	
	// Quit
	delete (st->client);
	st->client = NULL;

	// Post game, we need to restore menu GL settings
	this->setupGLstate();
}
