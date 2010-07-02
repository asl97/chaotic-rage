#include <iostream>
#include <SDL.h>
#include <math.h>
#include "rage.h"

using namespace std;


Particle::Particle(ParticleType *pt, GameState *st) : Entity(st)
{
	this->pt = pt;
	this->animation_start = st->anim_frame;
	
	this->speed = getRandom(pt->lin_speed.min, pt->lin_speed.max);
	this->lin_accel = getRandom(pt->lin_accel.min, pt->lin_accel.max);
	this->max_age = getRandom(pt->age.min, pt->age.max);
	this->unit_damage = getRandom(pt->unit_damage.min, pt->unit_damage.max);
	this->wall_damage = getRandom(pt->wall_damage.min, pt->wall_damage.max);
	
	this->angle = 0;
	this->age = 0;
}

Particle::~Particle()
{
}


void Particle::update(int delta)
{
	this->age += delta;
	if (this->age >= this->max_age) {
		this->del = true;
	}
	
	this->speed += ppsDelta(this->lin_accel, delta);
	
	if (this->speed <= 0) return;
	
	this->x = pointPlusAngleX(this->x, this->angle, ppsDelta(this->speed, delta));
	this->y = pointPlusAngleY(this->y, this->angle, ppsDelta(this->speed, delta));
	
	this->x += getRandom(-3, 3);
	this->y += getRandom(-3, 3);
	
	
	// Hit a wall?
	data_pixel pixel = this->st->map->getDataAt(this->x, this->y);
	
	AreaType *at = getAreaTypeByID(pixel.type);
	if (at->wall) {
		this->del = true;
		
		this->pt->doActions(this, HIT_WALL);
		
		this->st->map->setDataHP(this->x, this->y, pixel.hp - this->wall_damage);
	}
	
}

SpritePtr Particle::getSprite()
{
	int idx = 0;
	if (this->pt->directional) idx = round(this->angle / 45);
	
	int frame = this->st->anim_frame - this->animation_start;
	frame = frame % this->pt->num_frames;
	idx += frame;
	
	return this->pt->sprites.at(idx);
}


