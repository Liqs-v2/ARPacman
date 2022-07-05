#pragma once
#include <vector>
#include <cmath>
#include <thread>
#include <mutex>
#include <chrono>
#include "Pacman.h"
#include "Ghost.h"
#define ArrayX(x) round(x / gameLength) + 9
#define ArrayZ(z) round(z / gameLength) + 10
const int STARTED = 1;
const int PAUSED = 2;
const int END = 3;
const int PACDIE = 4;
typedef std::chrono::steady_clock::time_point time_point;
class GameManager
{
public:
    GameManager();
    bool powerUpEaten, powerModeStart;
    int gameStatus;
    int pacDeaths;
    std::thread controlThread;
    time_point t_powerModeStart;
    const std::chrono::seconds powerModeDuration;
    int backUpMap[22][19] =
    {
                        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                        {1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1},
                        {1,3,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,3,1},
                        {1,2,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,2,1},
                        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
                        {1,2,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,2,1},
                        {1,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,1},
                        {1,1,1,1,2,1,1,1,0,1,0,1,1,1,2,1,1,1,1},
                        {0,0,0,1,2,1,0,0,0,0,0,0,0,1,2,1,0,0,0},
                        {1,1,1,1,2,1,0,1,1,1,1,1,0,1,2,1,1,1,1}, //line 10
                        {0,0,0,0,2,0,0,1,0,0,0,1,0,0,2,0,0,0,0}, //line 11
                        {1,1,1,1,2,1,0,1,1,1,1,1,0,1,2,1,1,1,1},
                        {0,0,0,1,2,1,0,0,0,0,0,0,0,1,2,1,0,0,0},
                        {1,1,1,1,2,1,0,1,1,1,1,1,0,1,2,1,1,1,1},
                        {1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1},
                        {1,2,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,2,1},
                        {1,3,2,1,2,2,2,2,2,0,2,2,2,2,2,1,2,3,1},
                        {1,1,2,1,2,1,2,1,1,1,1,1,2,1,2,1,2,1,1},
                        {1,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,1},
                        {1,2,1,1,1,1,1,1,2,1,2,1,1,1,1,1,1,2,1},
                        {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
                        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
                        //
    };
private:
    friend void gameManager(GameManager& manager);
    void triggerPowerMode();
    void inPowerMode();
    void quitPowerMode();
    void inNormalMode();
    void endGame();
    void startAnotherRound();
};


void gameManager(GameManager& manager);