#include <Windows.h>
#include "Pacman.h"
#include "Ghost.h"
#include "Feature.h"
#include "gameManager.h"
#include <GLFW/glfw3.h>
#include <GL/GLU.h>
#include "DrawPrimitives.h"
#include <iostream>
#include <iomanip>

#define _USE_MATH_DEFINES
#include <math.h>

#include<opencv2/opencv.hpp>
#include"MarkerTracker.h"

using namespace std;

cv::VideoCapture cap(9);
// Gameboard
int gameMap[22][19] = {
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1},
{1,3,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,3,1},
{1,2,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,2,1},
{1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
{1,2,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,2,1},
{1,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,1},
{1,1,1,1,2,1,1,1,0,1,0,1,1,1,2,1,1,1,1},
{0,0,0,1,2,1,0,0,0,0,0,0,0,1,2,1,0,0,0},
{1,1,1,1,2,1,0,1,1,1,1,1,0,1,2,1,1,1,1}, 
{0,0,0,0,2,0,0,1,0,0,0,1,0,0,2,0,0,0,0},
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
Ghost ghost1(0.0f, 0.6f, 1.0f); //blue-colored ghost
Ghost ghost2(1.0f, 0.15f, 0.79f); //pink-colored ghost
Ghost ghost3(1.0f, 0.5f, 0.0f); //yellow-colored ghost
Ghost ghost4(1.0f, 0.0f, 0.0f); //red-colored ghost

std::vector<Ghost*> ghosts = { &ghost1,&ghost2,&ghost3,&ghost4 };
extern void ghostAction(Ghost& ghost, const Pacman& pacman);

extern GameManager manager;
extern Feature feature;
typedef enum { IDLE, UP, DOWN, LEFT, RIGHT } moveFlag;
moveFlag oldPacMoveDir = IDLE, pacMoveDir; //current moving direction

GLuint gameboard = glGenLists(1);
const int camera_width = 640;
const int camera_height = 360;
//unsigned char background[camera_width * camera_height * 3];
unsigned char background[camera_width * camera_height * 3];
cv::Mat frame;
void detectMarkers(MarkerTracker* m, int* mouvement, float boardPos[16], bool* tlBrMarkers);

//reshape
void reshape(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Remember these frustum params!
    float ratio = (GLfloat)width / (GLfloat)height;
    int fov = 30;
    float _near = 0.01f, _far = 100.f;
    float top = tan((double)(fov * M_PI / 360.0f)) * _near;
    float bottom = -top;
    float left = ratio * bottom;
    float right = ratio * top;
    glFrustum(left, right, bottom, top, _near, _far);
}

///* Program & OpenGL initialization */
void init(int argc, char* argv[]) {
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelZoom(1.0, -1.0);

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
    glTranslatef(-8 * gameLength, 0, -9 * gameLength);
    for (int i = 1; i < 21; ++i)
    {
        for (int j = 1; j < 18; ++j)
        {
            if (gameMap[i][j] == PELLET)
            {
                drawSphere(0.1, 10, 10);
            }
            glTranslatef(1 * gameLength, 0, 0);         //go to next spot
        }
        glTranslatef(-17 * gameLength, 0, 1 * gameLength);  //go to next row
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
    gluCylinder(obj, 0.4 * gameLength, 0.4 * gameLength, 0.3 * gameLength, 32, 5);
    glTranslatef(0, 0, 0.3 * gameLength);
    drawSphere(0.2, 20, 20);
    glPopMatrix();
}

//draw powerpellet
void drawPowerPellets()
{
    //set material
    GLfloat mat_color[] = { 1.0f, 0.5f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_color);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_color);

    glColor3f(1.0f, 0.5f, 0.0f);
    //spots on the gameMap to draw powerpellet
    int powerpellet[4][2] =
    {
        {2,1},{2,17},{16,1},{16,17}
    };
    //draw 4 disks as powerpellet
    GLUquadric* objDisk = gluNewQuadric();
    for (int i = 0; i < 4; ++i)
    {
        int j = powerpellet[i][0], k = powerpellet[i][1];
        if (gameMap[j][k] != POWERPELLET)
            continue;
        float xShift = (float)gameLength * (k - 9);
        float zShift = (float)gameLength * (j - 10);
        glPushMatrix();
        glTranslatef(xShift, 0, zShift);
        glRotatef(-90, 1, 0, 0);
        gluCylinder(objDisk, 0.15, 0.15, 0.15 * gameLength, 20, 20);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(xShift, 0.15 * gameLength, zShift);
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
    GLfloat mat_color[] = { 0.015996f, 0.039546f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_color);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_color);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_color);

    /*draw blue cylinders*/
    glColor3f(0.015996f, 0.039546f, 1.0f);
    GLUquadricObj* objCylinder = gluNewQuadric();
    //background
    if(feature.enableBackground){
        glPushMatrix();
        glTranslatef(0, 0, 0);
        glRotatef(90, 1, 0, 0);
        glRectf(-9 * gameLength, -10 * gameLength, 9 * gameLength, 11 * gameLength);
        glPopMatrix();
    }
    

    //rectangle at the center
    glPushMatrix();
    glTranslatef(-2 * gameLength, 0, -1 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5); //down
    glTranslatef(4 * gameLength, 0, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5); //down
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 4 * gameLength, 32, 5); //left
    glTranslatef(2 * gameLength, 0, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 4 * gameLength, 32, 5); //left
    glTranslatef(0, 0, 4 * gameLength);
    glPopMatrix();

    //draw up-down T shapes (1/2)
    glPushMatrix();
    glTranslatef(-2 * gameLength, 0, 3 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 4 * gameLength, 32, 5);
    glTranslatef(8 * gameLength, 0, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 4 * gameLength, 32, 5);
    glTranslatef(-12 * gameLength, 0, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 4 * gameLength, 32, 5);
    glPopMatrix();

    //draw up-down T shapes (2/2)
    glPushMatrix();
    glTranslatef(0, 0, 3 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);
    glTranslatef(0, 0, -8 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);
    glTranslatef(0, 0, 12 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);
    glPopMatrix();

    //draw the up-down direction cylinder 
    glPushMatrix();
    glTranslatef(0, 0, -7 * gameLength);
    glRotatef(180, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 3 * gameLength, 32, 5);
    glPopMatrix();

    /*the rest can be symmetric, draw the right half first*/
    //draw the furthest wall (upper half)
    glPushMatrix();
    glTranslatef(0, 0, -10 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 9 * gameLength, 32, 5); //right
    glTranslatef(0, 0, 9 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 7 * gameLength, 32, 5); //down
    glTranslatef(0, 0, 7 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 3 * gameLength, 32, 5); //left
    glTranslatef(0, 0, 3 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5); //down
    glTranslatef(0, 0, 2 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 3 * gameLength, 32, 5); //right
    glPopMatrix();
    //draw the furthest wall (lower half)
    glPushMatrix();
    glTranslatef(0, 0, 11 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 9 * gameLength, 32, 5); //right
    glTranslatef(0, 0, 9 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 8 * gameLength, 32, 5); //up
    glTranslatef(0, 0, 8 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 3 * gameLength, 32, 5); //left
    glTranslatef(0, 0, 3 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5); //up
    glTranslatef(0, 0, 2 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 3 * gameLength, 32, 5); //right
    glPopMatrix();

    //upper right rectangles
    glPushMatrix();
    glTranslatef(2 * gameLength, 0, -8 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //down
    glTranslatef(0, 0, 1 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //right
    glTranslatef(0, 0, 2 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //up
    glTranslatef(0, 0, 1 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //left
    glPopMatrix();
    glPushMatrix();
    glTranslatef(6 * gameLength, 0, -8 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //down
    glTranslatef(0, 0, 1 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //right
    glTranslatef(0, 0, 1 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //up
    glTranslatef(0, 0, 1 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //left
    glPopMatrix();

    //T-shape
    glPushMatrix();
    glTranslatef(4 * gameLength, 0, -5 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 4 * gameLength, 32, 5);  //down
    glTranslatef(0, 0, 2 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //left
    glPopMatrix();
    //another T-shape
    glPushMatrix();
    glTranslatef(4 * gameLength, 0, 7 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //down
    glTranslatef(-2 * gameLength, 0, 2 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 5 * gameLength, 32, 5);  //right
    glPopMatrix();
    //L shape
    glPushMatrix();
    glTranslatef(6 * gameLength, 0, 5 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //down
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //right
    glPopMatrix();
    //line
    glPushMatrix();
    glTranslatef(2 * gameLength, 0, 5 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //right
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(4 * gameLength, 0, 1 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //down
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(8 * gameLength, 0, 7 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //right
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(6 * gameLength, 0, -5 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //right
    glPopMatrix();


    /*the rest is symmetric to the above, draw the left half of the maze*/
    //draw the furthest wall (upper half)
    glPushMatrix();
    glTranslatef(0, 0, -10 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 9 * gameLength, 32, 5); //left
    glTranslatef(0, 0, 9 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 7 * gameLength, 32, 5); //down
    glTranslatef(0, 0, 7 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 3 * gameLength, 32, 5); //right
    glTranslatef(0, 0, 3 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5); //down
    glTranslatef(0, 0, 2 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 3 * gameLength, 32, 5); //left
    glPopMatrix();
    //draw the furthest wall (lower half)
    glPushMatrix();
    glTranslatef(0, 0, 11 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 9 * gameLength, 32, 5); //left
    glTranslatef(0, 0, 9 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 8 * gameLength, 32, 5); //up
    glTranslatef(0, 0, 8 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 3 * gameLength, 32, 5); //right
    glTranslatef(0, 0, 3 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5); //up
    glTranslatef(0, 0, 2 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 3 * gameLength, 32, 5); //left
    glPopMatrix();

    //upper right rectangles
    glPushMatrix();
    glTranslatef(-2 * gameLength, 0, -8 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //down
    glTranslatef(0, 0, 1 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //left
    glTranslatef(0, 0, 2 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //up
    glTranslatef(0, 0, 1 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //right
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-6 * gameLength, 0, -8 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //down
    glTranslatef(0, 0, 1 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //left
    glTranslatef(0, 0, 1 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //up
    glTranslatef(0, 0, 1 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //right
    glPopMatrix();

    //T-shape
    glPushMatrix();
    glTranslatef(-4 * gameLength, 0, -5 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 4 * gameLength, 32, 5);  //down
    glTranslatef(0, 0, 2 * gameLength);
    glRotatef(90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //right
    glPopMatrix();
    //another T-shape
    glPushMatrix();
    glTranslatef(-4 * gameLength, 0, 7 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //down
    glTranslatef(2 * gameLength, 0, 2 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 5 * gameLength, 32, 5);  //left
    glPopMatrix();

    //L shape
    glPushMatrix();
    glTranslatef(-6 * gameLength, 0, 5 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //down
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //left
    glPopMatrix();
    //line
    glPushMatrix();
    glTranslatef(-2 * gameLength, 0, 5 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //left
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(-4 * gameLength, 0, 1 * gameLength);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 2 * gameLength, 32, 5);  //down
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(-8 * gameLength, 0, 7 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //left
    glPopMatrix();
    //another line
    glPushMatrix();
    glTranslatef(-6 * gameLength, 0, -5 * gameLength);
    glRotatef(-90, 0, 1, 0);
    gluCylinder(objCylinder, 0.15 * gameLength, 0.15 * gameLength, 1 * gameLength, 32, 5);  //left
    glPopMatrix();

    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * gameLength, 0, -10 * gameLength); //line 1
    drawSphere(0.1, 20, 20);
    glTranslatef(18 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-7 * gameLength, 0, -8 * gameLength); //line 3
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-7 * gameLength, 0, -7 * gameLength); //line 4
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-7 * gameLength, 0, -5 * gameLength); //line 6
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * gameLength, 0, -3 * gameLength); //line 8
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * gameLength, 0, -1 * gameLength); //line 10
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * gameLength, 0, 1 * gameLength); //line 12
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * gameLength, 0, 3 * gameLength); //line 14
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(3 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-7 * gameLength, 0, 5 * gameLength); //line 16
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(1 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-8 * gameLength, 0, 7 * gameLength); //line 18
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(4 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-7 * gameLength, 0, 9 * gameLength); //line 20
    drawSphere(0.1, 20, 20);
    glTranslatef(5 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(2 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glTranslatef(5 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    //add spheres at the corners, line by line
    glPushMatrix();
    glTranslatef(-9 * gameLength, 0, 11 * gameLength); //line 22 (last)
    drawSphere(0.1, 20, 20);
    glTranslatef(18 * gameLength, 0, 0);
    drawSphere(0.1, 20, 20);
    glPopMatrix();
    glEndList();
}
/*draws the arrows that serve as a marker for the user mouvement **/
void drawDirectionArrows() {
    /*draw blue arrow*/
    float centerX = camera_width / 20;
    float centerY = camera_height / 2;
    int distance = 0.5f;
    GLUquadric* objDisk = gluNewQuadric();
    GLUquadricObj* objCylinder = gluNewQuadric();
    //Position at center of arrows
    glPushMatrix();
    glTranslatef(-10, 0, 0);
    //top arrow
    glPushMatrix();
    if (feature.arrowUp) {
        glColor3f(0.0f, 1.0f, 0.0f);
    }
    else {
        glColor3f(1.0f, 0.0f, 0.0f);
    }
    glTranslatef(0, 0, -3);

    glPushMatrix();
    glRotatef(-45, 0, 1, 0);
    gluCylinder(objCylinder, 0.3 * gameLength, 0.3 * gameLength, 2.5 * gameLength, 32, 5); //down
    glPopMatrix();

    glPushMatrix();
    glRotatef(45, 0, 1, 0);
    gluCylinder(objCylinder, 0.3 * gameLength, 0.3 * gameLength, 2.5 * gameLength, 32, 5); //down
    glPopMatrix();

    //we reset to center
    glPopMatrix();
    //end top arrow
 
    //bottom arrow
    //we go to center of bottom arrow
    glPushMatrix();
    if (feature.arrowDown) {
        glColor3f(0.0f, 1.0f, 0.0f);
    }
    else {
        glColor3f(1.0f, 0.0f, 0.0f);
    }
    glTranslatef(0, 0, 3);

    glPushMatrix();
    glRotatef(-45 - 90, 0, 1, 0);
    gluCylinder(objCylinder, 0.3 * gameLength, 0.3 * gameLength, 2.5 * gameLength, 32, 5); //down
    glPopMatrix();

    glPushMatrix();
    glRotatef(45 + 90, 0, 1, 0);
    gluCylinder(objCylinder, 0.3 * gameLength, 0.3 * gameLength, 2.5 * gameLength, 32, 5); //down
    glPopMatrix();
    //we reset to center
    glPopMatrix();
    //end bottom arrow

    //left arrow
    //we go to center of bottom arrow
    glPushMatrix();
    if (feature.arrowLeft) {
        glColor3f(0.0f, 1.0f, 0.0f);
    }
    else {
        glColor3f(1.0f, 0.0f, 0.0f);
    }
    glTranslatef(-3, 0, 0);

    glPushMatrix();
    glRotatef(45, 0, 1, 0);
    gluCylinder(objCylinder, 0.3 * gameLength, 0.3 * gameLength, 2.5 * gameLength, 32, 5); //down
    glPopMatrix();

    glPushMatrix();
    glRotatef(45 + 90, 0, 1, 0);
    gluCylinder(objCylinder, 0.3 * gameLength, 0.3 * gameLength, 2.5 * gameLength, 32, 5); //down
    glPopMatrix();
    //we reset to center
    glPopMatrix();
    //end left arrow

    //Right arrow
    //we go to center of bottom arrow
    glPushMatrix();
    if (feature.arrowRight) {
        glColor3f(0.0f, 1.0f, 0.0f);
    }
    else {
        glColor3f(1.0f, 0.0f, 0.0f);
    }
    glTranslatef(3, 0, 0);

    glPushMatrix();
    glRotatef(-45, 0, 1, 0);
    gluCylinder(objCylinder, 0.3 * gameLength, 0.3 * gameLength, 2.5 * gameLength, 32, 5); //down
    glPopMatrix();

    glPushMatrix();
    glRotatef(-45 - 90, 0, 1, 0);
    gluCylinder(objCylinder, 0.3 * gameLength, 0.3 * gameLength, 2.5 * gameLength, 32, 5); //down
    glPopMatrix();
    //we reset to center
    glPopMatrix();
    //end right arrow

    //pop final matrix
    glPopMatrix();

}
//display func
void display(GLFWwindow* window, cv::Mat img_bg, float *boardPos, bool tlBrMarker)
{

    memcpy(background, img_bg.data, sizeof(background));

    // Added in Exercise 8 - End *****************************************************************

        // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Needed for rendering the real camera image
    glMatrixMode(GL_MODELVIEW);
    // No position changes
    glLoadIdentity();

    // Added in Exercise 8 - Start *****************************************************************

    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    // Push the projection matrix (frustum) -> frustum will be saved on the stack
    glPushMatrix();
    glLoadIdentity();
    // In the ortho view all objects stay the same size at every distance
    glOrtho(0.0, camera_width, 0.0, camera_height, -1, 1);

    // -> Render the camera picture as background texture
    // Making a raster of the image -> -1 otherwise overflow
    glRasterPos2i(0, camera_height - 1);
    // Load and render the camera image -> unsigned byte because of bkgnd as unsigned char array
    // bkgnd 3 channels -> pixelwise rendering
    glDrawPixels(camera_width, camera_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, background);

    // Go back to the previous projection -> frustum
    glPopMatrix();

    // Activate depth -> that snowman can be scaled with depth
    glEnable(GL_DEPTH_TEST);

    // Move to marker-position
    glMatrixMode(GL_MODELVIEW);

    // Sadly doesn't work for Windows -> so we made own solution!
    //glLoadTransposeMatrixf(resultMatrix);

    // -> Transpose the Modelview Matrix
    float resultTransposedMatrix[16];
    for (int x = 0; x < 4; ++x) {
        for (int y = 0; y < 4; ++y) {
            // Change columns to rows
            resultTransposedMatrix[x * 4 + y] = boardPos[y * 4 + x];
        }
    }
    // Load the transpose matrix
    glLoadMatrixf(resultTransposedMatrix);

    // Rotate 90 desgress in x-direction
    if (tlBrMarker) {
        glRotatef(90, 0, 0, 1);
        glRotatef(-90, 1, 0, 0);
        //cout << "lol" << endl;
    }
    else
        glRotatef(-90, 1, 0, 0);

    // Scale down!
    glScalef(0.03, 0.03, 0.03);

    // render
    drawDirectionArrows();
    drawGameboard();
    drawPellets();
    drawPowerPellets();
    // draw pacman,cosidering motion
    switch (pacMoveDir)
    {
    case IDLE:
        pacmanAgent.drawPacman();
        break;
    case UP:
        pacmanAgent.moveUp();
        pacmanAgent.drawPacman();
        feature.colorArrowTop();
        break;
    case DOWN:
        pacmanAgent.moveDown();
        pacmanAgent.drawPacman();
        feature.colorArrowBottom();
        break;
    case LEFT:
        pacmanAgent.moveLeft();
        pacmanAgent.drawPacman();
        feature.colorArrowLeft();
        break;
    case RIGHT:
        pacmanAgent.moveRight();
        pacmanAgent.drawPacman();
        feature.colorArrowRight();
        break;
    }

    ghost1.drawGhost();
    ghost2.drawGhost();
    ghost3.drawGhost();
    ghost4.drawGhost();
    ////allow keyboard control of direction
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
                break;
            case GLFW_KEY_S:
                pacMoveDir = DOWN;
                break;
            case GLFW_KEY_A:
                pacMoveDir = LEFT;
                break;
            case GLFW_KEY_D:
                pacMoveDir = RIGHT;
                break;
            case GLFW_KEY_B:
                feature.setBackground();
                break;
            case GLFW_KEY_P:
                feature.setSound();
                break;
            default:
                break;
        }
    }
}


//idle func
void idle()
{
    //when pacman dies/wins,stop the animation for a while
    while (manager.gameStatus == PACDIE || manager.gameStatus == END)
    {
        //disable control of directionc
        pacmanAgent.enableKey = false;
        //resume pacman's moveflag
        pacMoveDir = IDLE;
        //stop redisplaying until everything is resumed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
//float oldboardPos[16];
void detectMarkers(MarkerTracker* m, int* mouvement, float boardPos[16], bool* tlBrMarkers) {
    //if a new frame is available:
    float distance;
    float results[5][16];

    if (cap.read(frame)) {
        bool found[5];
        for (int i = 0; i <= 4; i++)
            found[i] = false;
        cv::Vec2f inputPos;

        //cv::imshow("test", frame);


        m->findMarker(frame, results, found, inputPos);

        int check = 0;
        //calculate game board position
        cv::Vec3f markerPoss[4];

        for (int i = 0; i < 4; i++) {
            if (found[i]) {
                markerPoss[i] = { results[i][3],results[i][7],results[i][11] };
                check++;
            }
        }

        if (check >= 2) {

            /* we use topleft and bottom right marker*/
            if (found[0] && found[3]) {
                for (int i = 0; i < 16; i++) {
                    //Need to rotate -90degree
                    boardPos[i] = results[0][i] * 0.5 + results[3][i] * 0.5;
                    //oldboardPos[i] = boardPos[i];
                    *tlBrMarkers = true;
                }
            }
            else if (found[1] && found[2]) { /*We use top right and bootom left marker*/
                for (int i = 0; i < 16; i++) {
                    boardPos[i] = results[1][i] * 0.5 + results[2][i] * 0.5;
                    //oldboardPos[i] = boardPos[i];
                    *tlBrMarkers = false;
                }
            }

        }
        /*else {
            //if no marker detected we use old oboard position
            cout << "no marker" << endl;
            for (int i = 0; i < 16; i++) {
                boardPos[i] = oldboardPos[i];
            }
            
        }*/
        
        
        //SCRAPPED : handheld tracker mouvement, RIP

        /*float originX = boardPos[3]-0.02f, originY = boardPos[11]; //TODO : makes this In H File
        const float deadZoneRange = 0.1f;
        //we calculate the mouvement direction
        if (found[4]) {
            cout << "board Origin : " << originX << " and " << originY << endl;
            cout << "marker pos " << results[4][3] << " and " << results[4][11] << endl;
            float Xdistance = results[4][3] - originX;
            float Ydistance = results[4][11] - originY;
            cout << "distance x" << Xdistance << " and y  " << Ydistance << endl;
            bool isXmovement = (Xdistance > Ydistance);

            //
            *mouvement = -1;
            if (isXmovement && Xdistance > deadZoneRange) {
                *mouvement = (Xdistance > 0) ? 0 : 1;
            }
            else if (Ydistance > deadZoneRange) {
                *mouvement = (Ydistance > 0) ? 2 : 3;
            }
            cout << "mouvement int is " << *mouvement << endl;
        }*/
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
    window = glfwCreateWindow(camera_width, camera_height, "ARPacman", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    
    
    /*cap.set(cv::CAP_PROP_BUFFERSIZE, 3);

    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FOURCC, 0x32595559);
    cap.set(cv::CAP_PROP_FPS, 25);*/

    //const string streamWindow = "Stream";
    cout << "start" << endl;
    if (!cap.isOpened()) {
        cout << "No webcam, using video file" << endl;
        cap.open("MarkerMovie.mp4");
        if (cap.isOpened() == false) {
            cout << "No video!" << endl;
            exit(0);
            return -1;
        }
    } else cout << "Found webcam" << endl;

    //cv::namedWindow("test", CV_WINDOW_AUTOSIZE);


    MarkerTracker* m = new MarkerTracker(0.070,125,125);

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

    //pos board
    float boardPos[16];
    
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {

        int mouvement;
        bool tlbrMarkers; //need a different rotation based on the markers detected
        detectMarkers(m, &mouvement, boardPos, &tlbrMarkers);
        //else take old pos

        // Render here
        display(window, frame, boardPos, tlbrMarkers);

        // Swap front and back buffers
        // -> the front buffer is the current in the window rendered frame
        // -> and the back buffer is the newly generated frame buffer
        glfwSwapBuffers(window);

        // Poll for and process events -> Check for inputs: e.g. mouse clicks on the exit button/keyboard gets pressed/etc.
        glfwPollEvents();

    }


    cv::destroyAllWindows();

    // Free the memory (IMPORTANT to avoid memory leaks!!!)
    glfwTerminate();

    return 0;
}