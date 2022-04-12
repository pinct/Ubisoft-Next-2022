//------------------------------------------------------------------------
// GameTest.cpp
//------------------------------------------------------------------------
#include "stdafx.h"
//------------------------------------------------------------------------
#include <windows.h> 
#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
//------------------------------------------------------------------------
#include "app\app.h"
//------------------------------------------------------------------------


//File Variables
const char *bulletFile = ".\\TestData\\bullet.bmp";
const char *turretFile = ".\\TestData\\turret.bmp";
const char* fuelFile = ".\\TestData\\fuel.bmp";
const char* beamFile = ".\\TestData\\beam.bmp";
const char* planetFile = ".\\TestData\\planet.bmp";

//Scene Management
int SCENE;
bool canLeave = false;

std::vector<CSimpleSprite*> planets; //Vector with all of the planets

Player *player; //Player's Sprite
float gravity = -0.005f; //Arbitrary gravity for levels
CSimpleSprite* beam; //Tractor beam sprite

std::vector<std::vector<std::vector<float>>> maps; //Vector containing all of the polygons for a given level

std::vector<Bullet*> bullets; //Vector of all player bullets in game
float bulletCooldown = 0.0f; //Time left before another bullet can be fired

std::vector<Turret*> turrets; //Vector of all turrets in level
std::vector<Bullet*> turretBullets; //Vector of all bullets fired from turrets
float turretBulletCooldown = 0.0f; //Time left until turrets can fire another bullet

std::vector<Fuel*> fuels; //Vector containing all fuel cells in level

//------------------------------------------------------------------------
// Method Declarations
//------------------------------------------------------------------------

//Map Creation
void CreateMap(); //Generates the level
void AddFuel(int i, std::vector<std::vector<float>> &map); //Adds fuel pod into map at given edge
void AddTurret(int i, std::vector<std::vector<float>> &map); //Adds turret into map at given edge
void RemoveIfOutOfBounds(std::vector<Bullet*> &bulletVector, float deltaTime); //Removed bullets if they leave the screen bounds
void CreateBullet(); //Adds a new player bullet to the game
void CreateTurretBullet(); //Adds a new turret bullet to the game

//Collisions
void DoCollisions(); //Check multiple collisions in the game
void CheckBulletCollisions(); //Checks if player bullets are colliding with both the map and turrets
void CheckTurretBulletCollisions(); //Checks if turret bullets are colliding with both the map and player
void CheckBeamCollisions(); //Checks if beam is colliding with fuel
void CheckPlanetCollisions(); //Checks if player is colliding with planets
void CheckPlayerLineCollisions(); //Checks if player is colliding with map

//Scene Management
void LoadMainScene(); //Loads the overview scene
void LoadMap(); //Loads individual planet scenes
void CheckIfOutOfBounds(); //Wraps player's position around if they leave the screen bounds
std::vector<std::vector<float>> CreatePolygon(const int points, int width, int height, int i, int j); //Generates a new polygon shape for the map
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Called before first update. Do any initial setup here.
//------------------------------------------------------------------------
void Init()
{
	srand(time(NULL)); //Gets new random seed
	player = new Player(".\\TestData\\ship.bmp", 1, 1); //Creates Player
	player->SetScale(2.0f);

	beam = new CSimpleSprite(beamFile, 1, 1); //Creates Beam
	beam->SetScale(8.0f);
	beam->SetPosition(APP_VIRTUAL_WIDTH + beam->GetWidth() * beam->GetScale(), 
		APP_VIRTUAL_HEIGHT + beam->GetHeight() * beam->GetScale()); //Place beam off the edge of the screen

	LoadMainScene(); //Loads overview Scene

	//map = CreatePolygon(10, 400, 400);

	/*const int end = rand() % 30;
	map[0][0] = rand() % APP_VIRTUAL_WIDTH / 2 + 200;
	map[0][1] = rand() % APP_VIRTUAL_HEIGHT / 2 + 100;
	map[1][0] = rand() % APP_VIRTUAL_WIDTH / 2 + 200;
	map[1][1] = rand() % APP_VIRTUAL_HEIGHT / 2 + 100;
	map[2][0] = rand() % APP_VIRTUAL_WIDTH / 2 + 200;
	map[2][1] = rand() % APP_VIRTUAL_HEIGHT / 2 + 100;
	for (int i = 3; i < end; i++) {
		map[i][0] = rand() % APP_VIRTUAL_WIDTH / 2 + 200;
		map[i][1] = rand() % APP_VIRTUAL_HEIGHT / 2 + 100;
	}*/
	//------------------------------------------------------------------------
}

