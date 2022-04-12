#pragma once
#include "App/SimpleSprite.h"

class Bullet : public CSimpleSprite {
public:
	Bullet(const char* file, unsigned int x, unsigned int y) : CSimpleSprite(file, x, y) {}
	void Update(float frameIndex) override; //override default update function to account for bullet functionality
	void setSpeedX(float sX) { speedX = sX * 0.03F; }
	void setSpeedY(float sY) { speedY = sY * 0.03F; }
private:
	float speedX = 0.0F;
	float speedY = 0.0F;
};