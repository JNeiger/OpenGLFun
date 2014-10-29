#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

//Include GLEW
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//OpenGL Headers
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/transform.hpp>
#include <shader.cpp>
#include <controls.cpp>

//Kinect Headers
#include <iostream>
#include <NuiApi.h>
#include <NuiImageCamera.h>
#define _USE_MATH_DEFINES
#include <math.h>

GLFWwindow* window;

float depthLookUp[2048];

int width = 320;
int height = 240;

float theta; //pitch
float fi;    //yaw
float psi;   //roll

glm::mat3x3 rotation;
glm::vec3 translation;
float scalar;

//Define an error callback
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

//Define the key input callback
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void createGrid()
{
	float color[] = { 0, 0, 0 };
	float depth = -0.5f;
	float min = -100.0f;
	float max = 100.0f;

	glLineWidth(0.05f);
	glBegin(GL_LINES);
	glColor3fv(color);
	for (float i = min; i <= max; i += 1)
	{
		glVertex3f(min, depth, i);
		glVertex3f(max, depth, i);
		glVertex3f(i, depth, max);
		glVertex3f(i, depth, min);
	}
	glEnd();
}

void createCube(float l, glm::vec3 vec)
{
	/*
	glPointSize(5.0f);
	glBegin(GL_POINTS);
	glVertex3f(vec.x, vec.y, vec.z);
	glEnd();*/
	glBegin(GL_POLYGON);
	glVertex3f(vec.x + l, vec.y + -l, vec.z + l);
	glVertex3f(vec.x + l, vec.y + l, vec.z + l);
	glVertex3f(vec.x + -l, vec.y + l, vec.z + l);
	glVertex3f(vec.x + -l, vec.y + -l, vec.z + l);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(vec.x + -l, vec.y + -l, vec.z + -l);
	glVertex3f(vec.x + -l, vec.y + -l, vec.z + l);
	glVertex3f(vec.x + -l, vec.y + l, vec.z + l);
	glVertex3f(vec.x + -l, vec.y + l, vec.z + -l);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(vec.x + l, vec.y + -l, vec.z + -l);
	glVertex3f(vec.x + l, vec.y + l, vec.z + -l);
	glVertex3f(vec.x + l, vec.y + l, vec.z + l);
	glVertex3f(vec.x + l, vec.y + -l, vec.z + l);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(vec.x + l, vec.y + l, vec.z + l);
	glVertex3f(vec.x + l, vec.y + l, vec.z + -l);
	glVertex3f(vec.x + -l, vec.y + l, vec.z + -l);
	glVertex3f(vec.x + -l, vec.y + l, vec.z + l);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(vec.x + l, vec.y + -l, vec.z + l);
	glVertex3f(vec.x + -l, vec.y + -l, vec.z + l);
	glVertex3f(vec.x + -l, vec.y + -l, vec.z + -l);
	glVertex3f(vec.x + l, vec.y + -l, vec.z + -l);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(vec.x + -l, vec.y + -l, vec.z + -l);
	glVertex3f(vec.x + -l, vec.y + l, vec.z + -l);
	glVertex3f(vec.x + l, vec.y + l, vec.z + -l);
	glVertex3f(vec.x + l, vec.y + -l, vec.z + -l);
	glEnd();
}

void createObject(GLuint buffer, GLenum mode, int vertexAmount)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	glDrawArrays(mode, 0, vertexAmount);
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

glm::vec3 realWorld(float depth, int lineNumber, int pixelNumber, float scale, float decimalPlace) {
	float depthWidthHalf = width / 2.0f;
	float depthHeightHalf = height / 2.0f;
	float depthHFOV = 57.0f;
	float depthVFOV = 43.0f;
	float depthH = tan((depthHFOV / 2.0f) * (M_PI / 180.0f));
	float depthV = tan((depthVFOV / 2.0f) * (M_PI / 180.0f));

	glm::vec3 position;
	position.x = depth * depthH * (pixelNumber / depthWidthHalf) / -scale;
	position.y = depth * depthV * (lineNumber / depthHeightHalf) / scale;
	position.z = depth / -scale;

	//position.x = round(position.x * decimalPlace) / decimalPlace;
	//position.y = round(position.y * decimalPlace) / decimalPlace;
	//position.z = round(position.z * decimalPlace) / decimalPlace;

	return rotation * position;
}

glm::mat3x3 getRotationMatrix(float pitch, float roll, float yaw)
{
	return{
		cosf(pitch) * cosf(roll), cosf(pitch) * sinf(roll), -sinf(pitch),
		-cosf(yaw) * sinf(roll) + sinf(yaw) * sinf(pitch) * cosf(roll), cosf(yaw) * cosf(roll) + sinf(yaw) * sinf(pitch) * sinf(roll), sinf(yaw) * cosf(pitch),
		sinf(yaw) * sinf(roll) + cosf(yaw) * sinf(pitch) * cosf(roll), -sinf(yaw) * cosf(roll) + cosf(yaw) * sinf(pitch) * sinf(roll), cosf(yaw) * cosf(pitch)
	};
}

glm::vec3 convertToVec3AndScale(Vector4 in)
{
	glm::vec3 loc = glm::vec3(in.x, in.y, in.z);
	loc = scalar * loc;
	//loc = rotation * loc;
	loc = translation + loc;

	return loc;
}