//------------------------------------------------------------------------
// Update your simulation here. deltaTime is the elapsed time since the last update in ms.
// This will be called at no greater frequency than the value of APP_MAX_FRAME_RATE
//------------------------------------------------------------------------
void Update(float deltaTime)
{
	if (SCENE != 0) { //Allows us to skip some scenarios that wouldn't be possible on main scene
		turretBulletCooldown -= deltaTime;
		RemoveIfOutOfBounds(turretBullets, deltaTime);
		if (turrets.size() == 0) {
			canLeave = true;
		}
	}
	bulletCooldown -= deltaTime;
	
	RemoveIfOutOfBounds(bullets, deltaTime);

	if (player != nullptr) { //Checks that the player hasn't been destroyed
		player->Update(deltaTime); //Used the player's update to move

		//Checks if A is pressed, then fires a bullet
		if (App::GetController().CheckButton(XINPUT_GAMEPAD_A, false) && bulletCooldown <= 0.0F)
		{
			CreateBullet();
		}

		//Checks if B is pressed, then opens tractor beam...
		if (App::GetController().CheckButton(XINPUT_GAMEPAD_B, false)) {
			float x, y;
			player->GetPosition(x, y);
			beam->SetPosition(x, y);
			beam->SetAngle(player->GetAngle());
		}
		else {
			//...else it is placed back off screen
			beam->SetPosition(APP_VIRTUAL_WIDTH + beam->GetWidth() * beam->GetScale(), APP_VIRTUAL_HEIGHT + beam->GetHeight() * beam->GetScale());
			beam->SetAngle(0);
		}

		DoCollisions();
		CreateTurretBullet();
		CheckIfOutOfBounds();

		//Verify if player is ready to leave
		float x, y;
		player->GetPosition(x, y);
		if (canLeave && y > APP_VIRTUAL_HEIGHT - 50) {
			player->FinishedPlanet();
			LoadMainScene();
		}
	}
}

