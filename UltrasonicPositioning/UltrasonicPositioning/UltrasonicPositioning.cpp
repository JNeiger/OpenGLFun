// UltrasonicPositioning.cpp : Defines the entry point for the console application.
//

/*

Field Coordinate Grid

		|------------------------------------ (w,h)
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
		|									|
---------------------------------------------
		| (0,0)
		|


Robot Coordinate Grid

						| y
						|
						|
						|
						|
						|
						|
						|
-x						|								+x
------------------------------------------------------------
						|
						|
						|
						|
						|
						|
						|
						| -y

*/




#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

struct Pos {
	float x;
	float y;
};

struct Robot {
	float r_right;
	float r_forward;
	float r_left;
	float r_back;

	Pos p_right;
	Pos p_forward;
	Pos p_left;
	Pos p_back;

	Pos location;
};

struct Box {
	float width;
	float height;
};

Box field;
Box boundingBox;
Box difference;

float angle;
Robot me;

Pos getPos(float mag, float angle)
{
	Pos newPos;
	newPos.x = mag * cosf(angle);
	newPos.y = mag * sinf(angle);

	return newPos;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Create constants/input
	field.width  = 50;
	field.height = 100;

	me.r_right   = 50;
	me.r_forward = 50;
	me.r_left    = 0;
	me.r_back    = 50;

	angle = 0;

	// Rotate raw values by [angle]
	me.p_right =   getPos(me.r_right,   angle + 0 * M_PI_2);
	me.p_forward = getPos(me.r_forward, angle + 1 * M_PI_2);
	me.p_left =    getPos(me.r_left,    angle + 2 * M_PI_2);
	me.p_back =    getPos(me.r_back,    angle + 3 * M_PI_2);

	// Calculate bouding box
	Pos positions[] = { me.p_right, me.p_forward, me.p_left, me.p_back };

	float xmin = 0; //Min should always be negative
	float xmax = 0; //Max should always be positive
	float ymin = 0;
	float ymax = 0;

	for (int i = 0; i < 4; i++)
	{
		// Check x bounds
		if (positions[i].x > xmax)
		{
			xmax = positions[i].x;
		}
		else if (positions[i].x < xmin)
		{
			xmin = positions[i].x;
		}

		// Check y bounds
		if (positions[i].y > ymax)
		{
			ymax = positions[i].y;
		}
		else if (positions[i].y < ymin)
		{
			ymin = positions[i].y;
		}
	}

	// Print out everything and do some house keeping

	printf("\n");
	printf("		%f	\n",   me.r_forward);
	printf("%f			%f\n", me.r_left, me.r_right);
	printf("		%f		\n",   me.r_back);
	printf("				Angle: %f\n", angle);
	printf("\n\n");
	printf("Field:\n");
	printf("	Width:  %f\n", field.width);
	printf("	Height: %f\n", field.height);
	printf("\n");

	boundingBox.width  = xmax - xmin;
	boundingBox.height = ymax - ymin;

	printf("Bounding Box is : x %f \t y %f\n", boundingBox.width, boundingBox.height);

	difference.width  = field.width  - boundingBox.width;
	difference.height = field.height - boundingBox.height;

	printf("Difference is   : x %f \t y %f\n", difference.width, difference.height);

	me.location.x = -1 * xmin;
	me.location.y = -1 * ymin;

	printf("Location is     : x %f \t y %f\n", me.location.x, me.location.y);

	return 0;
}

