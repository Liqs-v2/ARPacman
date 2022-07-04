#include <Windows.h>
#include "GameManager.h"
#include "Ghost.h"
#include <iostream>

extern Pacman pacmanAgent;
extern std::vector<Ghost*> ghosts;
extern void ghostAction(Ghost& ghost, const Pacman& pacman);
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
    for (auto gh : ghosts)
    {
        gh->setColor(1.0f, 1.0f, 1.0f);
        gh->ghostSpeed = LOW;
    }
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
    for (int i = 0; i < 4; i++)
    {
        if (ArrayX(ghosts[i]->x) == ArrayX(pacmanAgent.x) && ArrayZ(ghosts[i]->z) == ArrayZ(pacmanAgent.z))
        {
            ghosts[i]->killed = true;
            ghosts[i]->setColor(0, 0, 0); //dead, body is invisible
            ghosts[i]->x = 30 * len; //dead, move it out of scene
            ghosts[i]->z = 50 * len;
        }
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
    for (auto gh : ghosts)
    {
        if (gh->killed)
            continue;
        gh->setColor(gh->originalR, gh->originalG, gh->originalB);
        gh->ghostSpeed = HIGH;
    }

    //flag reset
    manager.powerModeStart = false;
    manager.powerUpEaten = false;
}

//handling normal mode
void GameManager::inNormalMode()
{
    for (int i = 0; i < 4; i++)
    {
        //if pacman is killed, start another round or end game
        if (ArrayX(ghosts[i]->x) == ArrayX(pacmanAgent.x) &&
            ArrayZ(ghosts[i]->z) == ArrayZ(pacmanAgent.z))
        {
            pacmanAgent.killed = true;
            manager.pacDeaths++;
            manager.gameStatus = PACDIE;

            if (manager.pacDeaths == 3)
            {
                std::cout << "Pacman deaths : 3" << std::endl;
                std::cout << "GAME OVER, YOU LOST ..." << std::endl << std::endl;
                manager.endGame();
            }
            else
            {
                std::cout << "Pacman deaths : " << manager.pacDeaths << std::endl;
                manager.startAnotherRound();
            }

            break;
        }
    }
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
    
    while (!ghosts[0]->ready4AnotherRound || !ghosts[1]->ready4AnotherRound ||
        !ghosts[2]->ready4AnotherRound || !ghosts[3]->ready4AnotherRound)
    {
        Sleep(200);
    }
    //here, ready to restart
    //let ghosts resume their status, and go catch pacman
    for (auto gh : ghosts)
    {
        gh->resume();
        gh->ghostThread.detach();
        gh->ghostThread = std::thread(ghostAction, std::ref(*gh), std::ref(pacmanAgent));
    }
   
    pacmanAgent.respawn();
    pacmanAgent.countEats = 0;
    pacmanAgent.killed = false;
    manager.gameStatus = PAUSED;
}

//when pacman is killed,start again
void GameManager::startAnotherRound()
{
    while (!ghosts[0]->ready4AnotherRound || !ghosts[1]->ready4AnotherRound ||
        !ghosts[2]->ready4AnotherRound || !ghosts[3]->ready4AnotherRound)
    {
        Sleep(200);
    }

    //here, ready to start another round
    //let ghosts resume their status, and go catch pacman
    for (auto gh : ghosts)
    {
        gh->resume();
        gh->ghostThread.detach();
        gh->ghostThread = std::thread(ghostAction, std::ref(*gh), std::ref(pacmanAgent));
    }
    pacmanAgent.respawn();
    pacmanAgent.killed = false;
    manager.gameStatus = PAUSED;
}

//constructor
GameManager::GameManager() :powerModeDuration(5)
{
    gameStatus = PAUSED;
}