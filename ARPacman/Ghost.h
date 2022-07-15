#pragma once
#include <vector>
#include <cmath>
#include <thread>
#include <mutex>
#include <chrono>
#include "GameManager.h"
#include "Pacman.h"

typedef std::chrono::steady_clock::time_point time_point;
typedef float speed;
const speed HIGH = 4;
const speed LOW = 1.5;
extern int gameMap[22][19];

class Ghost
{
public:
	Ghost();
	//construct and set color,set a pacman to chase
	Ghost(float R, float G, float B);
	void waitForRelease();
	void drawGhost();
	void setColor(float R, float G, float B);
	void spawn(); //return the spot number
	void resume();
	friend void ghostAction(Ghost& ghost, const Pacman& pacman);
	friend void gameManager(GameManager& manager);
	friend class GameManager;
	std::thread ghostThread;
	std::thread::id threadID;
	bool ready4AnotherRound;

private:
	//four respawn spot, each containing a (x,z) coordinates
	static float spawnSpot[4][2];
	//flag indicating ghost spots' status
	static bool spotOccupied[4];
	static time_point lastRelease;
	static std::chrono::seconds releaseInterval;
	static std::mutex mtx;
	float colorR, colorG, colorB;
	float originalR, originalG, originalB;
	float x, z; //coordinates on x-z plane (y pointing up)
	speed ghostSpeed; //speed is represented by m steps/sec.
	bool killed;
	void move(int dir);
	int spotNum;
	int direction;
	int const max_ = 4;
	int const min_ = 1;

};
