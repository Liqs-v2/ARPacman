#include <Windows.h>
#include "Pacman.h"
#include "GameManager.h"
#include <cmath>
extern enum moveFlag pacMoveDir, oldPacMoveDir;
extern GameManager manager;

//constructor
Pacman::Pacman() : colorR(1.0f), colorG(1.0f), colorB(1.0f)
{
    //setting initial speed and location
    x = 0.0f;
    z = 6 * len;
    speed = 5 * len; //per second
    killed = false;
    countEats = 0;
    totalEatsToWin = 156;
}


//pacman respawn after killed or game ends
void Pacman::respawn()
{
    //to show pacman is dead, pause the animation for 1s
    Sleep(1000);
    //back to starting point
    this->x = 0.0f;
    this->z = 6 * len;
}
//set color
void Pacman::setColor(float R, float G, float B)
{
    colorR = R;
    colorG = G;
    colorB = B;
}

//pacman moves one step up
void Pacman::moveUp()
{
    //check walls. see if moving up one step would crash onto a wall
    int zTemp = round((z - 1 * len) / len) + 10;
    int xTemp = round(x / len) + 9;
    //if the intended move is illegal, continue move at the old direction
    if (map[zTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (map[zTemp][xTemp] == POWERPELLET)
    {
        manager.powerUpEaten = true;
        countEats++;
    }
    else if (map[zTemp][xTemp] == PELLET)
    {
        countEats++;
    }
    //if the move is legal, update coordinates
    z -= 1 * len;
    //set wherever the pacman went to "EMPTY"
    map[zTemp][xTemp] = EMPTY;
    //direction changed
    oldPacMoveDir = pacMoveDir;
    //in the displayfunc(),call drawPacman() to move the pacman image up
}
void Pacman::moveDown()
{
    //check walls
    int zTemp = round((z + len) / len) + 10;
    int xTemp = round(x / len) + 9;
    //if the intended move is illegal, continue move at the old direction
    if (map[zTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (map[zTemp][xTemp] == POWERPELLET)
    {
        manager.powerUpEaten = true;
        countEats++;
    }
    else if (map[zTemp][xTemp] == PELLET)
    {
        countEats++;
    }
    //update coordinates
    z += 1 * len;
    //set wherever the pacman went to "EMPTY"
    map[zTemp][xTemp] = EMPTY;
    //direction changed
    oldPacMoveDir = pacMoveDir;
    //in the displayfunc(),call drawPacman() to move the pacman image up
}
void Pacman::moveLeft()
{
    //check walls
    int zTemp = round(z / len) + 10;
    int xTemp = round((x - len) / len) + 9;
    //it's ok to pass special spots
    if (zTemp == 10 && xTemp == -1)
    {
        x = 9 * len;
        return;
    }
    //if the intended move is illegal, continue move at the old direction
    if (map[zTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (map[zTemp][xTemp] == POWERPELLET)
    {
        manager.powerUpEaten = true;
        countEats++;
    }
    else if (map[zTemp][xTemp] == PELLET)
    {
        countEats++;
    }
    //update coordinates
    x -= 1 * len;
    //set wherever the pacman went to "EMPTY"
    map[zTemp][xTemp] = EMPTY;
    //direction changed
    oldPacMoveDir = pacMoveDir;
    //in the displayfunc(),call drawPacman() to move the pacman image up
}
void Pacman::moveRight()
{
    //check walls
    int zTemp = round(z / len) + 10;
    int xTemp = round((x + len) / len) + 9;
    //it's ok to pass special spots
    if (zTemp == 10 && xTemp == 19)
    {
        x = -9 * len;
        return;
    }
    //if the intended move is illegal, continue move at the old direction
    if (map[zTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (map[zTemp][xTemp] == POWERPELLET)
    {
        manager.powerUpEaten = true;
        countEats++;
    }
    else if (map[zTemp][xTemp] == PELLET)
    {
        countEats++;
    }
    //update coordinates
    x += 1 * len;
    //set wherever the pacman went to "EMPTY"
    map[zTemp][xTemp] = EMPTY;
    //direction changed
    oldPacMoveDir = pacMoveDir;
}