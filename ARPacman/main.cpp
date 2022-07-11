#include <Windows.h>
#include <GLFW/glfw3.h>
#include <GL/GLU.h>
#include "DrawPrimitives.h"
#include <iostream>
#include <iomanip>
//#include "OpenCVSetup.h"
//#include<opencv2/opencv.hpp>


//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#include <opencv2/opencv.hpp>
#include "MarkerTracker.h"
#include "PoseEstimation.h"

#include "Pacman.h"
#include "Ghost.h"
#include "gameManager.h"

using namespace std;

//cv::VideoCapture cap(9);
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

cv::VideoCapture cap;

// Camera settings
const int camera_width = 640;
const int camera_height = 480;
const int virtual_camera_angle = 30;
unsigned char background[camera_width * camera_height * 3];

void initVideoStream(cv::VideoCapture& cap) {
    if (cap.isOpened())
        cap.release();

    cap.open(1);
    if (cap.isOpened() == false) {
        std::cout << "No webcam found, using a video file" << std::endl;
        cap.open("MarkerMovie.MP4");
        if (cap.isOpened() == false) {
            std::cout << "No video file found. Exiting." << std::endl;
            exit(0);
        }
    }
}

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
//const int camera_width = 640;
//const int camera_height = 360;
//unsigned char background[camera_width * camera_height * 3];
//unsigned char background[camera_width * camera_height * 3];
cv::Mat frame;
//float* detectMarkers(MarkerTracker* m);

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
    /*
    gluPerspective(30.0, (GLfloat)width / (GLfloat)height, 1.0, 40.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    */
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
    GLfloat mat_color[] = { 0.93f, 0.86f, 0.51f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_color);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_color);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_color);

    glColor3f(0.93f, 0.86f, 0.51f); //golden color
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
    GLfloat mat_color[] = { 0.0f, 0.60f, 0.80f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_color);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_color);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_color);

    /*draw blue cylinders*/
    glColor3f(0.0f, 0.60f, 0.80f);
    GLUquadricObj* objCylinder = gluNewQuadric();
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

//display func
void display(GLFWwindow* window, const cv::Mat& img_bg, float resultMatrix[16])
{

    //float x = boardPos[3];
    // float y = boardPos[7];
    // float z = boardPos[11];
    // cout << "boardPos: [3]=" << x << " [7]=" << y << " [11]" << z << endl;

    // for (size_t i = 0; i < 16; i++)
    // {
    //     cout << "boardPos: ["<< i <<"] = "<< boardPos[i] << endl;
    // }

    // float resultTransposedMatrix[16];
    // for (int x = 0; x < 4; ++x) {
    //     for (int y = 0; y < 4; ++y) {
    //         // Change columns to rows
    //         resultTransposedMatrix[x * 4 + y] = boardPos[y * 4 + x];
    //         cout << boardPos[y * 4 + x] << " " << (y * 4 + x) << endl;
    //     }
    // }

    //// 0 4 8 12
    //// 1 5 9 13
    //// 2 6 10 14
    //// 3 7 11 15

    // // Copy picture data into bkgnd array
    // memcpy(background, img_bg.data, sizeof(background));
     //memcpy(background, img_bg.data, sizeof(background));

    // x = boardPos[3];
    // y = boardPos[7];
    // z = boardPos[11];
    // cout << "display" << x << " " << y << " " << z << endl;

     //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     //glClearColor(0, 0, 0, 1);

     //glMatrixMode(GL_MODELVIEW);
     //glLoadIdentity();

     //glDisable(GL_DEPTH_TEST);
     //glMatrixMode(GL_PROJECTION);
     //glPushMatrix();
     //glLoadIdentity();

     //// In the ortho view all objects stay the same size at every distance
     //glOrtho(0.0, camera_width, 0.0, camera_height, -1, 1);

     //// -> Render the camera picture as background texture
     //// Making a raster of the image -> -1 otherwise overflow
     //glRasterPos2i(0, camera_height - 1);

     //// Load and render the camera image -> unsigned byte because of bkgnd as unsigned char array
     //// bkgnd 3 channels -> pixelwise rendering
     //glDrawPixels(camera_width, camera_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, background);

     //glPopMatrix();
     //glEnable(GL_DEPTH_TEST);

     //// Specifies which matrix stack is the target for subsequent matrix operations
     //// -> Three values are accepted: GL_MODELVIEW, GL_PROJECTION, and GL_TEXTURE
     //// Move to marker-position
     //glMatrixMode(GL_MODELVIEW);

     //glLoadIdentity();

     //// -> Transpose the Modelview Matrix
     //float resultTransposedMatrix[16];
     //for (int x = 0; x < 4; ++x) {
     //    for (int y = 0; y < 4; ++y) {
     //        // Change columns to rows
     //        resultTransposedMatrix[x * 4 + y] = resultMatrix[y * 4 + x];
     //    }
     //}

     //// Load the transpose matrix
     //glLoadMatrixf(resultTransposedMatrix);

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
            resultTransposedMatrix[x * 4 + y] = resultMatrix[y * 4 + x];
        }
    }

    // Load the transpose matrix
    glLoadMatrixf(resultTransposedMatrix);

    // Rotate 90 desgress in x-direction
    glRotatef(-90, 1, 0, 0);

    // Scale down!
    glScalef(0.03, 0.03, 0.03);

    // Set the camera with the y-axis pointing up
    //gluLookAt(15, 15, 15, 0, 0, 0, 0, 1, 0);
    //glRotatef(-angle, 0, 1, 0); //press R to rotate
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
    if (action == GLFW_PRESS) {
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
        }
    }

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

    //if (!cap.isOpened()) {
    //    cout << "No webcam, using video file" << endl;
    //    cap.open("MarkerMovie.mp4");
    //    if (cap.isOpened() == false) {
    //        cout << "No video!" << endl;
    //        exit(0);
    //        return -1;
    //    }
    //} else cout << "Found webcam" << endl;

    //cv::namedWindow("test", CV_WINDOW_AUTOSIZE);


    //MarkerTracker* m = new MarkerTracker(0.070,125,125);

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

    // Setup OpenCV
    cv::Mat frame;
    // Get video stream
    initVideoStream(cap);
    // [m]
    const double kMarkerSize = 0.045;
    // Constructor with the marker size (similar to Exercise 5)
    MarkerTracker markerTracker(kMarkerSize);

    manager.controlThread = std::thread(gameManager, std::ref(manager));
    for (auto pGhost : ghosts)
    {
        pGhost->ghostThread = std::thread(ghostAction, std::ref(*pGhost), std::ref(pacmanAgent));
    }

    //pos board
    //float *boardPos;
    float resultMatrix[16];
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {

        // Capture here
        cap >> frame;

        // Make sure that we got a frame -> otherwise crash
        if (frame.empty()) {
            std::cout << "Could not query frame. Trying to reinitialize." << std::endl;
            initVideoStream(cap);
            // Wait for one sec.
            cv::waitKey(1000);
            continue;
        }

        //boardPos = detectMarkers(m);
        //else take old pos
        // Track a marker and get the pose of the marker
        markerTracker.findMarker(frame, resultMatrix);


        // Render here
        display(window, frame, resultMatrix);

        // Swap front and back buffers
        // -> the front buffer is the current in the window rendered frame
        // -> and the back buffer is the newly generated frame buffer
        glfwSwapBuffers(window);

        // Poll for and process events -> Check for inputs: e.g. mouse clicks on the exit button/keyboard gets pressed/etc.
        glfwPollEvents();

    }


    //cv::destroyAllWindows();

    // Free the memory (IMPORTANT to avoid memory leaks!!!)
    glfwTerminate();

    return 0;
}




