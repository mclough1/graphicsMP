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


#ifndef HERO_H
#define HERO_H

class Hero {
	public:
		glm::vec3 dir;
		glm::vec3 pos;

		virtual void Draw();
		virtual void recompute();

};

#endif
