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
#include "dio.h"

using namespace std;




// drawClosedCylinder() //////////////////////////////////////////////////////////////////
//
//  Function to draw a cylinder with closed ends
//
////////////////////////////////////////////////////////////////////////////////
void Dio::drawClosedCylinder(float height, float radius){
	CSCI441::drawSolidCylinder(radius, radius, height, 10, 10);
	glm::mat4 EndsMtx = glm::rotate(glm::mat4(1.0f), (float)M_PI/2, glm::vec3(1.0f, 0.0f, 0.0f));
	glMultMatrixf( &EndsMtx[0][0] ); {
		CSCI441::drawSolidDisk(0, radius, 10, 10);
		glm::mat4 otherEnd = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, 0.0, -height) );
		glMultMatrixf( &otherEnd[0][0] ); {
			CSCI441::drawSolidDisk(0, radius, 10, 10);
		}; glMultMatrixf( &(glm::inverse( otherEnd ))[0][0] );
	}; glMultMatrixf( &(glm::inverse( EndsMtx ))[0][0] );
}

// drawFront() //////////////////////////////////////////////////////////////////
//
//  Function to draw the front half of the road roller with the roller part
//
////////////////////////////////////////////////////////////////////////////////
void Dio::drawFront(){

	//box of the front
	glColor3f(1,1,0.5);
	glm::mat4 boxMtx = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, 0.0, 4.5) );
	boxMtx = glm::scale( boxMtx, glm::vec3( 16.0f, 5.0f, 7.0f ) );
	glMultMatrixf( &boxMtx[0][0] ); {
		CSCI441::drawSolidCube(1);
	}; glMultMatrixf( &(glm::inverse( boxMtx ))[0][0] );

	//cylinder, 'roll'
	glColor3f(0.5, 0.5, 0.5);
	glm::mat4 rollerMtx = glm::translate( glm::mat4(1.0f), glm::vec3(7.0, -1.0, 7.0) );
	rollerMtx = glm::rotate(rollerMtx, (float)M_PI/2, glm::vec3(0.0f, 0.0f, 1.0f));
	rollerMtx = glm::rotate(rollerMtx, -wheelTurn, glm::vec3(0.0f, 1.0f, 0.0f)); //rotate according to wheels rotation
	glMultMatrixf( &rollerMtx[0][0] ); {
		drawClosedCylinder(14,3);
	}; glMultMatrixf( &(glm::inverse( rollerMtx ))[0][0] );

}

// drawWheel() //////////////////////////////////////////////////////////////////
//
//  Function to draw a wheel
//
////////////////////////////////////////////////////////////////////////////////
void Dio::drawWheel(){
	glm::mat4 rotMtx = glm::rotate(glm::mat4(1.0f), (float)M_PI/2, glm::vec3(1.0f, 0.0f, 0.0f));
	glMultMatrixf( &rotMtx[0][0] ); {

		// draw tire part
		glm::mat4 tire = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, -1.25, 0.0) );
		tire = glm::rotate(tire, wheelTurn, glm::vec3(0.0f, 1.0f, 0.0f));
		glMultMatrixf( &tire[0][0] ); {
			glColor3f(0, 0, 0);
			drawClosedCylinder(2.5,2);
		}; glMultMatrixf( &(glm::inverse( tire ))[0][0] );

		// draw inner metal wheel part
		glm::mat4 wheel = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, -1.3, 0.0) );
		wheel = glm::rotate(wheel, wheelTurn, glm::vec3(0.0f, 1.0f, 0.0f));
		glMultMatrixf( &wheel[0][0] ); {
			glColor3f(0.7, 0.7, 0.7);
			drawClosedCylinder(2.6,1);
		}; glMultMatrixf( &(glm::inverse( wheel ))[0][0] );

	}; glMultMatrixf( &(glm::inverse( rotMtx ))[0][0] );
}