//------------------------------------------------------------------------
// Add your display calls here (DrawLine,Print, DrawSprite.) 
// See App.h 
//------------------------------------------------------------------------
void Render()
{
	//Drawing all of the sprites in each vector	
	for each (Bullet* bullet in bullets)
	{
		bullet->Draw();
	}
	if (SCENE == 0) {
		for each (CSimpleSprite * planet in planets)
		{
			planet->Draw();
		}
	}
	else {
		for each (Bullet * bullet in turretBullets)
		{
			bullet->Draw();
		}
		for each (Turret * turret in turrets)
		{
			turret->Draw();
		}
		for each (Fuel * fuel in fuels)
		{
			fuel->Draw();
		}
		//Drawing each map line, including from last point to first
		for (int i = 0; i < maps.size(); i++) {
			for (int j = 1; j < maps[i].size(); j++) {
				App::DrawLine(maps[i][j - 1][0], maps[i][j - 1][1], maps[i][j][0], maps[i][j][1], 0.0f, 1.0f, 0.0f);
			}
			App::DrawLine(maps[i][0][0], maps[i][0][1], maps[i][maps[i].size() - 1][0], maps[i][maps[i].size() - 1][1], 0.0f, 1.0f, 0.0f);
		}
	}
	beam->Draw();
	if (player != nullptr) {
		player->Draw();
		//Displaying UI
		App::Print(APP_VIRTUAL_WIDTH / 2 - APP_VIRTUAL_WIDTH / 5, APP_VIRTUAL_HEIGHT - APP_VIRTUAL_HEIGHT / 7, std::to_string((int)player->GetScore()).c_str());
		App::Print(APP_VIRTUAL_WIDTH / 2, APP_VIRTUAL_HEIGHT - APP_VIRTUAL_HEIGHT / 7, "SCORE");
		App::Print(APP_VIRTUAL_WIDTH / 2 - APP_VIRTUAL_WIDTH / 5, APP_VIRTUAL_HEIGHT - APP_VIRTUAL_HEIGHT / 5, std::to_string((int)player->GetFuel()).c_str()); App::Print(APP_VIRTUAL_WIDTH / 2 - APP_VIRTUAL_WIDTH / 5, APP_VIRTUAL_HEIGHT - APP_VIRTUAL_HEIGHT / 5, std::to_string(player->GetFuel()).c_str());
		App::Print(APP_VIRTUAL_WIDTH / 2, APP_VIRTUAL_HEIGHT - APP_VIRTUAL_HEIGHT / 5, "FUEL");
		App::Print(APP_VIRTUAL_WIDTH / 2 - APP_VIRTUAL_WIDTH / 5, APP_VIRTUAL_HEIGHT - APP_VIRTUAL_HEIGHT / 6, std::to_string((int)player->GetHealth()).c_str());
		App::Print(APP_VIRTUAL_WIDTH / 2, APP_VIRTUAL_HEIGHT - APP_VIRTUAL_HEIGHT / 6, "LIVES");
		if (SCENE != 0) {
			App::Print(APP_VIRTUAL_WIDTH / 2 - APP_VIRTUAL_WIDTH / 5, APP_VIRTUAL_HEIGHT - APP_VIRTUAL_HEIGHT / 4, std::to_string((int)turrets.size()).c_str());
			App::Print(APP_VIRTUAL_WIDTH / 2, APP_VIRTUAL_HEIGHT - APP_VIRTUAL_HEIGHT / 4, "Enemies Remaining");
		}
		//Checks if player is dead, then destroys it
		if (player->GetHealth() <= 0) {
			delete player;
			player = nullptr;
		}
	}
}
//------------------------------------------------------------------------
// Add your shutdown code here. Called when the APP_QUIT_KEY is pressed.
// Just before the app exits.
//------------------------------------------------------------------------
void Shutdown()
{	
	delete player;
	delete beam;
	turrets.clear();
	turrets.shrink_to_fit();
	bullets.clear();
	bullets.shrink_to_fit();
	turretBullets.clear();
	turretBullets.shrink_to_fit();
	planets.clear();
	planets.shrink_to_fit();
	maps.clear();
	maps.shrink_to_fit();
	fuels.clear();
	fuels.shrink_to_fit();
}

//------------------------------------------------------------------------
// Map Creation
//------------------------------------------------------------------------

//Creates polygons and places items in the map
void CreateMap()
{
	//Randomly get number of rows and columns
	int rows = ((rand() % 2)) * 2;
	int cols = ((rand() % 2)) * 2;
	for (int i = 0; i <= rows; i++) {
		for (int j = 0; j <= cols; j++) {
			if (rows == 0) {
				if (cols == 0) {
					maps.push_back(CreatePolygon(10, APP_VIRTUAL_WIDTH, APP_VIRTUAL_HEIGHT, i, j));
				}
				else {
					maps.push_back(CreatePolygon(10, APP_VIRTUAL_WIDTH, APP_VIRTUAL_HEIGHT / (cols + 1), i, j));
				}
			}
			else {
				if (cols == 0) {
					maps.push_back(CreatePolygon(10, APP_VIRTUAL_WIDTH / (rows + 1), APP_VIRTUAL_HEIGHT, i, j));
				}
				else {
					maps.push_back(CreatePolygon(10, APP_VIRTUAL_WIDTH / (rows + 1), APP_VIRTUAL_HEIGHT / (cols + 1), i, j));
				}
			}
		}
	}

	//Randomly place turrets and fuels, with at least 1 turret
	for (int i = 0; i < maps.size(); i++) {
		AddTurret(1, maps[i]);
		for (int j = 2; j < maps[i].size(); j++) {
			int chance = rand() % 2;
			if (chance == 0) {
				AddTurret(j, maps[i]);
			}
			if (chance == 1) {
				AddFuel(j, maps[i]);
			}
		}
	}
}

