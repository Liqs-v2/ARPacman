#include <Windows.h>
#include "GameManager.h"
#include <iostream>

extern Pacman pacmanAgent;
extern int map[22][19];
GameManager manager;


//game manager thread func
void gameManager(GameManager& manager)
{
    while (1)
    {
        //wait for the game to start
        while (manager.gameStatus != STARTED)
        {
            Sleep(200);
        }
        //check if we should enter power-mode
        if (!manager.powerModeStart && manager.powerUpEaten)
        {
            manager.triggerPowerMode();
        }
        //if already in power-up mode (somewhere between the 5s)
        if (manager.powerModeStart)
        {
            manager.inPowerMode();
        }
        //if not in power-up mode, check if pacman should be dead
        else
        {
            manager.inNormalMode();
        }
        //check if pacman won
        if (pacmanAgent.countEats == pacmanAgent.totalEatsToWin)
        {
            std::cout << "CONGRATS, YOU WON !!!" << std::endl << std::endl;
            manager.endGame();
        }

    }

}

//trigger power-up mode
void GameManager::triggerPowerMode()
{
    //if a power-up mode is triggered, set all ghosts to white
    //and slow the ghosts down, start counting 5s
    manager.powerModeStart = true;
    manager.t_powerModeStart = std::chrono::steady_clock::now();
}

//handling power-up mode game control
void GameManager::inPowerMode()
{
    //eat another pill? then start over the 5s counting 
    if (manager.powerUpEaten)
    {
        manager.t_powerModeStart = std::chrono::steady_clock::now();
        manager.powerUpEaten = false;
    }

    //if 5s ellapsed, quit power-up mode
    time_point rightNow = std::chrono::steady_clock::now();
    if (rightNow - manager.t_powerModeStart >= manager.powerModeDuration)
    {
        quitPowerMode();
    }
}

//quit power-up mode
void GameManager::quitPowerMode()
{

    //flag reset
    manager.powerModeStart = false;
    manager.powerUpEaten = false;
}

//handling normal mode
void GameManager::inNormalMode()
{

}

//end game
void GameManager::endGame()
{
    manager.gameStatus = END;
    manager.pacDeaths = 0;
    //reset map
    for (int i = 0; i < 21; i++)
        for (int j = 0; j < 18; j++)
        {
            map[i][j] = manager.backUpMap[i][j];
        }
    
    //here, ready to restart
   
    pacmanAgent.respawn();
    pacmanAgent.countEats = 0;
    pacmanAgent.killed = false;
    manager.gameStatus = PAUSED;
}

//when pacman is killed,start again
void GameManager::startAnotherRound()
{
    
    pacmanAgent.respawn();
    pacmanAgent.killed = false;
    manager.gameStatus = PAUSED;
}

//constructor
GameManager::GameManager() :powerModeDuration(5)
{
    gameStatus = PAUSED;
}