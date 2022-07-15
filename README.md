# ARPacman
Team project for the course IN2018 Erweiterte Realität in the summer semester 2022 at Technical University Munich

## Project Description
We created a marker-tracking based AR pacman game. To achieve this we utilized openCV and openGL as taught in the lecture and exercises
re-using and adapting code where we saw fit.

## How to play
W: Pacman up
S: Pacman down
A: Pacman left
D: Pacman right
B: enable/disable Board Background
P: enable/disable Pacman Soundtrack

## Setup Windows

### ARPacman Properties

**VC++ Directories:**

Include Directories:
C:\opencv\build\include

Library Directories:
C:\opencv\build\x64\vc15\lib

**Linker:**

Input - Additional Dependencies

* glfw3.lib
* opengl32.lib
* glew32.lib
* Glu32.lib
* opencv_world3414d.lib
* User32.lib
* Shell32.lib
* Gdi32.lib
* winmm.lib

