// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#include "effects_manager.h"
#include "SPK_TextureQuadRenderer.h"
#include "SPK_ColorQuadRenderer.h"
#include "../rage.h"
#include "../game_state.h"
#include "../game_engine.h"
#include "../spark/SPK.h"
#include "../render/render.h"
#include "../render_opengl/texture_2d_array.h"
#include "../mod/mod.h"
#include "../mod/mod_manager.h"


using namespace std;


// Gravity, the same for all particles
SPK::Vector3D gravity(0.0f, -0.9f, 0.0f);


EffectsManager::EffectsManager(GameState* st)
{
	this->st = st;
	this->setUpCoreEffects();
}


EffectsManager::~EffectsManager()
{
	delete points;
	delete render_blood;
}


/**
* Load the effects models
**/
void EffectsManager::setUpCoreEffects()
{
	points = new SPK::GL::GL2ColorQuadRenderer(0.25f);
	points->initGLbuffers();
	GEng()->render->addParticleRenderer(points);

	bullets_model = SPK::Model::create(
		SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_ALPHA,
		SPK::FLAG_ALPHA,
		SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE
	);
	bullets_model->setParam(SPK::PARAM_ALPHA, 1.0f, 0.0f);
	bullets_model->setParam(SPK::PARAM_RED, 0.0f, 0.1f);
	bullets_model->setParam(SPK::PARAM_GREEN, 0.0f, 0.1f);
	bullets_model->setParam(SPK::PARAM_BLUE, 0.0f, 0.1f);
	bullets_model->setLifeTime(0.5f, 0.7f);

	bullets_group = SPK::Group::create(bullets_model, 1024);
	bullets_group->setRenderer(points);
	st->addParticleGroup(bullets_group);


	SPK::Model* flames_model = SPK::Model::create(
		SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_ALPHA,
		SPK::FLAG_ALPHA,
		SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE
	);
	flames_model->setParam(SPK::PARAM_ALPHA, 0.5f, 0.0f);
	flames_model->setParam(SPK::PARAM_RED, 0.5f, 0.9f);
	flames_model->setParam(SPK::PARAM_GREEN, 0.2f, 0.3f);
	flames_model->setParam(SPK::PARAM_BLUE, 0.0f, 0.1f);
	flames_model->setLifeTime(0.5f, 0.7f);

	flames_group = SPK::Group::create(flames_model, 1024);
	flames_group->setRenderer(points);
	st->addParticleGroup(flames_group);


	tex_dust = new Texture2DArray();
	tex_dust->loadSingleImage(
		"textures/dust_atlas.png", GEng()->mm->getBase(), 4
	);

	render_dust = new SPK::GL::GL2InstanceQuadRenderer(1.0f);
	render_dust->setTexture(tex_dust);
	render_dust->initGLbuffers();
	GEng()->render->addParticleRenderer(render_dust);

	model_dust = SPK::Model::create(
			SPK::FLAG_ALPHA | SPK::FLAG_TEXTURE_INDEX,
			SPK::FLAG_ALPHA,
			SPK::FLAG_TEXTURE_INDEX
	);
	model_dust->setParam(SPK::PARAM_ALPHA, 0.3f, 0.0f);
	model_dust->setParam(SPK::PARAM_TEXTURE_INDEX, 0.0f, 3.0f);
	model_dust->setLifeTime(1.2f, 8.0f);


	//  Blood  //

	tex_blood = new Texture2DArray();
	tex_blood->loadSingleImage(
		"textures/blood_atlas.png", GEng()->mm->getBase(), 4
	);

	render_blood = new SPK::GL::GL2InstanceQuadRenderer(0.25f);
	render_blood->setTexture(tex_blood);
	render_blood->initGLbuffers();
	GEng()->render->addParticleRenderer(render_blood);

	model_blood = SPK::Model::create(
		SPK::FLAG_ALPHA | SPK::FLAG_TEXTURE_INDEX,
		SPK::FLAG_ALPHA,
		SPK::FLAG_ALPHA | SPK::FLAG_TEXTURE_INDEX
	);
	model_blood->setParam(SPK::PARAM_ALPHA, 0.8f, 1.0f,  0.0f, 0.2f);
	model_blood->setParam(SPK::PARAM_TEXTURE_INDEX, 0.0f, 3.0f);
	model_blood->setLifeTime(0.2f, 0.4f);


	//  Fireball (explosion)  //

	tex_fireball = new Texture2DArray();
	tex_fireball->loadSingleImage(
		"textures/explosion_atlas.png", GEng()->mm->getBase(), 9
	);

	render_fireball = new SPK::GL::GL2InstanceQuadRenderer(3.0f);
	render_fireball->setTexture(tex_fireball);
	render_fireball->initGLbuffers();
	GEng()->render->addParticleRenderer(render_fireball);
	
	model_fireball = SPK::Model::create(
		SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_ALPHA | SPK::FLAG_TEXTURE_INDEX,
		SPK::FLAG_ALPHA,
		SPK::FLAG_RED | SPK::FLAG_GREEN | SPK::FLAG_BLUE | SPK::FLAG_TEXTURE_INDEX
	);
	model_fireball->setParam(SPK::PARAM_ALPHA, 0.8f, 1.0f);
	model_fireball->setParam(SPK::PARAM_RED, 0.8f, 1.0f);
	model_fireball->setParam(SPK::PARAM_GREEN, 0.0f, 0.5f);
	model_fireball->setParam(SPK::PARAM_BLUE, 0.0f, 0.1f);
	model_fireball->setParam(SPK::PARAM_TEXTURE_INDEX, 0.0f, 8.0f);
	model_fireball->setLifeTime(0.2f, 0.3f);
}