//Creates a polygon to add to the map
std::vector<std::vector<float>> CreatePolygon(const int points, int width, int height, int i, int j)
{
	//Adjust the polygon to be inside it's sector
	int xMult = width * (i);
	int yMult = height * (j);
	//Create a vector with "points" amount of points
	std::vector<std::vector<float>> coords;
	for (int a = 0; a < points; a++) {
		//Create a vector for the x and y positions
		std::vector<float> temp;
		temp.push_back(xMult + (rand() % (width - width / 5)));
		temp.push_back(yMult + (rand() % (height - height / 5)));
		coords.push_back(temp);
	}

	//Get the centre of the polygon
	float centre[2] = { (xMult + width) / 2, (yMult + height) / 2 };

	//Sort the points by angle with the centre point to prevent clipping
	std::sort(coords.begin(), coords.end(), [centre](const std::vector<float>& coords1, const std::vector<float>& coords2) {
		return atan2(coords1[1] - centre[1], coords1[0] - centre[0]) > atan2(coords2[1] - centre[1], coords2[0] - centre[0]);
		}
	);

	return coords;
}

//Adds a new turret into the game on edge i of map
void AddTurret(int i, std::vector<std::vector<float>>& map)
{
	//Check if surface is too short for a turret
	if (sqrt((map[i - 1][1] - map[i][1]) * (map[i - 1][1] - map[i][1]) + (map[i - 1][0] - map[i][0]) * (map[i - 1][0] - map[i][0])) <= 100) { return; }
	//Unfinished code to check the angle to make sure that the turret wouldn't be cropping through surfaces, WIP
	/*if (map[i] != map[map.size() - 1]) {
		float x1 = map[i - 1][0] - map[i][0];
		float x2 = map[i + 1][0] - map[i][0];
		float y1 = map[i - 1][1] - map[i][1];
		float y2 = map[i + 1][1] - map[i][1];
		float s1 = sqrt(x1 * x1 + y1 * y1);
		float s2 = sqrt(x2 * x2 + y2 * y2);
		float theta = abs(atan(s2) - atan(s1));
		if (theta < PI/2) { return; }
	}
	else if (map[i] == map[map.size() - 1]){
		if (-PI/2 <= atan2(map[i - 1][1] - map[i][1], map[i - 1][0] - map[i][0]) - atan2(map[0][1] - map[i][1], map[0][0] - map[i][0]) <= PI / 2) { return; }
	}*/
	Turret* tempTurret = App::CreateTurret(turretFile, 1, 1); //Create new Turret
	float newX = (map[i - 1][0] - map[i][0]) / 2 + map[i][0]; //Get x halfway on line
	float newY = (map[i - 1][1] - map[i][1]) / 2 + map[i][1]; //Get y halfway on line
	float angle = atan(((map[i - 1][1] - map[i][1]) / 2) / ((map[i - 1][0] - map[i][0]) / 2)); //Get angle of line
	float angle2 = (PI / 2) - atan((map[i - 1][0] - map[i][0]) / (map[i - 1][1] - map[i][1])); //Get angle needed to translate perpendicular to the line
	//Following adjusts x and y depending on location of the turret so the bottom edge of the turret is on the line
	if (map[i - 1][0] > map[i][0]) {
		angle += PI;
		if (map[i - 1][1] > map[i][1]) {
			newY -= cos(angle2) * (tempTurret->GetHeight() * tempTurret->GetScale());
			newX += sin(angle2) * (tempTurret->GetHeight() * tempTurret->GetScale());
		}
		else {
			newY += cos(angle2) * (tempTurret->GetHeight() * tempTurret->GetScale());
			newX -= sin(angle2) * (tempTurret->GetHeight() * tempTurret->GetScale());
		}
	}
	else {
		if (map[i - 1][1] > map[i][1]) {
			newX += sin(angle2) * (tempTurret->GetHeight() * tempTurret->GetScale());
			newY -= cos(angle2) * (tempTurret->GetHeight() * tempTurret->GetScale());
		}
		else {
			newX -= sin(angle2) * (tempTurret->GetHeight() * tempTurret->GetScale());
			newY += cos(angle2) * (tempTurret->GetHeight() * tempTurret->GetScale());
		}
	}
	//Set values then add to turret vector
	tempTurret->SetAngle(angle);
	tempTurret->SetPosition(newX, newY);
	tempTurret->SetScale(5.0f);
	turrets.push_back(tempTurret);
}

