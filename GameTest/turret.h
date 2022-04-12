#pragma once
#include "App/SimpleSprite.h"

class Turret : public CSimpleSprite {
public:
	Turret(const char* file, unsigned int x, unsigned int y) : CSimpleSprite(file, x, y) {}
};