#pragma once

#include "App/SimpleSprite.h"
#include <vector>

class Player : public CSimpleSprite
{
public:
	Player(const char* file, unsigned int row, unsigned int col) : CSimpleSprite(file, row, col) {}
	float GetHealth() { return health; };
	void SetGravity(float nGrav) { gravity = nGrav; };
	void Update(float deltaTime) override; //override default update function to account for player functionality
	float GetFuel() { return fuel; };
	void AddFuel() { fuel += 2000; }
	void DealDamage();
	void FinishedPlanet() { score += 2000; }
	int GetScore() { return score; }
private:
	int score = 0;
	float health = 5.0F;
	float momentum[2] = { 0.0f, 0.0f };
	float gravity = 0.0F;
	void changeMomentum(float newDirection[2]);
	void move();
	void normalizeVector(float vector[2]);
	float fuel = 20000.0f;
};