//Adds a new fuel cell into the game on edge i of map
void AddFuel(int i, std::vector<std::vector<float>>& map)
{
	//Check if surface is too short for a fuel cell
	if (sqrt((map[i - 1][1] - map[i][1]) * (map[i - 1][1] - map[i][1]) + (map[i - 1][0] - map[i][0]) * (map[i - 1][0] - map[i][0])) <= 100) { return; }
	Fuel* tempFuel = App::CreateFuel(fuelFile, 1, 1); //Create new fuel cell
	float newX = (map[i - 1][0] - map[i][0]) / 2 + map[i][0]; //Get x halfway on line
	float newY = (map[i - 1][1] - map[i][1]) / 2 + map[i][1]; //Get y halfway on line
	float angle = atan(((map[i - 1][1] - map[i][1]) / 2) / ((map[i - 1][0] - map[i][0]) / 2)); //Get angle of line
	float angle2 = (PI / 2) - atan((map[i - 1][0] - map[i][0]) / (map[i - 1][1] - map[i][1])); //Get angle needed to translate perpendicular to the line
	//Following adjusts x and y depending on location of the fuel cell so the top edge of the fuel cell is on the line
	if (map[i - 1][0] > map[i][0]) {
		angle += PI;
		if (map[i - 1][1] > map[i][1]) {
			newY += cos(angle2) * (tempFuel->GetHeight() * tempFuel->GetScale() / 2);
			newX -= sin(angle2) * (tempFuel->GetHeight() * tempFuel->GetScale() / 2);
		}
		else {
			newY -= cos(angle2) * (tempFuel->GetHeight() * tempFuel->GetScale() / 2);
			newX += sin(angle2) * (tempFuel->GetHeight() * tempFuel->GetScale() / 2);
		}
	}
	else {
		if (map[i - 1][1] > map[i][1]) {
			newX -= sin(angle2) * (tempFuel->GetHeight() * tempFuel->GetScale() / 2);
			newY += cos(angle2) * (tempFuel->GetHeight() * tempFuel->GetScale() / 2);
		}
		else {
			newX += sin(angle2) * (tempFuel->GetHeight() * tempFuel->GetScale() / 2);
			newY -= cos(angle2) * (tempFuel->GetHeight() * tempFuel->GetScale() / 2);
		}
	}
	//Set values then add to fuel vector
	tempFuel->SetAngle(angle);
	tempFuel->SetPosition(newX, newY);
	tempFuel->SetScale(2.0f);
	fuels.push_back(tempFuel);
}

//Removed the bullet if it exits the screen bounds
void RemoveIfOutOfBounds(std::vector<Bullet*>& bulletVector, float deltaTime) {
	//Iterate through bulletVector
	int counter = 0;
	for (std::vector<Bullet*>::iterator bullet = bulletVector.begin(); bullet != bulletVector.end();) {
		//Get bullet position
		float x, y;
		bulletVector[counter]->GetPosition(x, y);
		//If outside screen bounds, erase
		if (x > APP_VIRTUAL_WIDTH || x < 0 || y > APP_VIRTUAL_HEIGHT || y < 0) {
			bulletVector.erase(bullet);
		}
		//else update the bullet's position and continue iterating
		else {
			bulletVector[counter]->Update(deltaTime);
			counter++;
			bullet++;
		}
	}
}

//Creates a new player bullet
void CreateBullet()
{
	//Setting up variables
	float x, y;
	player->GetPosition(x, y);
	float angle = player->GetAngle();
	float deltaX = sin(angle) * 5.0f; //Get direction bullet should move on x
	float deltaY = cos(angle) * 5.0f; //Get direction bullet should move on y
	//Assign values
	Bullet* bullet = App::CreateBullet(bulletFile, 1, 1);
	bullet->SetPosition(x, y);
	bullet->SetAngle(angle);
	bullet->setSpeedX(deltaX);
	bullet->setSpeedY(deltaY);
	bullet->SetScale(1.0F);
	//Add bullet to vector and reset cooldown timer
	bullets.push_back(bullet);
	bulletCooldown = 500.0F;
}

