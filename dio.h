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

#ifndef DIO_H
#define DIO_H

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



class Dio : public Hero {
	public:
		
		float carTheta = 0;
		float turnSpeed = 0.02;
		float speed = 0.5;
		float scale = 0.3;
		bool moveForward = false, moveBack = false, turnRight = false, turnLeft = false;
		float wheelTurn = 0;
		glm::vec3 norm = glm::vec3(0,1,0);
		glm::mat4 rot = glm::mat4(1.0f);
		float carPitch = 0;
		float carRoll = 0;

		Dio(){
			pos = glm::vec3(0,0,0);
			dir = glm::vec3(0,0,1);
		}

		Dio(glm::vec3 position, glm::vec3 direction){
			pos = position;
			dir = direction;
		}

		
		void drawClosedCylinder(float height, float radius);
		void drawFront();
		void drawWheel();
		void drawBack();
		void drawConnector();
		void drawRoadRoller();
		void Draw();
		void recompute();
	

};

#endif