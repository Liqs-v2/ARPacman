#include <Windows.h>
#include "Ghost.h"
#include <iostream>
extern GameManager manager;

void ghostAction(Ghost& ghost, const Pacman& pacman)
{
    ghost.waitForRelease();
    while (manager.gameStatus == STARTED)
    {
        ghost.direction = rand() % (ghost.max_ - ghost.min_ + 1) + ghost.min_;
        //alive, do as following
        if (!ghost.killed)
        {
            //pursue pacman
            if (!manager.powerModeStart)
            {
                //incase pacman is suddenly dead, don't waste time here,finish this thread
                if (pacman.killed || manager.powerModeStart || manager.gameStatus == END)
                    break;
                ghost.move(ghost.direction);
                std::cout << ghost.direction;
                Sleep(1000 / ghost.ghostSpeed);
            }
            //run away from pacman
            else if (manager.powerModeStart)
            {
                if (pacman.killed || !manager.powerModeStart || manager.gameStatus == END)
                    break;
                ghost.move(ghost.direction);
                Sleep(1000 / ghost.ghostSpeed);
            }

        }
        //killed, go back to the maze center
        else if (ghost.killed)
        {
            //wait for 5s to respawn:
            time_point start = std::chrono::steady_clock::now();
            time_point now = std::chrono::steady_clock::now();
            while (now - start < std::chrono::seconds(5))
            {
                if (manager.gameStatus == PACDIE || manager.gameStatus == END)
                {
                    ghost.ready4AnotherRound = true;
                    return;
                }
                now = std::chrono::steady_clock::now();
            }
            ghost.resume();
            ghost.waitForRelease();
        }
    }
    //when the code goes here,the pacman should be dead, 
    //the ghost ready to play another round, inform the control thread
    ghost.ready4AnotherRound = true;
    
}

//default constructor
Ghost::Ghost() : colorR(0.5), colorG(0.5), colorB(0.5)
{
}

//constructor with arguments
Ghost::Ghost(float R, float G, float B)
    : colorR(R), colorG(G), colorB(B),
    originalR(R), originalG(G), originalB(B)
{
    this->threadID = this->ghostThread.get_id();
    ghostSpeed = HIGH;
    this->spawn();
}

//set the Ghost's color
void Ghost::setColor(float R, float G, float B)
{
    colorR = R;
    colorG = G;
    colorB = B;
}

//move toward given direction
void Ghost::move(int dir)
{
    int xTemp, zTemp;
    //update coordinates 0-idle, 1-up, 2-down, 3-left, 4-right
    switch (dir)
    {
    case 0:
        break;
    case 1:
        //check walls. see if moving one step would crash onto a wall
        zTemp = round((z - 1 * len) / len) + 10;
        xTemp = round(x / len) + 9;
        //if the move is illegal, don't move
        if (map[zTemp][xTemp] == WALL)
        {
            return;
        }
        else
            z -= 1 * len;
        break;
    case 2:
        //check walls. see if moving one step would crash onto a wall
        zTemp = round((z + 1 * len) / len) + 10;
        xTemp = round(x / len) + 9;
        //if the move is illegal, don't move
        if (map[zTemp][xTemp] == WALL)
        {
            return;
        }
        z += 1 * len;
        break;
    case 3:
        //check walls. see if moving one step would crash onto a wall
        zTemp = round(z / len) + 10;
        xTemp = round((x - 1 * len) / len) + 9;
        //it's ok to pass special spots
        if (zTemp == 10 && xTemp == -1)
        {
            x = 9 * len;
            return;
        }
        //if the move is illegal, don't move
        if (map[zTemp][xTemp] == WALL)
        {
            return;
        }
        x -= 1 * len;
        break;
    case 4:
        //check walls. see if moving one step would crash onto a wall
        zTemp = round(z / len) + 10;
        xTemp = round((x + 1 * len) / len) + 9;
        //it's ok to pass special spots
        if (zTemp == 10 && xTemp == 19)
        {
            x = -9 * len;
            return;
        }
        //if the move is illegal, don't move
        if (map[zTemp][xTemp] == WALL)
        {
            return;
        }
        x += 1 * len;
        break;
    }
}

//when the game start, init ghost, name the pacman to chase
void Ghost::waitForRelease()
{
    while (manager.gameStatus != STARTED)
    {
    }
    //if the last ghost is released >2s ago, allow releasing this ghost
    time_point now = std::chrono::steady_clock::now();
    mtx.lock();
    while (now - lastRelease < releaseInterval)
    {
        //not allowed out ,wait for 0.5s and check again
        Sleep(500);
        now = std::chrono::steady_clock::now();
    }
    //if the code goes down here, this ghost is allowed out, reset lastRelease
    lastRelease = std::chrono::steady_clock::now();
    //the respawn seat is open now
    spotOccupied[this->spotNum] = false;
    mtx.unlock();
    this->z = -2 * len;
    this->x = 0;
}

//spawn
void Ghost::spawn()
{
    //find a spot to put a newly-spawn ghost, up to 4 spots
    mtx.lock();
    for (int i = 0; i < 4; ++i)
    {
        if (spotOccupied[i])
            continue;
        else
        {
            this->x = spawnSpot[i][0];
            this->z = spawnSpot[i][1];
            spotOccupied[i] = true;
            this->spotNum = i;
            break;
        }
    }
    mtx.unlock();
}

//resume after pacman dies or game ends
void Ghost::resume()
{
    if (manager.powerModeStart)
    {
        this->setColor(1.0f, 1.0f, 1.0f);
        ghostSpeed = LOW;
    }
    else
    {
        ghostSpeed = HIGH;
        this->setColor(originalR, originalG, originalB);
    }
    this->spawn();
    this->killed = false;
}


/*static members*/

//four respawn spots
float Ghost::spawnSpot[4][2] = { {0, -0.5 * len}, {0, 0.5 * len},{-1 * len, 0.5 * len},{1 * len, 0.5 * len} };
//flag indicating ghost spots' status
bool Ghost::spotOccupied[4] = { false, false,false,false };
//time at last release
time_point Ghost::lastRelease = std::chrono::steady_clock::now();
//release interval = 2s
std::chrono::seconds Ghost::releaseInterval(2);
//init the mutex object
std::mutex Ghost::mtx;