#include "pacman.h"
#include <GLFW/glfw3.h>
#include <math.h>

extern enum moveFlag pacMoveDir, oldPacMoveDir;

//constructor
Pacman::Pacman() 
{
    //setting initial speed and location
    x = 0.0f;
    y = 6 * len;
    speed = 5 * len; //per second
    killed = false;
    countEatenPellets = 0;
}

//pacman move up direction
void Pacman::moveUp()
{
    
    int yTemp = round((y - 1 * len) / len) + 10;
    int xTemp = round(x / len) + 9;
    if (gridTiles[yTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (gridTiles[yTemp][xTemp] == PELLET)
    {
        countEatenPellets++;
    }
    else if (gridTiles[yTemp][xTemp] == POWERPELLET)
    {
        countEatenPellets++;
    }
    
    y -= 1 * len;
    gridTiles[yTemp][xTemp] = EMPTY;
    oldPacMoveDir = pacMoveDir;
}

//pacman move down direction
void Pacman::moveDown()
{

    int yTemp = round((y + len) / len) + 10;
    int xTemp = round(x / len) + 9;
    if (gridTiles[yTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (gridTiles[yTemp][xTemp] == PELLET)
    {
        countEatenPellets++;
    }
    else if (gridTiles[yTemp][xTemp] == POWERPELLET)
    {
        countEatenPellets++;
    }

    y += 1 * len;
    gridTiles[yTemp][xTemp] = EMPTY;
    oldPacMoveDir = pacMoveDir;

}

//pacman move left direction
void Pacman::moveLeft()
{

    int yTemp = round(y / len) + 10;
    int xTemp = round((x - len) / len) + 9;
    if (gridTiles[yTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (gridTiles[yTemp][xTemp] == PELLET)
    {
        countEatenPellets++;
    }
    else if (gridTiles[yTemp][xTemp] == POWERPELLET)
    {
        countEatenPellets++;
    }

    x -= 1 * len;
    gridTiles[yTemp][xTemp] = EMPTY;
    oldPacMoveDir = pacMoveDir;

}

//rpacman move right direction
void Pacman::moveRight()
{

    int yTemp = round(y / len) + 10;
    int xTemp = round((x + len) / len) + 9;
    if (gridTiles[yTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (gridTiles[yTemp][xTemp] == PELLET)
    {
        countEatenPellets++;
    }
    else if (gridTiles[yTemp][xTemp] == POWERPELLET)
    {
        countEatenPellets++;
    }

    x += 1 * len;
    gridTiles[yTemp][xTemp] = EMPTY;
    oldPacMoveDir = pacMoveDir;

}

//respawn pacman after dead or end of game
void Pacman::respawn()
{

    //move back to starting point
    this->x = 0.0f;
    this->y = 6 * len;

}