//Create a turret-fired bullet
void CreateTurretBullet()
{
	//Check that we are ready for another turret bullet
	if (turretBulletCooldown <= 0.0F) {
		//Iterate through turrets
		for each (Turret * turret in turrets)
		{
			//Set up initial variables
			float x, y;
			turret->GetPosition(x, y);
			float pX, pY;
			player->GetPosition(pX, pY);
			float deltaX = x - pX; //Change the bullet's x direction to move towards the player's x
			float deltaY = pY - y; //Change the bullet's y direction to move towards the player's y
			//Create bullet and assign values
			Bullet* bullet = App::CreateBullet(bulletFile, 1, 1);
			bullet->SetPosition(x, y);
			float magnitute = sqrt(deltaX * deltaX + deltaY * deltaY); //Even out the magnitude so that the bullets move the same speed no matter the player distance
			bullet->setSpeedX((deltaX / magnitute) * 3.0F); //Normalize the vector
			bullet->setSpeedY((deltaY / magnitute) * 3.0F); //Normalize the vector
			bullet->SetAngle(atan(deltaX / deltaY));
			bullet->SetScale(1.0F);
			//Add to turret bullet vector
			turretBullets.push_back(bullet);
		}
		turretBulletCooldown = 1000.0F;
	}
}

//------------------------------------------------------------------------
// Collisions
//------------------------------------------------------------------------

//Check several collisions depending on scene
void DoCollisions()
{
	if (SCENE == 0) {
		CheckPlanetCollisions();
	}
	else {
		CheckTurretBulletCollisions();
		CheckBeamCollisions();
		CheckBulletCollisions();
		CheckPlayerLineCollisions();
	}
}

//Check if bullet is colliding with turret or map
void CheckBulletCollisions()
{
	//Iterate through bullet vector
	for (std::vector<Bullet*>::iterator bullet = bullets.begin(); bullet != bullets.end();) {
		bool increment = true;
		bool bExists = true;
		//Check if colliding with any of the turrets, then erase both if true
		for (std::vector<Turret*>::iterator turret = turrets.begin(); turret != turrets.end();) {
			if (App::CheckCollision(*bullet, *turret)) {
				turrets.erase(turret);
				bullets.erase(bullet);
				bExists = false;
				increment = false;
				break;
			}
			else {
				turret++;
			}
		}
		//Check if colliding with the map, then remove bullet if it is
		//NOTE: can slightly reduce complexity by combining map iteration here with player collision check, but that would also 
		//break the single responsibility principle
		for (int i = 0; i < maps.size(); i++) {
			for (int j = 1; j < maps[i].size(); j++) {
				if (bExists && App::CheckCollision(*bullet, maps[i][j][0], maps[i][j][1], maps[i][j - 1][0], maps[i][j - 1][1])) {
					bullets.erase(bullet);
					increment = false;
					bExists = false;
					break;
				}
			}
			if (bExists && App::CheckCollision(*bullet, maps[i][0][0], maps[i][0][1], maps[i][maps[i].size() - 1][0], maps[i][maps[i].size() - 1][1])) {
				bullets.erase(bullet);
				increment = false;
				bExists = false;
				break;
			}
		}
		if (increment) { bullet++; }
	}
}

//Check if bullets from the turret collide with map or player
void CheckTurretBulletCollisions() {
	//Iterate through turret bullets
	for (std::vector<Bullet*>::iterator bullet = turretBullets.begin()++; bullet != turretBullets.end();) {
		bool increment = true;
		//Check if it hits a player, then reset the player and remove the bullet
		if (App::CheckCollision(*bullet, player)) {
			turretBullets.erase(bullet);
			player->DealDamage();
			increment = false;
			break;
		}
		//Check if it hits the map, then remove the bullet
		for (int i = 0; i < maps.size(); i++) {
			for (int j = 1; j < maps[i].size(); j++) {
				if (App::CheckCollision(*bullet, maps[i][j][0], maps[i][j][1], maps[i][j - 1][0], maps[i][j - 1][1])) {
					turretBullets.erase(bullet);
					increment = false;
					break;
				}
			}
			if (App::CheckCollision(*bullet, maps[i][0][0], maps[i][0][1], maps[i][maps[i].size() - 1][0], maps[i][maps[i].size() - 1][1])) {
				turretBullets.erase(bullet);
				increment = false;
				break;
			}
		}
		if (increment) { bullet++; }
	}
}

