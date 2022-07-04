#include <Windows.h>
#include "Pacman.h"
#include "Ghost.h"
#include "gameManager.h"
#include <GLFW/glfw3.h>
#include <GL/GLU.h>
#include "DrawPrimitives.h"
#include <iostream>
#include <iomanip>

//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

//#include "PoseEstimation.h"

using namespace std;


// Gameboard
int map[22][19] = {
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
};

Pacman pacmanAgent;
Ghost ghost1(0.0f, 0.96f, 1.0f); //green-colored ghost
Ghost ghost2(1.0f, 0.75f, 0.79f); //pink-colored ghost
Ghost ghost3(0.85f, 0.65f, 0.13f); //orange-colored ghost
Ghost ghost4(1.0f, 0.0f, 0.0f); //red-colored ghost

std::vector<Ghost*> ghosts = { &ghost1,&ghost2,&ghost3,&ghost4 };
extern void ghostAction(Ghost& ghost, const Pacman& pacman);

extern GameManager manager;
float angle = -45; //grid rotation angle
typedef enum { IDLE, UP, DOWN, LEFT, RIGHT } moveFlag;
moveFlag oldPacMoveDir = IDLE, pacMoveDir; //current moving direction

GLuint gameboard = glGenLists(1);


//reshape
void reshape(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (GLfloat)width / (GLfloat)height, 1.0, 40.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

///* Program & OpenGL initialization */
void init(int argc, char* argv[]) {
    // Enable and set color material -> ambient, diffuse, specular
    glEnable(GL_COLOR_MATERIAL);
    // Set background color, default is black
    glClearColor(0.3, 0.3, 1.0, 1.0);

    // Enable and set depth parameters
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0);

    // Light parameters
    GLfloat light_pos[] = { 1.0, 1.0, 1.0, 0.0 };
    // Light from top
    //GLfloat light_pos[] = {0, 1, 0, 0};
    // Light from bottom
    //GLfloat light_pos[] = {0, -1, 0, 0};

    // Takes the object color and multiplies with the ambient value
    GLfloat light_amb[] = { 0.2, 0.2, 0.2, 1.0 };

    // We need the normal vector for each face of the mesh to calculate the specular and diffuse lighting parts
    // """float diff = max(dot(norm, lightDir), 0.0)"""
    // -> If the light direction is in the opposite direction of the face, then the final pixel color will be black
    GLfloat light_dif[] = { 0.9, 0.9, 0.9, 1.0 };
    // Angel between the light direction and the normal of the face -> Like a mirror, the incoming light gets reflected
    // Directional light source
    GLfloat light_spe[] = { 1, 1, 1, 1.0 };

    // -> All three lighting parts combined is called Phong Lighting Model

    // Set the light values -> In this scenario we use only one white light
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_spe);
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

//draw pellets
void drawPellets()
{
    //set material
    GLfloat mat_color[] = { 0.75, 0.75, 0.75, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_color);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_color);

    glColor3f(0.75, 0.75, 0.75); //silver solor

    glPushMatrix();
    glTranslatef(-8 * len, 0, -9 * len);
    for (int i = 1; i < 21; ++i)
    {
        for (int j = 1; j < 18; ++j)
        {
            if (map[i][j] == PELLET)
            {
                drawSphere(0.1, 10, 10);
            }
            glTranslatef(1 * len, 0, 0);         //go to next spot
        }
        glTranslatef(-17 * len, 0, 1 * len);  //go to next row
    }
    glPopMatrix();

}

//draw pacman
void Pacman::drawPacman()
{
    GLfloat mat_color[] = { 1.0, 1.0, 0, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_color);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_color);
    glEnable(GL_COLOR_MATERIAL);

    glColor3f(1.0, 1.0, 0.0); //yellow
    glPushMatrix();
    glTranslatef(this->x, 0, this->z);
    drawSphere(0.3, 20, 20);
    glPopMatrix();
}

void Ghost::drawGhost()
{
    //material color
    GLfloat mat_color[] = { this->colorR, this->colorG, this->colorB, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_color);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_color);
    glEnable(GL_COLOR_MATERIAL);

    glColor3f(this->colorR, this->colorG, this->colorB);
    GLUquadric* obj = gluNewQuadric();
    glPushMatrix();
    glTranslatef(this->x, 0, this->z);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(obj, 0.4 * len, 0.4 * len, 0.3 * len, 32, 5);
    glTranslatef(0, 0, 0.3 * len);
    drawSphere(0.2, 20, 20);
    glPopMatrix();
}

