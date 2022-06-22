// ARPacman.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "pacman.h"
#include "gameboard.h"
#include <GLFW/glfw3.h>
#include "DrawPrimitives.h"


typedef enum { IDLE, UP, DOWN, LEFT, RIGHT } moveFlag;
moveFlag oldPacMoveDir = IDLE, pacMoveDir;

GLfloat angle = 0;

Pacman pacmanAgent;


// Frustum is defined in reshape method
void display(GLFWwindow* window) {
	// Frame buffer consists out of Depth, Color, Stencil -> here we clear the buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Calculate Modelview (the transformation from world coordinate system to camera coordinates) -> Camera object
	glMatrixMode(GL_MODELVIEW);
	// Move to origin
	glLoadIdentity();

	// -> Presettings which will be used for the objects which are created afterwards

	// Move the object backwards
	glTranslatef(0.0, -0.8, -10.0);

	// [degree/sec]
	const float degreePerSec = 90.0f;

	glRotatef(angle, 0, 1, 0);

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

}


/* Program & OpenGL initialization */
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

void reshape(GLFWwindow* window, int width, int height) {
	// Set a whole-window viewport
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	float ratio = width / (float)height;

	// Specifies which matrix stack is the target for subsequent matrix operations
	// -> Three values are accepted: GL_MODELVIEW, GL_PROJECTION, and GL_TEXTURE
	// Calculate projection matrix and define the frustum -> project it on the near plane
	glMatrixMode(GL_PROJECTION);
	// Starting the translation from the origion to avoid a null matrix
	glLoadIdentity();

	// Field of view of the camera
	int fov = 30;
	// Frustum paramters -> area where objects can be rendered
	float near = 0.01f, far = 100.f;
	float top = tan((double)(fov * M_PI / 360.0f)) * near;
	float bottom = -top;
	float left = ratio * bottom;
	float right = ratio * top;

	// The projection matrix will be calculated
	glFrustum(left, right, bottom, top, near, far);
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
	glTranslatef(this->x, 0, this->y);
	drawSphere(0.1, 10, 10);
	glPopMatrix();
}

void keyboardControl(int key)
{
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
	default:
		break;
	}
}


int main(int argc, char* argv[]) {
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

	// Initialize the frustum with the size of the framebuffer
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	reshape(window, width, height);

	// Initialize the GL library
	// -> Give app arguments for configuration, e.g. depth or color should be activated or not
	init(argc, argv);

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