// drawBack() //////////////////////////////////////////////////////////////////
//
//  Function to draw the back half of the road roller with 2 tire wheels
//
////////////////////////////////////////////////////////////////////////////////
void Dio::drawBack(){

	//body of the back part
	glColor3f(1,1,0.5);
	glm::mat4 boxMtx = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, 1.0, 3.5) );
	boxMtx = glm::scale( boxMtx, glm::vec3( 16.0f, 6.0f, 3.0f ) );
	glMultMatrixf( &boxMtx[0][0] ); {
		CSCI441::drawSolidCube(1);
	}; glMultMatrixf( &(glm::inverse( boxMtx ))[0][0] );

	boxMtx = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, 1.0, -2.5) );
	boxMtx = glm::scale( boxMtx, glm::vec3( 16.0f, 6.0f, 1.0f ) );
	glMultMatrixf( &boxMtx[0][0] ); {
		CSCI441::drawSolidCube(1);
	}; glMultMatrixf( &(glm::inverse( boxMtx ))[0][0] );

	boxMtx = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, 1.8, 0.0) );
	boxMtx = glm::scale( boxMtx, glm::vec3( 16.0f, 4.0f, 4.0f ) );
	glMultMatrixf( &boxMtx[0][0] ); {
		CSCI441::drawSolidCube(1);
	}; glMultMatrixf( &(glm::inverse( boxMtx ))[0][0] );

	boxMtx = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, -1.0, 0.0) );
	boxMtx = glm::scale( boxMtx, glm::vec3( 10.0f, 2.0f, 4.0f ) );
	glMultMatrixf( &boxMtx[0][0] ); {
		CSCI441::drawSolidCube(1);
	}; glMultMatrixf( &(glm::inverse( boxMtx ))[0][0] );

	//the two wheels, also turn according to the wheels rotation
	glm::mat4 wheelMtx = glm::translate( glm::mat4(1.0f), glm::vec3(6.5, -2.0, 0.0) );
	wheelMtx = glm::rotate(wheelMtx, (float)M_PI/2, glm::vec3(0.0f, 1.0f, 0.0f));
	glMultMatrixf( &wheelMtx[0][0] ); {
		drawWheel();
	}; glMultMatrixf( &(glm::inverse( wheelMtx ))[0][0] );

	wheelMtx = glm::translate( glm::mat4(1.0f), glm::vec3(-6.5, -2.0, 0.0) );
	wheelMtx = glm::rotate(wheelMtx, (float)M_PI/2, glm::vec3(0.0f, 1.0f, 0.0f));
	glMultMatrixf( &wheelMtx[0][0] ); {
		drawWheel();
	}; glMultMatrixf( &(glm::inverse( wheelMtx ))[0][0] );
	
	
}

// drawConnector() //////////////////////////////////////////////////////////////////
//
//  Function to draw the connector of the front and back halves
//
////////////////////////////////////////////////////////////////////////////////
void Dio::drawConnector(){
	glColor3f(0, 0, 0);
	glm::mat4 boxMtx = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, 0.0, -1.0) );
	boxMtx = glm::scale( boxMtx, glm::vec3( 4.0f, 2.0f, 6.0f ) );
	glMultMatrixf( &boxMtx[0][0] ); {
		CSCI441::drawSolidCube(1);
	}; glMultMatrixf( &(glm::inverse( boxMtx ))[0][0] );
}


// drawConnector() //////////////////////////////////////////////////////////////////
//
//  Function to draw the whole road roller
//
////////////////////////////////////////////////////////////////////////////////
void Dio::drawRoadRoller(){
	
	drawFront();
	
	glm::mat4 backMtx = glm::translate( glm::mat4(1.0f), glm::vec3(0.0, 0.0, -8.0) );
	glMultMatrixf( &backMtx[0][0] ); {
		drawBack();
	}; glMultMatrixf( &(glm::inverse( backMtx ))[0][0] );

	drawConnector();

}





void Dio::Draw(){
	
	glm::mat4 carTrans = glm::translate( glm::mat4(1.0f), glm::vec3(pos.x, 4*scale+pos.y, pos.z ));
	glMultMatrixf( &carTrans[0][0] ); {
		glMultMatrixf( &rot[0][0] ); {
			glm::mat4 carMtx = glm::rotate(glm::mat4(1.0f), carTheta, norm);
			carMtx = glm::scale( carMtx, glm::vec3( scale, scale, scale ) );
			glMultMatrixf( &carMtx[0][0] ); {
			
			
				drawRoadRoller();
			}; glMultMatrixf( &(glm::inverse( carMtx ))[0][0] );
		}; glMultMatrixf( &(glm::inverse( rot ))[0][0] );
	}; glMultMatrixf( &(glm::inverse( carTrans ))[0][0] );
}


void Dio::recompute(){
	

	// flip the rotation direction if going backwards since thats how cars work
	if(turnRight){
		if(moveBack&&!moveForward){
			carTheta+=turnSpeed;
		}else{
			carTheta-=turnSpeed;
			
		}
	}
	if(turnLeft){
		if(moveBack&&!moveForward){
			carTheta-=turnSpeed;
		}else{
			carTheta+=turnSpeed;
		}
	}

	dir = glm::vec3(sinf(carTheta), 0, cosf(carTheta));

	if(moveForward){
		pos+=dir*speed;
		wheelTurn+=0.06;
	}
	if(moveBack){
		pos-=dir*speed;
		wheelTurn-=0.06;
	}
	//keep road roller within bounds of grid
	if(pos.x<-50){
		pos.x = -50;
	}else if(pos.x>50){
		pos.x = 50;
	}
	if(pos.z<-50){
		pos.z = -50;
	}else if(pos.z>50){
		pos.z = 50;
	}
	
}