/**
* Creates particles for a weapon.
* Angles and speeds are randomised. Lifetime is calculated based on the end vector and a constant speed.
*
* @param num The number of particles to create
* @param start The start location of the particle stream
* @param end The end location of the particle stream
* @param float speed How fast bullets should be moving, in metres/
**/
void EffectsManager::weaponBullets(btVector3* begin, btVector3* end, float speed)
{
	btVector3 velocity = *end - *begin;
	velocity.normalize();
	velocity *= speed;

	SPK::Vector3D spk_pos(begin->x(), begin->y(), begin->z());
	SPK::Vector3D spk_vel(velocity.x(), velocity.y(), velocity.z());

	bullets_group->addParticles(1, spk_pos, spk_vel);
}


/**
* Creates particles for a flamethrower.
* Angles and speeds are randomised. Lifetime is calculated based on the end vector and a constant speed.
*
* @param num The number of particles to create
* @param start The start location of the particle stream
* @param end The end location of the particle stream
**/
void EffectsManager::weaponFlamethrower(btVector3 * begin, btVector3 * end)
{
	btVector3 velocity = *end - *begin;
	velocity.normalize();
	velocity *= 45.0f;

	SPK::Vector3D spk_vel(velocity.x(), velocity.y(), velocity.z());

	SPK::Zone* spk_zone = SPK::Cylinder::create(
		SPK::Vector3D(begin->x(), begin->y(), begin->z()),
		spk_vel,
		0.5f,   // radius
		0.1f    // length
	);

	flames_group->addParticles(10, spk_zone, spk_vel, true);
}


/**
* Spray blood in all directions
**/
void EffectsManager::bloodSpray(const btVector3& location, float damage)
{
	SPK::RandomEmitter* emitter;
	SPK::Group* group;

	emitter = SPK::RandomEmitter::create();
	emitter->setZone(SPK::Point::create(SPK::Vector3D(location.x(), location.y() - 0.5f, location.z())));
	emitter->setFlow(-1);
	emitter->setTank(damage / 10.0f);
	emitter->setForce(5.0f, 10.0f);

	group = SPK::Group::create(model_blood, damage);
	group->addEmitter(emitter);
	group->setGravity(gravity);
	group->setRenderer(render_blood);
	st->addParticleGroup(group);
}


/**
* It's an EXPLOSION!
**/
void EffectsManager::explosion(const btVector3& location, float damage)
{
	SPK::Emitter* emitter;
	SPK::Group* group;

	// Emitter
	emitter = SPK::RandomEmitter::create();
	emitter->setZone(SPK::Point::create(SPK::Vector3D(location.x(), location.y(), location.z())));
	emitter->setFlow(-1);
	emitter->setTank(100);
	emitter->setForce(10.0f, 20.0f);

	// Create group
	group = SPK::Group::create(model_fireball, 100);
	group->addEmitter(emitter);
	group->setGravity(gravity);
	group->setRenderer(render_fireball);
	st->addParticleGroup(group);

	// Emitter
	emitter = SPK::NormalEmitter::create();
	emitter->setZone(SPK::Sphere::create(SPK::Vector3D(location.x(), location.y(), location.z()), 2.0f));
	emitter->setFlow(-1);
	emitter->setTank(50);
	emitter->setForce(3.0f, 5.0f);

	// Create group
	group = SPK::Group::create(model_dust, 50);
	group->addEmitter(emitter);
	group->setGravity(gravity);
	group->setFriction(1.0f);
	group->setRenderer(render_dust);
	st->addParticleGroup(group);
}