//float* detectMarkers(MarkerTracker* m){
//    //if a new frame is available:
//    cv::Vec3f boardPos;
//    float distance;
//    float originX = 1.0f, originY = 1.0f; //TODO : makes this In H File
//    float results[5][16];
//
//        if (cap.read(frame)) {
//            bool found[5];
//            cv::Vec2f inputPos;
//
//            //cv::imshow("test", frame);
//
//
//            m->findMarker(frame, results, found, inputPos);
//
//
//            int check = 0;
//            //calculate game board position
//            cv::Vec3f markerPoss[4];
//            
//            for (int i = 0; i < 4;i++) {
//                if (found[i]) {
//                    markerPoss[i] = {results[i][3],results[i][7],results[i][11]};
//                    check++;
//                }
//            }
//
//            if (check >= 3) {
//
//                cv::Vec3f pos1;
//                float dist1;
//                bool pos1Set = false;
//
//                if (found[0] && found[3]) {
//                    pos1 = 0.5f * markerPoss[0] + 0.5f * markerPoss[3];
//                    
//                    pos1Set = true;
//
//                    boardPos = pos1;
//
//                }
//
//
//                cv::Vec3f pos2;
//                float dist2;
//                bool pos2Set = false;
//
//                if (found[1] && found[2]) {
//                    pos1 = 0.5f * markerPoss[0] + 0.5f * markerPoss[3];
//
//                    pos2Set = true;
//
//                    boardPos = pos2;
//                }
//
//                if (check == 4) {
//                    boardPos = 0.5 * pos1 + 0.5 * pos2;
//                }
//
//                //do rotation here
//
//
//                
//
//            }
//
//            const float deadZoneRange = 0.5f;
//            //we calculate the mouvement direction
//            int mouvement; //0 = Right, 1= left, 2=up, 3= down
//            if (found[4]){
//                float Xdistance = inputPos[0]-originX;
//                float Ydistance = inputPos[1]-originY;
//
//                bool isXmovement = (Xdistance > Ydistance);
//                
//                if (isXmovement && Xdistance > deadZoneRange){
//                    mouvement = (Xdistance > 0) ? 0 : 1;
//                } else if (Ydistance > deadZoneRange){
//                    mouvement = (Ydistance > 0) ? 2 : 3;
//                }
//            }
//            
//            
//        }
//
//        return results[1];
//}