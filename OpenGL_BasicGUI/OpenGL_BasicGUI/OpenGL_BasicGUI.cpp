// OpenGL_BasicGUI.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <string>
#include <Windows.h>
#include <math.h>

//Include GLEW
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//OpenGL Headers
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>
#include <GL\glut.h>
#include "shader.hpp"
#include "controls.hpp"


const std::string SL = "..\\OpenGL_BasicGUI\\Shaders\\";


GLFWwindow* window;
int width, height;
float camera_x, camera_y, camera_z;
float hud_length;

static void error_callback(int error, const char* description);
void OpenGLReady3D();
void OpenGLReady3D();
void modelMatrix(GLuint MatrixID);
void OpenGLStart();
void OpenGLSetView();
void OpenGLRender2D();
void OpenGLRender3D();


int _tmain(int argc, _TCHAR* argv[])
{
	OpenGLStart();
	for (int i = 0; i < 500; i++) 
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		OpenGLRender2D();
		OpenGLRender3D();
		
		glfwSwapBuffers(window);

		camera_x = cosf( (float) i / 500.0 * 2 * 3.14 ) * 200;
		camera_y = sinf( (float) i / 500.0 * 2 * 3.14 ) * 200;
		camera_z = sinf( (float) i / 500.0 * 2 * 3.14 ) * 50 + 250;
		hud_length =  (-0.5 * cosf( (float) i / 500.0 * 2 * 3.14 ) + .5 ) * width;
		//Sleep(0);
	}
	return 0;
}

//Define an error callback
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

void OpenGLReady3D()
{
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(45, (float)width / height, 0.1, 5000.0);
	gluLookAt(camera_x, camera_y, camera_z, 0, 0, 0, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
}

void OpenGLReady2D()
{
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0.0f, width, 0.0f, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.375, 0.375, 0.0);
}

void modelMatrix(GLuint MatrixID)
{
	// Model matrix : an identity matrix (model will be at the origin)
	computeMatricesFromInputs();
	glm::mat4 ProjectionMatrix = getProjectionMatrix();
	glm::mat4 ViewMatrix = getViewMatrix();
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
}

void OpenGLStart()
{
	width = 960;
	height = 720;
	camera_z = 200;

	//Set the error callback
	glfwSetErrorCallback(error_callback);

	//Initialize GLFW
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	//Create a window and create its OpenGL context
	window = glfwCreateWindow(width, height, "Test Window", NULL, NULL);

	//If the window couldn't be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		//exit(EXIT_FAILURE);
	}

	//This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(window);

	//Sets the key callback
	// glfwSetKeyCallback(window, key_callback);

	//Initialize GLEW
	GLenum err = glewInit();

	//If GLEW hasn't initialized
	if (err != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return;
	}
	
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	OpenGLSetView();
}

void OpenGLSetView()
{
	GLuint grid = LoadShaders((SL + "SimpleVertexShader.vertexshader").c_str(), (SL + "SimpleFragmentShader.fragmentshader").c_str());
	GLuint MatrixID = glGetUniformLocation(grid, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(grid, "V");
	GLuint ModelMatrixID = glGetUniformLocation(grid, "M");

	modelMatrix(MatrixID);
}

void OpenGLRender3D()
{
	OpenGLReady3D();

	// Draw 3D Scene here
	glColor3f(0.0, 1.0, 1.0);
	glBegin(GL_QUADS);
		glVertex3f(100, -50, 0);
		glVertex3f(100, 50, 0);
		glVertex3f(-100, 50, 0);
		glVertex3f(-100, -50, 0);
	glEnd();
}

void OpenGLRender2D()
{
	OpenGLReady2D();

	// Draw 2D Scene here
	
	glColor3f(1.0, 0.0, 1.0);
	glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(0, 100);
		glVertex2f(hud_length, 100);
		glVertex2f(hud_length, 0);
	glEnd();
	/*
	glColor3f(0.0, 1.0, 0.0);
	std::string words = "shit";
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRasterPos2f(100.0, 0.0);
	GLfloat pos[4] = { 0, 0, 0, 0};
	glGetFloatv(GL_CURRENT_RASTER_POSITION, pos);
	printf("%f: %f: %f: %f\n", pos[0], pos[1], pos[2], pos[3]);

	for (int i = 0; i < 4; i++) {
		glutStrokeCharacter(GLUT_STROKE_ROMAN, words[i]);
	}*/
}