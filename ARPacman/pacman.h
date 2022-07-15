#pragma once

#define gameLength 0.5f 
#define EMPTYSPACE 0
#define WALL 1
#define PELLET 2
#define POWERPELLET 3
extern int gameMap[22][19];
class Ghost;
class GameManager;
class Pacman
{
public:
	Pacman();
	void drawPacman();
	void setColor(float R, float G, float B);
	void moveUp();
	void moveDown();
	void moveLeft();
	void moveRight();
	void respawn();
	friend void gameManager(GameManager& manager);
	friend class GameManager;
	int countEats;
	int totalEatsToWin;
	bool enableKey;
	bool killed;
private:
	float colorR, colorG, colorB; //color
	float x, z; //coordinates on x-z plane (y pointing up)
	float speed; //default is 5 steps / sec
};