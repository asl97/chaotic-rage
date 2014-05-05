// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#pragma once
#include <glm/glm.hpp>
#include <string>
#include "../rage.h"

using namespace std;


#define RENDER_FRAME_DATA -1
#define RENDER_FRAME_BG -2


class Zone;
class Render;
class Mod;
class Light;
class MapMesh;
class RangeF;
class btTriangleMesh;
class btRigidBody;


// TODO: Cleanup, improve and move to own file.
class Light {
	public:
		unsigned int type;		// 1, 2 or 3
		float x;
		float y;
		float z;
		float diffuse [4];
		float specular [4];
		
	public:
		Light(unsigned int type);
		~Light();
		
		void setDiffuse(short r, short g, short b, short a);
		void setSpecular(short r, short g, short b, short a);
};

// TODO: Cleanup, improve and move to own file.
class Heightmap {
	public:
		int sx;
		int sz;
		float scale;
		float* data;
		btRigidBody* ground;

	public:
		Heightmap() : sx(0), sz(0), scale(0.0f), data(NULL), ground(NULL) {};
		~Heightmap();

	public:
		bool loadIMG(Mod* mod, string filename);
		bool createRigidBody(float mapSX, float mapSZ);
};

// TODO: Cleanup, improve and move to own file.
class MapMesh {
	public:
		btTransform xform;
		btRigidBody *body;
		AssimpModel *model;
		AnimPlay *play;
};

class Map {
	friend class RenderSDL;
	friend class RenderOpenGL;
	
	private:
		vector<Zone*> zones;
		vector<Light*> lights;
		float ambient[3];
		vector<MapMesh*> meshes;
		Render * render;
		GameState * st;
		Mod * mod;
		string name;
		
	public:
		float width;
		float height;
		Heightmap* heightmap;		// TODO: Support multiple
		SpritePtr terrain;
		
		SpritePtr skybox;
		glm::vec3 skybox_size;

		float water_level;
		SpritePtr water;
		RangeF water_range;
		float water_speed;		// distance per ms

	public:
		Map(GameState * st);
		~Map();
		bool preGame();
		void postGame();

	public:
		int load(string name, Render *render, Mod* insideof);
		void loadDefaultEntities();
		
		void update(int delta);

		string getName() { return this->name; }
		
		Zone* getSpawnZone(Faction f);
		Zone* getPrisonZone(Faction f);
		Zone* getCollectZone(Faction f);
		Zone* getDestZone(Faction f);
		Zone* getNearbaseZone(Faction f);
		float getRandomX();
		float getRandomY();
		
		// todo: support for multiple heightmaps (in different areas)?
		void createHeightmapRaw();
		float heightmapGet(int X, int Z);
		float heightmapSet(int X, int Z, float val);
		float heightmapAdd(int X, int Z, float amt);
		float heightmapScaleX();
		float heightmapScaleY();
		float heightmapScaleZ();
		btRigidBody * createGroundBody();

	private:
		void fillTriangeMesh(btTriangleMesh* trimesh, AnimPlay *play, AssimpModel *am, AssimpNode *nd);
		btRigidBody* createBoundaryPlane(const btVector3 &axis, const btVector3 &loc);
};


/**
* A single "registered" map
**/
class MapReg {
	friend class MapRegistry;
	
	protected:
		string name;
		string title;
		Mod* mod;
		bool arcade;
		
	public:
		MapReg() : name(""), title(""), mod(NULL), arcade(true) {}
		MapReg(string name) : name(name), title(name), mod(NULL), arcade(true) {}
		MapReg(string name, string title) : name(name), title(title), mod(NULL), arcade(true) {}
		MapReg(string name, string title, Mod* mod) : name(name), title(title), mod(mod), arcade(true) {}
		MapReg(string name, string title, Mod* mod, bool arcade) : name(name), title(title), mod(mod), arcade(arcade) {}
		
	public:
		string getName() { return this->name; }
		string getTitle() { return this->title; }
		Mod* getMod() { return this->mod; }
		bool getArcade() { return this->arcade; }
};


/**
* The registry of maps
**/
class MapRegistry {
	public:
		vector<MapReg> maps;
		
	public:
		void find(string dir);
		void find(Mod* mod);
		MapReg* get(string name);

		string nameAt(int index) { return maps.at(index).name; }
		string titleAt(int index) { return maps.at(index).title; }
		MapReg* at(int index) { return &maps.at(index); }
		unsigned int size() { return maps.size(); }
};




