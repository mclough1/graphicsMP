// include the OpenGL library header
#ifdef __APPLE__					// if compiling on Mac OS
	#include <OpenGL/gl.h>
#else										// if compiling on Linux or Windows OS
	#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>	// include GLFW framework header

#include <CSCI441/objects.hpp> // for our 3D objects

// include GLM libraries and matrix functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>				// for cos(), sin() functionality
#include <stdio.h>			// for printf functionality
#include <stdlib.h>			// for exit functionality
#include <time.h>			  // for time() functionality
#include <iostream>
#include "hero.h"
#include "torvesta.h"

using namespace std;

void Torvesta::drawLegs() {
	//draw character's legs
	glm::mat4 transMtx1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.1, 0, 0));
	glMultMatrixf( &transMtx1[0][0] );
	glColor3f(0.2,0.2,0.2);
	CSCI441::drawSolidCone(1, 3, 10, 10);
	glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );

	transMtx1 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.1, 0, 0));
	glMultMatrixf( &transMtx1[0][0] );
	glColor3f(0.2,0.2,0.2);
	CSCI441::drawSolidCone(1, 3, 10, 10);
	glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );
}

void Torvesta::drawBody() {
	//draw character's body

	glm::mat4 transMtx1 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 1.5, 0));
	glMultMatrixf( &transMtx1[0][0] );
	glColor3f(0.1,0.1,0.1);
	CSCI441::drawSolidCylinder(0.4,.6,1.5,10,10);
	glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );
	//neck
	transMtx1 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 3, 0));
	glMultMatrixf( &transMtx1[0][0] );
	glColor3f(0.1,0.1,0.1);
	CSCI441::drawSolidCylinder(.6,.4,0.2,10,10);
	glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );
}

void Torvesta::drawArms() {
	//draw character's arms
	glm::mat4 transMtx1 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.25, 3, 0));
	glm::mat4 rotMtx = glm::rotate(glm::mat4(1.0f), 2.65f, glm::vec3(0, 0, 1));
	glMultMatrixf( &transMtx1[0][0] );
	glMultMatrixf( &rotMtx[0][0] );
	glColor3f(0.2,0.2,0.2);
	CSCI441::drawSolidCylinder(.1, .4, 2, 10, 10);
	glMultMatrixf( &( glm::inverse( rotMtx ) )[0][0] );
	glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );

	transMtx1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.25, 3, 0));
	rotMtx = glm::rotate(glm::mat4(1.0f), -2.65f, glm::vec3(0, 0, 1));
	glMultMatrixf( &transMtx1[0][0] );
	glMultMatrixf( &rotMtx[0][0] );
	glColor3f(0.2,0.2,0.2);
	CSCI441::drawSolidCylinder(.1, .4, 2, 10, 10);
	glMultMatrixf( &( glm::inverse( rotMtx ) )[0][0] );
	glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );
}

void Torvesta::drawHead() {
	//draw character's head
	float radius = 0.5;
	glm::mat4 transMtx1 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 3.2 + radius - 0.1, 0));
	glMultMatrixf( &transMtx1[0][0] );
	glColor3f(0.89,0.86,0.78);
	CSCI441::drawSolidSphere(radius,10,10);
	glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );
}

void Torvesta::drawHat() {
	//draw character's helmet
	//bottom
	glm::mat4 transMtx1 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 3.6 + 0.5 - 0.25, 0));
	glMultMatrixf( &transMtx1[0][0] );
	glColor3f(0.8,0.8,0.8);
	CSCI441::drawSolidCylinder(0.6,0.2,0.4,10,10);
	glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );
	//top
	transMtx1 = glm::translate(glm::mat4(1.0f), glm::vec3(0, 3.6 + 0.5 - 0.15, 0));
	glMultMatrixf( &transMtx1[0][0] );
	glColor3f(0.8,0.8,0.8);
	CSCI441::drawSolidSphere(0.4, 10, 10);
	glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );
}

void Torvesta::drawCharacter() {
	//draw full character
	drawLegs();

	glm::mat4 transMtx1 = glm::translate(glm::mat4(1.0f), glm::vec3(0,idle - abs(idle), 0));
	if (walking) {
		glMultMatrixf( &transMtx1[0][0] );
	}

	drawBody();
	drawArms();
	drawHead();
	drawHat();

	if (walking) {
		glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );

		wait++; //animation waits a few frames to change
		if (wait >= 10) {
			wait = 0;
			idle = idle * -1;
		}
	}
}

void Torvesta::draw() {
	glm::mat4 rotMtx = glm::rotate(glm::mat4(1.0f), heroAngle, glm::vec3(0, 1, 0));
	glm::mat4 transMtx1 = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, pos.z));
	glMultMatrixf( &transMtx1[0][0] );
	glMultMatrixf( &rotMtx[0][0] );
	drawCharacter();
	glMultMatrixf( &( glm::inverse( rotMtx ) )[0][0] );
	glMultMatrixf( &( glm::inverse( transMtx1 ) )[0][0] );
}