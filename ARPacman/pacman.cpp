#include <Windows.h>
#include "Pacman.h"
#include "GameManager.h"
#include "Feature.h"
#include <cmath>
extern enum moveFlag pacMoveDir, oldPacMoveDir;
extern GameManager manager;
extern Feature feature;

//constructor
Pacman::Pacman() : colorR(1.0f), colorG(1.0f), colorB(1.0f)
{
    //setting initial speed and location
    x = 0.0f;
    z = 6 * gameLength;
    speed = 0.1 * gameLength; //per second
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
    this->z = 6 * gameLength;
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
    int zTemp = round((z - 1 * gameLength) / gameLength) + 10;
    int xTemp = round(x / gameLength) + 9;
    //if the intended move is illegal, continue move at the old direction
    if (gameMap[zTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (gameMap[zTemp][xTemp] == POWERPELLET)
    {
        manager.powerUpEaten = true;
        countEats++;
        if (feature.enableSound) {
            PlaySound(L"pacman_eatfruit.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
    }
    else if (gameMap[zTemp][xTemp] == PELLET)
    {
        countEats++;
        if (feature.enableSound) {
            PlaySound(L"pacman_eatfruit.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
    }
    Sleep(20 / speed);
    //if the move is legal, update coordinates
    z -= 1 * gameLength;
    //set wherever the pacman went to "EMPTYSPACE"
    gameMap[zTemp][xTemp] = EMPTYSPACE;
    //direction changed
    oldPacMoveDir = pacMoveDir;
}
void Pacman::moveDown()
{
    //check walls
    int zTemp = round((z + gameLength) / gameLength) + 10;
    int xTemp = round(x / gameLength) + 9;
    //if the intended move is illegal, continue move at the old direction
    if (gameMap[zTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (gameMap[zTemp][xTemp] == POWERPELLET)
    {
        manager.powerUpEaten = true;
        countEats++;
        if (feature.enableSound) {
            PlaySound(L"pacman_eatfruit.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
    }
    else if (gameMap[zTemp][xTemp] == PELLET)
    {
        countEats++;
        if (feature.enableSound) {
            PlaySound(L"pacman_eatfruit.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
    }
    Sleep(20 / speed);
    //update coordinates
    z += 1 * gameLength;
    //set wherever the pacman went to "EMPTYSPACE"
    gameMap[zTemp][xTemp] = EMPTYSPACE;
    //direction changed
    oldPacMoveDir = pacMoveDir;
}
void Pacman::moveLeft()
{
    //check walls
    int zTemp = round(z / gameLength) + 10;
    int xTemp = round((x - gameLength) / gameLength) + 9;
    //it's ok to pass special spots
    if (zTemp == 10 && xTemp == -1)
    {
        x = 9 * gameLength;
        return;
    }
    //if the intended move is illegal, continue move at the old direction
    if (gameMap[zTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (gameMap[zTemp][xTemp] == POWERPELLET)
    {
        manager.powerUpEaten = true;
        countEats++;
        if (feature.enableSound) {
            PlaySound(L"pacman_eatfruit.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
    }
    else if (gameMap[zTemp][xTemp] == PELLET)
    {
        countEats++;
        if (feature.enableSound) {
            PlaySound(L"pacman_eatfruit.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
    }
    Sleep(20 / speed);
    //update coordinates
    x -= 1 * gameLength;
    //set wherever the pacman went to "EMPTYSPACE"
    gameMap[zTemp][xTemp] = EMPTYSPACE;
    //direction changed
    oldPacMoveDir = pacMoveDir;
}
void Pacman::moveRight()
{
    //check walls
    int zTemp = round(z / gameLength) + 10;
    int xTemp = round((x + gameLength) / gameLength) + 9;
    //it's ok to pass special spots
    if (zTemp == 10 && xTemp == 19)
    {
        x = -9 * gameLength;
        return;
    }
    //if the intended move is illegal, continue move at the old direction
    if (gameMap[zTemp][xTemp] == WALL)
    {
        pacMoveDir = oldPacMoveDir;
        return;
    }
    else if (gameMap[zTemp][xTemp] == POWERPELLET)
    {
        manager.powerUpEaten = true;
        countEats++;
        if (feature.enableSound) {
            PlaySound(L"pacman_eatfruit.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
    }
    else if (gameMap[zTemp][xTemp] == PELLET)
    {
        countEats++;
        if (feature.enableSound) {
            PlaySound(L"pacman_eatfruit.wav", NULL, SND_FILENAME | SND_ASYNC);
        }
        
    }
    Sleep(20 / speed);
    //update coordinates
    x += 1 * gameLength;
    //set wherever the pacman went to "EMPTYSPACE"
    gameMap[zTemp][xTemp] = EMPTYSPACE;
    //direction changed
    oldPacMoveDir = pacMoveDir;
}