#ifndef HANK_H
#define HANK_H

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

class HankHill : public Hero {
	public:
		
		float teapotScale = 1.0f, teapotScaleFactor = .01f;

		HankHill(){
			pos = glm::vec3(0,0,0);
			dir = glm::vec3(0,0,1);
		}

		HankHill(glm::vec3 position, glm::vec3 direction){
			pos = position;
			dir = direction;
		}

		void drawWheel();
		void drawWheels();
		void drawFront();
		void drawCar();

		void Draw();
		void recompute();
		

		
	
};

#endif