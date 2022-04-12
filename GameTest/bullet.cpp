#include "stdafx.h"
#include "bullet.h"


//override default update function to account for bullet functionality
void Bullet::Update(float deltaTime) {
	float x, y;
	GetPosition(x, y);
	SetPosition(x - speedX * deltaTime, y + speedY * deltaTime);
}