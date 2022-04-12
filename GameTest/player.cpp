#include "player.h"
#include "stdafx.h"
#include "App/app.h"


//override default update function to account for player functionality
void Player::Update(float deltaTime) {
	if (App::GetController().GetLeftThumbStickX() > 0.5f)
	{
		SetAngle(GetAngle() - 0.005f * deltaTime);
	}
	if (App::GetController().GetLeftThumbStickX() < -0.5f)
	{
		SetAngle(GetAngle() + 0.005f * deltaTime);
	}
	if (App::GetController().GetLeftThumbStickY() < -0.5f && fuel > 0)
	{
		SetSpeed(GLclampf(GetSpeed() + 0.2f) * 0.1f);
		float angle = GetAngle();
		float deltaX = sin(angle) * 0.001f * deltaTime;
		float deltaY = cos(angle) * 0.001f * deltaTime;
		float newDirection[2] = { -deltaX, deltaY };
		changeMomentum(newDirection); //Adds a momentum on the new vector
		fuel -= 0.1F * deltaTime;
	}
	move(); //Changes X and Y based on momentum
}

//Modifies the player's current direction vector based on new input force vector
void Player::changeMomentum(float newDirection[2]) {
	momentum[0] += newDirection[0];
	momentum[1] += newDirection[1];
	normalizeVector(momentum);
}

//Moves player in direction of momentum
void Player::move() {
	float x, y;
	GetPosition(x, y);
	SetPosition(x + momentum[0], y + momentum[1]);
	momentum[0] /= 1.005f;
	momentum[1] /= 1.005f;
	float gravityDir[2] = { 0.0f , gravity };
	changeMomentum(gravityDir);
}

//Normalize the input vector
void Player::normalizeVector(float vector[2]) {
	float magnitute = sqrt((vector[0] * vector[0] + vector[1] * vector[1]));
	if (magnitute >= 1) {
		vector[0] /= magnitute;
		vector[1] /= magnitute;
	}
}

//Reduces health and resets position
void Player::DealDamage() { 
	health--; 
	SetPosition(APP_VIRTUAL_WIDTH / 2, APP_VIRTUAL_HEIGHT - 100);
}