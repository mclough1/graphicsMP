/*
 *  CSCI 441, Computer Graphics, Fall 2019
 *
 *  Project: Midterm Project
 *  File: main.cpp
 *
 *	Author: Matthew Clough - Fall 2019
 *
 *  Description:
 *      Contains the abstract class for drawing a hero
 *
 */

#ifndef TORVESTA_H
#define TORVESTA_H

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

using namespace std;



class Torvesta : public Hero {
	public:
		float heroAngle = 0;
		bool walking = false;
		float idle = 0.05;
		int wait = 0;

		float turnSpeed = 0.02;
		float speed = 0.5;
		float scale = 1;
		bool moveForward = false, moveBack = false, turnRight = false, turnLeft = false;
		glm::vec3 norm = glm::vec3(0,1,0);
		glm::mat4 rot = glm::mat4(1.0f);
		float pitch = 0;
		float roll = 0;

		Torvesta(){
			pos = glm::vec3(0,0,0);
			dir = glm::vec3(0,0,1);
		}

		Torvesta(glm::vec3 position, glm::vec3 direction){
			pos = position;
			dir = direction;
		}

		
		void drawLegs();
		void drawBody();
		void drawArms();
		void drawHead();
		void drawHat();
		void drawCharacter();
		void Draw();
		void recompute();
	

};

#endif