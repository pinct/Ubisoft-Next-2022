//---------------------------------------------------------------------------------
// App.cpp
// Implementation of the calls that are exposed via the App namespace.
//---------------------------------------------------------------------------------
#include "stdafx.h"
//---------------------------------------------------------------------------------
#include <string>
#include "main.h"
#include "app.h"
#include "SimpleSound.h"
#include "SimpleController.h"
#include "SimpleSprite.h"

//---------------------------------------------------------------------------------
// Utils and externals for system info.

namespace App
{
	void DrawLine(float sx, float sy, float ex, float ey, float r, float g, float b)
	{
#if APP_USE_VIRTUAL_RES		
		APP_VIRTUAL_TO_NATIVE_COORDS(sx, sy);
		APP_VIRTUAL_TO_NATIVE_COORDS(ex, ey);
#endif
		glBegin(GL_LINES);
		glColor3f(r, g, b); // Yellow
		glVertex2f(sx, sy);
		glVertex2f(ex, ey);
		glEnd();
	}

	CSimpleSprite* CreateSprite(const char* fileName, int columns, int rows)
	{
		return new CSimpleSprite(fileName, columns, rows);
	}

	//Adds in bullet object
	Bullet* CreateBullet(const char* fileName, int columns, int rows)
	{
		return new Bullet(fileName, columns, rows);
	}

	//Adds in turret object
	Turret* CreateTurret(const char* fileName, int columns, int rows)
	{
		return new Turret(fileName, columns, rows);
	}

	//Adds in fuel object
	Fuel* CreateFuel(const char* fileName, int columns, int rows)
	{
		return new Fuel(fileName, columns, rows);
	}

	//Check if there is a collision between the two lines denoted by ((x1, y1), (x2, y2)) and ((x3, y3), (x4, y4))
	bool CheckLineIntersection(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
	{
		float slopeA = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));
		float slopeB = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));
		return 0 <= slopeA && slopeA <= 1 && 0 <= slopeB  && slopeB <= 1;
	}

	//Check collisions between two CSimpleSprite objects
	bool CheckCollision(CSimpleSprite* sprite1, CSimpleSprite* sprite2) {
		float x1, y1, x2, y2, w1, w2, h1, h2;
		sprite1->GetPosition(x1, y1);
		sprite2->GetPosition(x2, y2);
		w1 = sprite1->GetWidth() * sprite1->GetScale();
		w2 = sprite2->GetWidth() * sprite2->GetScale();
		h1 = sprite1->GetHeight() * sprite1->GetScale();
		h2 = sprite2->GetHeight() * sprite2->GetScale();
		x1 -= w1 / 2;
		x2 -= w2 / 2;
		y1 -= h1 / 2;
		y2 -= h2 / 2;
		return x1 + w1 >= x2 && x2 + w2 >= x1 && y1 + h1 >= y2 && y2 + h2 >= y1;
	}

	//Check if there is a collision between a sprite and the line denoted by ((x1, y1), (x2, y2))
	bool CheckCollision (CSimpleSprite* sprite1, float x1, float y1, float x2, float y2)
	{
		float sX, sY, sW, sH;
		sprite1->GetPosition(sX, sY);
		sX -= (sprite1->GetWidth() * sprite1->GetScale()) / 2;
		sY -= (sprite1->GetHeight() * sprite1->GetScale()) / 2;
		sW = sX + (sprite1->GetWidth() * sprite1->GetScale());
		sH = sY + (sprite1->GetHeight() * sprite1->GetScale());
		//Check if the line intersects any edge of the sprite
		bool s1 = CheckLineIntersection(x1, y1, x2, y2, sX, sY, sW, sY);
		bool s2 = CheckLineIntersection(x1, y1, x2, y2, sX, sY, sX, sH);
		bool s3 = CheckLineIntersection(x1, y1, x2, y2, sX, sH, sW, sH);
		bool s4 = CheckLineIntersection(x1, y1, x2, y2, sW, sY, sW, sH);
		return s1 || s2 || s3 || s4;
	}

	bool IsKeyPressed(int key)
	{
		return ((GetAsyncKeyState(key) & 0x8000) != 0);
	}

	void GetMousePos(float &x, float &y)
	{
		POINT mousePos;
		GetCursorPos(&mousePos);	// Get the mouse cursor 2D x,y position			
		ScreenToClient(MAIN_WINDOW_HANDLE, &mousePos);
		x = (float)mousePos.x;
		y = (float)mousePos.y;
		x = (x * (2.0f / WINDOW_WIDTH) - 1.0f);
		y = -(y * (2.0f / WINDOW_HEIGHT) - 1.0f);

#if APP_USE_VIRTUAL_RES		
		APP_NATIVE_TO_VIRTUAL_COORDS(x, y);
#endif
	}
	void PlaySound(const char *fileName, bool looping)
	{
		DWORD flags = (looping) ? DSBPLAY_LOOPING : 0;
		CSimpleSound::GetInstance().PlaySound(fileName, flags);
	}
	void StopSound(const char *fileName)
	{
		CSimpleSound::GetInstance().StopSound(fileName);
	}
	bool IsSoundPlaying(const char *fileName)
	{
		return CSimpleSound::GetInstance().IsPlaying(fileName);
	}
	// This prints a string to the screen
	void Print(float x, float y, const char *st, float r, float g, float b, void *font)
	{
#if APP_USE_VIRTUAL_RES		
		APP_VIRTUAL_TO_NATIVE_COORDS(x, y);
#endif		
		// Set location to start printing text
		glColor3f(r, g, b); // Yellow
		glRasterPos2f(x, y);
		int l = (int)strlen(st);
		for (int i = 0; i < l; i++)
		{
			glutBitmapCharacter(font, st[i]); // Print a character on the screen
		}
	}
	const CController &GetController( int pad )
	{
		return CSimpleControllers::GetInstance().GetController(pad);
	}
}