//Check if beam is interacting with a fuel cell
void CheckBeamCollisions()
{
	//Iterate through fuel cells
	for (std::vector<Fuel*>::iterator fuel = fuels.begin(); fuel != fuels.end();) {
		bool increment = true;
		//Check if it is colliding with beam
		if (App::CheckCollision(*fuel, beam)) {
			fuels.erase(fuel);
			//Increase player's fuel
			player->AddFuel();
			increment = false;
		}
		if (increment) { fuel++; }
	}
}

//Check if player collided with planet
void CheckPlanetCollisions()
{
	//Iterate through planets
	for (std::vector<CSimpleSprite*>::iterator planet = planets.begin()++; planet != planets.end();) {
		bool increment = true;
		//Check if planet collided with player
		if (App::CheckCollision(*planet, player)) {
			planets.erase(planet);
			//Load a new map if they collided
			LoadMap();
			increment = false;
			break;
		}
		if (increment) { planet++; }
	}
}

//Check if player collided with map
void CheckPlayerLineCollisions() {
	//Iterate through all lines in all maps
	for (int i = 0; i < maps.size(); i++) {
		for (int j = 1; j < maps[i].size(); j++) {
			//Check for collision
			if (App::CheckCollision(player, maps[i][j][0], maps[i][j][1], maps[i][j - 1][0], maps[i][j - 1][1])) {
				//Resets player if collided
				player->DealDamage();
				break;
			}
		}
		if (App::CheckCollision(player, maps[i][0][0], maps[i][0][1], maps[i][maps[i].size() - 1][0], maps[i][maps[i].size() - 1][1])) {
			player->DealDamage();
			break;
		}
	}
}

//------------------------------------------------------------------------
// Scene Management
//------------------------------------------------------------------------

//Generate a new map and place the player in it
void LoadMap()
{
	SCENE = 1;
	player->SetGravity(gravity);
	player->SetPosition(APP_VIRTUAL_WIDTH / 2, APP_VIRTUAL_HEIGHT - 100);
	CreateMap();
}

//Load overview scene
void LoadMainScene()
{
	SCENE = 0;
	//Clear out map vector
	maps.clear();
	player->SetGravity(0.0f);
	player->SetPosition(APP_VIRTUAL_WIDTH / 2, APP_VIRTUAL_HEIGHT / 2);
	//If planets is empty, generate new amount of planets
	if (planets.size() == 0) {
		int numOfPlanets = rand() % 4 + 3;
		for (int i = 0; i <= numOfPlanets; i++) {
			CSimpleSprite* planet = new CSimpleSprite(planetFile, 1, 1);
			int left = rand() % 2;
			int up = rand() % 2;
			planet->SetPosition((rand() % ((APP_VIRTUAL_WIDTH / 2) - 100)) + left * (APP_VIRTUAL_WIDTH / 2), (rand() % ((APP_VIRTUAL_HEIGHT / 2) - 100)) + up * (APP_VIRTUAL_HEIGHT / 2));
			planet->SetScale(5.0f);
			planets.push_back(planet);
		}
	}
	canLeave = false;
}

//Loops player around if they are outside of the screen bounds
void CheckIfOutOfBounds() {
	//Get player x and y position
	float x, y;
	player->GetPosition(x, y);
	//Sets position to the oposite side if they reach the edge
	if (x > APP_VIRTUAL_WIDTH) {
		x = 0;
	}
	else if (x < 0) {
		x = APP_VIRTUAL_WIDTH;
	}
	if (y > APP_VIRTUAL_HEIGHT) {
		y = 0;
	}
	else if (y < 0) {
		y = APP_VIRTUAL_HEIGHT;
	}
	player->SetPosition(x, y);
}