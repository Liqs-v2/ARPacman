#pragma once

#define len 0.5f 
#define EMPTY 0
#define WALL 1
#define PELLET 2
#define POWERPELLET 3
extern int gridTiles[31][28];

class Pacman
{
public:
	Pacman();
	void moveUp();
	void moveDown();
	void moveLeft();
	void moveRight();
	void respawn();
	void drawPacman();
	int countEatenPellets;
private:
	float x, y; 
	float speed; 
	bool killed;
};