int main(void)
{
	//Set the error callback
	glfwSetErrorCallback(error_callback);

	//Initialize GLFW
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	//Set the GLFW window creation hints - these are optional
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Request a specific OpenGL version
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Request a specific OpenGL version
	//glfwWindowHint(GLFW_SAMPLES, 4); //Request 4x antialiasing
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//Create a window and create its OpenGL context
	window = glfwCreateWindow(960, 720, "Test Window", NULL, NULL);

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
	glfwSetKeyCallback(window, key_callback);

	//Initialize GLEW
	GLenum err = glewInit();

	//If GLEW hasn't initialized
	if (err != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}

	//Set a background color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	//GLuint red = LoadShaders("SimpleTransform.vertexshader", "SingleColorRed.fragmentshader");
	//GLuint grid = LoadShaders("SimpleTransform.vertexshader", "SingleColorGrid.fragmentshader");
	GLuint grid = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	//glBindFragDataLocation(red, 0, "red");
	glBindFragDataLocation(grid, 0, "grid");
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(grid, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(grid, "V");
	GLuint ModelMatrixID = glGetUniformLocation(grid, "M");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);


	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	static const GLushort g_element_buffer_data[] = { 0, 1, 2 };

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	static const GLfloat g_triangle_buffer_data[] = {
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		0.0f, 1.0f, -1.0f,
	};

	GLuint triangle;
	glGenBuffers(1, &triangle);
	glBindBuffer(GL_ARRAY_BUFFER, triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_triangle_buffer_data), g_triangle_buffer_data, GL_STATIC_DRAW);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_SMOOTH);//OPENGL INSTANTIATION
	HRESULT hr;
	NUI_IMAGE_FRAME depthFrame;
	HANDLE hDepth;
	INuiSensor* pNuiSensor = NULL;
	int iSensorCount = 0;
	hr = NuiGetSensorCount(&iSensorCount);

	if (FAILED(hr))
		return hr;

	for (int i = 0; i < iSensorCount; i++)
	{
		INuiSensor* tempSensor;
		hr = NuiCreateSensorByIndex(i, &tempSensor);

		if (FAILED(hr))
			continue;

		hr = tempSensor->NuiStatus();
		if (S_OK == hr)
		{
			pNuiSensor = tempSensor;
			break;
		}

		tempSensor->Release();
	}

	if (!pNuiSensor)
		return -1;

	theta = 180.0f / 57.2957795;  //pitch
	fi = 0.0f / 57.2957795;     //yaw
	psi = 0.0f / 57.2957795;    //roll
	rotation = getRotationMatrix(theta, psi, fi);
	translation = glm::vec3(0.0f, 2.0f, 0.0f);
	scalar = 2.0f;

	pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH);
	pNuiSensor->NuiImageStreamOpen(
		NUI_IMAGE_TYPE_DEPTH,
		NUI_IMAGE_RESOLUTION_320x240,
		0,
		2,
		NULL,
		&hDepth);//KINECT INSTANTIATION

	cout << "Starting Main Loop";

	static double lastTime = glfwGetTime();
	//Main Loop
	do
	{
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);
		//Clear color buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(grid);
		modelMatrix(MatrixID);


		hr = pNuiSensor->NuiImageStreamGetNextFrame(hDepth, 0, &depthFrame);
		if (!FAILED(hr))
		{

			INuiFrameTexture* pTexture;
			NUI_LOCKED_RECT LockedRect;

			hr = pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
				hDepth, &depthFrame, false, &pTexture);

			if (FAILED(hr))
			{
				pNuiSensor->NuiImageStreamReleaseFrame(hDepth, &depthFrame);
				continue;
			}

			pTexture->LockRect(0, &LockedRect, NULL, 0);//Kinect Image Grab
			int skipX = 1;
			int skipY = 1;

			if (LockedRect.Pitch != 0)
			{
				for (int x = 0; x < width; x += skipX)
				{
					for (int y = 0; y < height; y += skipY)
					{
						const NUI_DEPTH_IMAGE_PIXEL * pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits) + x + y * width;
						
						Vector4 locationDepth = NuiTransformDepthImageToSkeleton(x, y, (short)(pBufferRun->depth << 3));
						createCube(0.005f, convertToVec3AndScale(locationDepth));
					}
				}
			}

			pTexture->UnlockRect(0);
			pTexture->Release();

			pNuiSensor->NuiImageStreamReleaseFrame(hDepth, &depthFrame);
		}

		createGrid();

		//Test drawings
		/*
		glUseProgram(red);
		modelMatrix(MatrixID);
		//createCube(0.05f, glm::vec3(1.0f,1.0f,1.0f));
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		//createObject(vertexbuffer, GL_TRIANGLES, 3);
		//createObject(triangle, GL_TRIANGLES, 3);
		glDisableVertexAttribArray(0);
		*/

		//Swap buffers
		glfwSwapBuffers(window);
		//Get and organize events, like keyboard and mouse input, window resizing, etc...
		glfwPollEvents();

		std::string title = "Title | FPS " + std::to_string(1.0f/deltaTime);
		const char* pszConstString = title.c_str();
		glfwSetWindowTitle(window, pszConstString);

		lastTime = currentTime;
	} //Check if the ESC key had been pressed or if the window had been closed
	while (!glfwWindowShouldClose(window));


	//Close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	//Finalize and clean up GLFW
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

