#pragma once
#include "App/SimpleSprite.h"

class Fuel : public CSimpleSprite {
public:
	Fuel(const char* file, unsigned int x, unsigned int y) : CSimpleSprite(file, x, y) {}
};