//draw powerpellet
void drawPowerPellets()
{
    //set material
    GLfloat mat_color[] = { 0.93f, 0.86f, 0.51f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_color);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_color);

    glColor3f(0.93f, 0.86f, 0.51f); //golden color
    //spots on the map to draw powerpellet
    int powerpellet[4][2] =
    {
        {2,1},{2,17},{16,1},{16,17}
    };
    //draw 4 disks as powerpellet
    GLUquadric* objDisk = gluNewQuadric();
    for (int i = 0; i < 4; ++i)
    {
        int j = powerpellet[i][0], k = powerpellet[i][1];
        if (map[j][k] != POWERPELLET)
            continue;
        float xShift = (float)len * (k - 9);
        float zShift = (float)len * (j - 10);
        glPushMatrix();
        glTranslatef(xShift, 0, zShift);
        glRotatef(-90, 1, 0, 0);
        gluCylinder(objDisk, 0.15, 0.15, 0.15 * len, 20, 20);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(xShift, 0.15 * len, zShift);
        glRotatef(-90, 1, 0, 0);
        gluDisk(objDisk, 0.00, 0.15, 20, 20);
        glPopMatrix();
    }
}

//draw gameboard
void drawGameboard()
{
    //for IDLE image,create a display list
    glNewList(gameboard, GL_COMPILE);
    GLfloat mat_color[] = { 0.0f, 0.60f, 0.80f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_color);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_color);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_color);

    /*draw blue cylinders*/
    glColor3f(0.0f, 0.60f, 0.80f);
    GLUquadricObj* objCylinder = gluNewQuadric();
    //rectangle at the center
    glPushMatrix();
    glTranslatef(-2 * len, 0, -1 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5); //down
    glTranslatef(4 * len, 0, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5); //down
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 4 * len, 32, 5); //left
    glTranslatef(2 * len, 0, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 4 * len, 32, 5); //left
    glTranslatef(0, 0, 4 * len);
    glPopMatrix();

    //draw up-down T shapes (1/2)
    glPushMatrix();
    glTranslatef(-2 * len, 0, 3 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 4 * len, 32, 5);
    glTranslatef(8 * len, 0, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 4 * len, 32, 5);
    glTranslatef(-12 * len, 0, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 4 * len, 32, 5);
    glPopMatrix();

    //draw up-down T shapes (2/2)
    glPushMatrix();
    glTranslatef(0, 0, 3 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);
    glTranslatef(0, 0, -8 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);
    glTranslatef(0, 0, 12 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);
    glPopMatrix();

    //draw the up-down direction cylinder 
    glPushMatrix();
    glTranslatef(0, 0, -7 * len);
    glRotatef(180, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 3 * len, 32, 5);
    glPopMatrix();

    /*the rest can be symmetric, draw the right half first*/
    //draw the furthest wall (upper half)
    glPushMatrix();
    glTranslatef(0, 0, -10 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 9 * len, 32, 5); //right
    glTranslatef(0, 0, 9 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 7 * len, 32, 5); //down
    glTranslatef(0, 0, 7 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 3 * len, 32, 5); //left
    glTranslatef(0, 0, 3 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5); //down
    glTranslatef(0, 0, 2 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 3 * len, 32, 5); //right
    glPopMatrix();
    //draw the furthest wall (lower half)
    glPushMatrix();
    glTranslatef(0, 0, 11 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 9 * len, 32, 5); //right
    glTranslatef(0, 0, 9 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 8 * len, 32, 5); //up
    glTranslatef(0, 0, 8 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 3 * len, 32, 5); //left
    glTranslatef(0, 0, 3 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5); //up
    glTranslatef(0, 0, 2 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 3 * len, 32, 5); //right
    glPopMatrix();

    //upper right rectangles
    glPushMatrix();
    glTranslatef(2 * len, 0, -8 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //down
    glTranslatef(0, 0, 1 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //right
    glTranslatef(0, 0, 2 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //up
    glTranslatef(0, 0, 1 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //left
    glPopMatrix();
    glPushMatrix();
    glTranslatef(6 * len, 0, -8 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //down
    glTranslatef(0, 0, 1 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //right
    glTranslatef(0, 0, 1 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //up
    glTranslatef(0, 0, 1 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //left
    glPopMatrix();

    //T-shape
    glPushMatrix();
    glTranslatef(4 * len, 0, -5 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 4 * len, 32, 5);  //down
    glTranslatef(0, 0, 2 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //left
    glPopMatrix();
    //another T-shape
    glPushMatrix();
    glTranslatef(4 * len, 0, 7 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //down
    glTranslatef(-2 * len, 0, 2 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 5 * len, 32, 5);  //right
    glPopMatrix();
    //L shape
    glPushMatrix();
    glTranslatef(6 * len, 0, 5 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //down
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //right
    glPopMatrix();
    //line
    glPushMatrix();
    glTranslatef(2 * len, 0, 5 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //right
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(4 * len, 0, 1 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //down
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(8 * len, 0, 7 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //right
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(6 * len, 0, -5 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //right
    glPopMatrix();


    /*the rest is symmetric to the above, draw the left half of the maze*/
    //draw the furthest wall (upper half)
    glPushMatrix();
    glTranslatef(0, 0, -10 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 9 * len, 32, 5); //left
    glTranslatef(0, 0, 9 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 7 * len, 32, 5); //down
    glTranslatef(0, 0, 7 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 3 * len, 32, 5); //right
    glTranslatef(0, 0, 3 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5); //down
    glTranslatef(0, 0, 2 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 3 * len, 32, 5); //left
    glPopMatrix();
    //draw the furthest wall (lower half)
    glPushMatrix();
    glTranslatef(0, 0, 11 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 9 * len, 32, 5); //left
    glTranslatef(0, 0, 9 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 8 * len, 32, 5); //up
    glTranslatef(0, 0, 8 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 3 * len, 32, 5); //right
    glTranslatef(0, 0, 3 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5); //up
    glTranslatef(0, 0, 2 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 3 * len, 32, 5); //left
    glPopMatrix();

    //upper right rectangles
    glPushMatrix();
    glTranslatef(-2 * len, 0, -8 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //down
    glTranslatef(0, 0, 1 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //left
    glTranslatef(0, 0, 2 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //up
    glTranslatef(0, 0, 1 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //right
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-6 * len, 0, -8 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //down
    glTranslatef(0, 0, 1 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //left
    glTranslatef(0, 0, 1 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //up
    glTranslatef(0, 0, 1 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //right
    glPopMatrix();

    //T-shape
    glPushMatrix();
    glTranslatef(-4 * len, 0, -5 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 4 * len, 32, 5);  //down
    glTranslatef(0, 0, 2 * len);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //right
    glPopMatrix();
    //another T-shape
    glPushMatrix();
    glTranslatef(-4 * len, 0, 7 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //down
    glTranslatef(2 * len, 0, 2 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 5 * len, 32, 5);  //left
    glPopMatrix();

    //L shape
    glPushMatrix();
    glTranslatef(-6 * len, 0, 5 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //down
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //left
    glPopMatrix();
    //line
    glPushMatrix();
    glTranslatef(-2 * len, 0, 5 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //left
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(-4 * len, 0, 1 * len);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 2 * len, 32, 5);  //down
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(-8 * len, 0, 7 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //left
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(-6 * len, 0, -5 * len);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * len, 0.15 * len, 1 * len, 32, 5);  //left
    glPopMatrix();

    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * len, 0, -10 * len); //line 1
    drawSphere(0.1, 20, 20);
    glTranslatef(18 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-7 * len, 0, -8 * len); //line 3
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-7 * len, 0, -7 * len); //line 4
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-7 * len, 0, -5 * len); //line 6
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * len, 0, -3 * len); //line 8
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * len, 0, -1 * len); //line 10
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * len, 0, 1 * len); //line 12
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * len, 0, 3 * len); //line 14
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-7 * len, 0, 5 * len); //line 16
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-8 * len, 0, 7 * len); //line 18
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-7 * len, 0, 9 * len); //line 20
    drawSphere(0.1, 20, 20);
    glTranslatef(5 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(5 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * len, 0, 11 * len); //line 22 (last)
    drawSphere(0.1, 20, 20);
    glTranslatef(18 * len, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    glEndList();
}

//display func
void display(GLFWwindow* window)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0, 0, 0, 1.0); //background color is black
    glLoadIdentity();
    // Set the camera with the y-axis pointing up
    gluLookAt(15, 15, 15, 0, 0, 0, 0, 1, 0);
    glRotatef(-angle, 0, 1, 0); //press R to rotate
    drawGameboard();
    drawPellets();
    drawPowerPellets();
    //draw pacman,cosidering motion
    switch (pacMoveDir)
    {
    case IDLE:
        pacmanAgent.drawPacman();
        break;
    case UP:
        pacmanAgent.moveUp();
        pacmanAgent.drawPacman();
        break;
    case DOWN:
        pacmanAgent.moveDown();
        pacmanAgent.drawPacman();
        break;
    case LEFT:
        pacmanAgent.moveLeft();
        pacmanAgent.drawPacman();
        break;
    case RIGHT:
        pacmanAgent.moveRight();
        pacmanAgent.drawPacman();
        break;
    }

    ghost1.drawGhost();
    ghost2.drawGhost();
    ghost3.drawGhost();
    ghost4.drawGhost();
    //allow keyboard control of direction
    pacmanAgent.enableKey = true;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    //while keyboard is disabled, do nothing
    if (!pacmanAgent.enableKey)
    {
        return;
    }
    //while keyboard is enabled, press any key to start the game
    else if (manager.gameStatus == PAUSED)
    {
        manager.gameStatus = STARTED;
        std::cout << "Game has started";
    }
    //control pac direction
    if (action == GLFW_PRESS){
    switch (key)
    {
    case GLFW_KEY_W:
        pacMoveDir = UP;
        std::cout << pacMoveDir;
        break;
    case GLFW_KEY_S:
        pacMoveDir = DOWN;
        std::cout << pacMoveDir;
        break;
    case GLFW_KEY_A:
        pacMoveDir = LEFT;
        std::cout << pacMoveDir;
        break;
    case GLFW_KEY_D:
        pacMoveDir = RIGHT;
        std::cout << pacMoveDir;
        break;
    default:
        break;
    }}
      
}


//idle func
void idle()
{
    //when pacman dies/wins,stop the animation for a while
    while (manager.gameStatus == PACDIE || manager.gameStatus == END)
    {
        //disable control of direction
        pacmanAgent.enableKey = false;
        //resume pacman's moveflag
        pacMoveDir = IDLE;
        //stop redisplaying until everything is resumed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main(int argc, char* argv[])
{
    // OpenGL can't create an output window -> Render output with GLFW
    // Window where we will render the scene
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Initialize the window system
    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(640, 480, "ARPacman", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Set callback function for GLFW -> When the window size changes, the content will be adapted to the new window size
    glfwSetFramebufferSizeCallback(window, reshape);

    glfwMakeContextCurrent(window);
    // The minimum number of screen updates to wait for until the buffers are swapped by glfwSwapBuffers
    glfwSwapInterval(1);

    // enable Keyboard control
    glfwSetKeyCallback(window, key_callback);

    // Initialize the frustum with the size of the framebuffer
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    reshape(window, width, height);

    // Initialize the GL library
    // -> Give app arguments for configuration, e.g. depth or color should be activated or not
    init(argc, argv);

    manager.controlThread = std::thread(gameManager, std::ref(manager));
    for (auto pGhost : ghosts)
    {
        pGhost->ghostThread = std::thread(ghostAction, std::ref(*pGhost), std::ref(pacmanAgent));
    }

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        // Render here
        display(window);

        // Swap front and back buffers
        // -> the front buffer is the current in the window rendered frame
        // -> and the back buffer is the newly generated frame buffer
        glfwSwapBuffers(window);

        // Poll for and process events -> Check for inputs: e.g. mouse clicks on the exit button/keyboard gets pressed/etc.
        glfwPollEvents();

    }


    // Free the memory (IMPORTANT to avoid memory leaks!!!)
    glfwTerminate();

    return 0;
}
