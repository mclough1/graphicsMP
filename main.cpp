/*
 *  CSCI 441, Computer Graphics, Fall 2019
 *
 *  Project: a3
 *  File: main.cpp
 *
 *	Author: Trevor Kerr - Fall 2019
 *
 *  Description:
 *      Drive around the world in Hank Hill's car
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
#include <glm/gtc/type_ptr.hpp>

#include <math.h>				// for cos(), sin() functionality
#include <stdio.h>			// for printf functionality
#include <stdlib.h>			// for exit functionality
#include <time.h>			  // for time() functionality
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "hero.h"
#include "dio.h"
#include "HankHill.h"
#include "torvesta.h"

//*************************************************************************************
//
// Global Parameters



// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int windowWidth = 640, windowHeight = 480;

int leftMouseButton;    	 						// status of the mouse button
glm::vec2 mousePos;			              		  	// last known X and Y of the mouse

glm::vec3 arcCamPos;            						// camera position in cartesian coordinates
float arcCameraTheta, arcCameraPhi;               		// camera DIRECTION in spherical coordinates
float cameraP, zoom;								// camera Distance from object of interest

glm::vec3 freeCamPos;            						// camera position in cartesian coordinates
float freeCameraTheta, freeCameraPhi;               		// camera DIRECTION in spherical coordinates
glm::vec3 freeCamDir; 			                    	// camera DIRECTION in cartesian coordinates
float freeCamSpeed;
glm::vec3 freeCamLookAt;

Hero* heroFocus;
glm::vec3* lookAt;
glm::vec3* camPos;

bool freeCamOn = false;
bool firstPersonOn = false;			                    	// camera DIRECTION in cartesian coordinates

bool zoomOn = false;								// true if zoom mode is on

vector<vector<glm::vec3>> controlSurfacePoints;

vector<glm::vec3> controlCurvePoints;

float dioDist = 0.0;
float dioSpeed = 0.1;
float hankDist = 0.0;
float hankSpeed = 0.01;

vector<float> bezDists, bezTimes;

GLuint environmentDL;                       		// display list for the 'city'
GLuint objectsDL;

Dio dio;
HankHill hank;
Torvesta torvesta;

Hero* heroes[] = {&torvesta, &dio, &hank};


//*************************************************************************************
//
// Helper Functions

// getRand() ///////////////////////////////////////////////////////////////////
//
//  Simple helper function to return a random number between 0.0f and 1.0f.
//
////////////////////////////////////////////////////////////////////////////////

bool loadFile(std::string filename) {
    std::ifstream in = std::ifstream(filename);
    int numPoints, numPatches, numObjects;
    char comma; //trash commas

    if (in) {
        in >> numPatches;
        controlSurfacePoints.resize(numPatches);
        for(int i = 0; i < numPatches; i++){
            for(int j = 0; j < 16; j++){
                glm::vec3 point;
                if(in >> point.x >> comma >> point.y >> comma >> point.z) {
					cout<<point.x<<" "<<point.y<<" "<<point.z<<endl;
                    controlSurfacePoints[i].push_back(point);
                } else {
                    return false;
                }
            }
        }

        in >> numPoints;
        for(int i = 0; i < numPoints; i++){
            glm::vec3 point;
            if(in >> point.x >> comma >> point.y >> comma >> point.z) {
                controlCurvePoints.push_back(point);
            } else {
                return false;
            }
        }

        in >> numObjects;
        objectsDL = glGenLists(1);
        glNewList(objectsDL, GL_COMPILE);
        for(int i = 0; i < numObjects; i++){
            std::string type;
            glm::vec3 pos;
            int size;
            in >> type >> pos.x >> comma >> pos.y >> comma >> pos.z >>  size;

            glm::mat4 transMtx = glm::translate(glm::mat4(1.0f), pos);
            glMultMatrixf( &transMtx[0][0] );

            if (type.compare("CUBE")) {
				cout<<"cube"<<endl;
                CSCI441::drawSolidCube(size);
            } else if (type.compare("SPHERE")) {
				cout<<"sphere"<<endl;
                CSCI441::drawSolidSphere(size, 50, 50);
            }

            glMultMatrixf( &( glm::inverse( transMtx ) )[0][0] );


        }
        glEndList();
		return true;


    } else {
        return false;
    }
}

float getRand() { return rand() / (float)RAND_MAX; }

// evaluateBezierCurve() //////////////////////////////////////////////////////////
//
//	Returns a point on the bezier curve
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 evaluateBezierCurve( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t ) {
	return (float)pow(1-t, 3) * p0 + 3.0f * (float)pow(1-t, 2) * t * p1 + 3.0f * (1-t) * (float)pow(t,2) * p2 + (float)pow(t, 3) * p3;
}

// tangentBezierCurve() //////////////////////////////////////////////////////////
//
//	Returns the tangent at a point on the curve
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 tangentBezierCurve( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t ) {
    return normalize(-3.0f * (float)pow(1-t, 2) * p0 + 3.0f * (float)pow(1-t, 2) * p1 - 6.0f * t * (1-t) * p1 - 3.0f * (float)pow(t, 2) * p2 + 6.0f * t * (1-t) * p2 + 3.0f * (float)pow(t, 2) * p3);
}

void loadDt(){
	glm::vec3 point = controlCurvePoints[0];
	glm::vec3 prevpoint = point;
	float distSum = 0;
	// for each curve and then each segment of curve generate a poin to put in our chart
	//chart is made up of times, which each curve spans a time of 1 (unit) and distances from the original point
	for( int i = 0; (i+3)<(int)controlCurvePoints.size(); i+=3){
		for(float t=0.0; t<1; t += 1.0/20){
			point = evaluateBezierCurve(controlCurvePoints[i], controlCurvePoints[i+1], controlCurvePoints[i+2], controlCurvePoints[i+3], t);
			distSum += distance(point, prevpoint);
			bezDists.push_back(distSum);
			bezTimes.push_back(t+i/3);
			prevpoint = point;
		}
	}
}

// getRotMatrix() //////////////////////////////////////////////////////////
//
//	Returns the rotation matrix between two vectors
//
////////////////////////////////////////////////////////////////////////////////
glm::mat4 getRotMatrix(glm::vec3 from, glm::vec3 to) {
    glm::vec3 axis = glm::normalize(glm::cross(from, to));

    float angle = acos(glm::dot(normalize(from), normalize(to))); //this is wrong
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, axis);

    return rot;
}

// recomputeOrientation() //////////////////////////////////////////////////////
//
// This function updates the camera's direction in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraTheta or cameraPhi is updated.
//
////////////////////////////////////////////////////////////////////////////////
void recomputeOrientation() {
	
	if(freeCamOn){
		float r = sinf(freeCameraPhi);
		freeCamDir = glm::vec3(1*r*sinf(freeCameraTheta), -1*cosf(freeCameraPhi), -1*r*cosf(freeCameraTheta));

		freeCamDir = normalize(freeCamDir);
		freeCamLookAt = freeCamPos+freeCamDir;
	}else{
		float r = sinf(arcCameraPhi);
		arcCamPos = glm::vec3(1*r*sinf(arcCameraTheta), cosf(arcCameraPhi), -1*r*cosf(arcCameraTheta));
		arcCamPos = cameraP*normalize(arcCamPos) + heroFocus->pos;
	}
}



void recomputeCarOrientation() {

	torvesta.dir = glm::vec3(sinf(torvesta.heroAngle), 0, cosf(torvesta.heroAngle));

	float d = 0.01;
	float x = (torvesta.pos.x+50)/100;
	float z = (torvesta.pos.z+50)/100;
	glm::vec3 zneg = (evaluateBezierCurve(
						evaluateBezierCurve(controlSurfacePoints[0][0], controlSurfacePoints[0][1], controlSurfacePoints[0][2], controlSurfacePoints[0][3], x),
						evaluateBezierCurve(controlSurfacePoints[0][4], controlSurfacePoints[0][5], controlSurfacePoints[0][6], controlSurfacePoints[0][7], x),
						evaluateBezierCurve(controlSurfacePoints[0][8], controlSurfacePoints[0][9], controlSurfacePoints[0][10], controlSurfacePoints[0][11], x),
						evaluateBezierCurve(controlSurfacePoints[0][12], controlSurfacePoints[0][13], controlSurfacePoints[0][14], controlSurfacePoints[0][15], x),
						z-d));
	glm::vec3 zpos = (evaluateBezierCurve(
						evaluateBezierCurve(controlSurfacePoints[0][0], controlSurfacePoints[0][1], controlSurfacePoints[0][2], controlSurfacePoints[0][3], x),
						evaluateBezierCurve(controlSurfacePoints[0][4], controlSurfacePoints[0][5], controlSurfacePoints[0][6], controlSurfacePoints[0][7], x),
						evaluateBezierCurve(controlSurfacePoints[0][8], controlSurfacePoints[0][9], controlSurfacePoints[0][10], controlSurfacePoints[0][11], x),
						evaluateBezierCurve(controlSurfacePoints[0][12], controlSurfacePoints[0][13], controlSurfacePoints[0][14], controlSurfacePoints[0][15], x),
						z+d));
	glm::vec3 xneg = (evaluateBezierCurve(
						evaluateBezierCurve(controlSurfacePoints[0][0], controlSurfacePoints[0][1], controlSurfacePoints[0][2], controlSurfacePoints[0][3], x-d),
						evaluateBezierCurve(controlSurfacePoints[0][4], controlSurfacePoints[0][5], controlSurfacePoints[0][6], controlSurfacePoints[0][7], x-d),
						evaluateBezierCurve(controlSurfacePoints[0][8], controlSurfacePoints[0][9], controlSurfacePoints[0][10], controlSurfacePoints[0][11], x-d),
						evaluateBezierCurve(controlSurfacePoints[0][12], controlSurfacePoints[0][13], controlSurfacePoints[0][14], controlSurfacePoints[0][15], x-d),
						z));
	glm::vec3 xpos = (evaluateBezierCurve(
						evaluateBezierCurve(controlSurfacePoints[0][0], controlSurfacePoints[0][1], controlSurfacePoints[0][2], controlSurfacePoints[0][3], x+d),
						evaluateBezierCurve(controlSurfacePoints[0][4], controlSurfacePoints[0][5], controlSurfacePoints[0][6], controlSurfacePoints[0][7], x+d),
						evaluateBezierCurve(controlSurfacePoints[0][8], controlSurfacePoints[0][9], controlSurfacePoints[0][10], controlSurfacePoints[0][11], x+d),
						evaluateBezierCurve(controlSurfacePoints[0][12], controlSurfacePoints[0][13], controlSurfacePoints[0][14], controlSurfacePoints[0][15], x+d),
						z));



	glm::vec3 zvec = normalize(zpos-zneg);
	glm::vec3 xvec = normalize(xpos-xneg);
	
	torvesta.pitch = acos(glm::dot(normalize(zvec), glm::vec3(0,0,1)));
	if(zvec.y >0){
		torvesta.pitch = - torvesta.pitch;
	}
	torvesta.roll = -acos(glm::dot(normalize(xvec), glm::vec3(1,0,0)));
	
	if(xvec.y >0){
		torvesta.roll = - torvesta.roll;
	}
	torvesta.rot = glm::rotate(glm::mat4(1.0f), torvesta.roll, glm::vec3(0,0,1));
	torvesta.rot = glm::rotate(torvesta.rot, torvesta.pitch, glm::vec3(1,0,0));
}

// recomputeSpriteLocation() //////////////////////////////////////////////////////
//
// This function updates the sprite's location on the loaded bezier curve
//
////////////////////////////////////////////////////////////////////////////////
void recomputeDioLocation(){
	dioDist += dioSpeed;
	dioDist = fmod(dioDist, bezDists.back());

	float time = 0.0;
	int index = 0;
	// find the lower bound of the location of the sprite
	while(bezDists[index]<dioDist){
		index++;
	}

	// if it is at the distance then send back respective time otherwise calculate time
	if(bezDists[index]<dioDist){
		time = bezTimes[index];
	}else{
		time = (dioDist-bezDists[index])/(bezDists[index+1]-bezDists[index])*(bezTimes[index+1]-bezTimes[index])+bezTimes[index];
	}

	//curve that we are on which is time truncated since each curve is 1 (unit) time
	int curve = int(time);
	// time on the specific curve
	time -= curve;
	// index of the first control point of the curve
	curve = curve*3;
	//evaluate and set sprite location to the place
	dio.pos = evaluateBezierCurve(controlCurvePoints[curve], controlCurvePoints[curve+1], controlCurvePoints[curve+2], controlCurvePoints[curve+3], time);
	dio.rot = getRotMatrix(glm::vec3(0,0,1), tangentBezierCurve(controlCurvePoints[curve], controlCurvePoints[curve+1], controlCurvePoints[curve+2], controlCurvePoints[curve+3], time));
	dio.dir = tangentBezierCurve(controlCurvePoints[curve], controlCurvePoints[curve+1], controlCurvePoints[curve+2], controlCurvePoints[curve+3], time);

	dio.wheelTurn+=dio.turnSpeed;
}

void recomputeHankLocation(){
	hankDist += hankSpeed;
	if (hankDist >= (controlCurvePoints.size()-1)/3) {
		hankDist -= (controlCurvePoints.size()-1)/3;
	}
	//curve that we are on which is time truncated since each curve is 1 (unit) time
	int curve = int(hankDist);
	// time on the specific curve
	float time = hankDist-curve;
	// index of the first control point of the curve
	curve = curve*3;
	//evaluate and set sprite location to the place
	hank.pos = evaluateBezierCurve(controlCurvePoints[curve], controlCurvePoints[curve+1], controlCurvePoints[curve+2], controlCurvePoints[curve+3], time);
	hank.rot = getRotMatrix(glm::vec3(1,0,0), tangentBezierCurve(controlCurvePoints[curve], controlCurvePoints[curve+1], controlCurvePoints[curve+2], controlCurvePoints[curve+3], time));
	hank.dir = tangentBezierCurve(controlCurvePoints[curve], controlCurvePoints[curve+1], controlCurvePoints[curve+2], controlCurvePoints[curve+3], time);
}

// recomputeAll() //////////////////////////////////////////////////////
//
// This function updates the car and camera's position/direction
// vectors based on any changes made due to movement
//
////////////////////////////////////////////////////////////////////////////////
void recomputeAll(){

	float x = (torvesta.pos.x+50)/100;
	float z = (torvesta.pos.z+50)/100;
	glm::vec3 point = (evaluateBezierCurve(
						evaluateBezierCurve(controlSurfacePoints[0][0], controlSurfacePoints[0][1], controlSurfacePoints[0][2], controlSurfacePoints[0][3], x),
						evaluateBezierCurve(controlSurfacePoints[0][4], controlSurfacePoints[0][5], controlSurfacePoints[0][6], controlSurfacePoints[0][7], x),
						evaluateBezierCurve(controlSurfacePoints[0][8], controlSurfacePoints[0][9], controlSurfacePoints[0][10], controlSurfacePoints[0][11], x),
						evaluateBezierCurve(controlSurfacePoints[0][12], controlSurfacePoints[0][13], controlSurfacePoints[0][14], controlSurfacePoints[0][15], x),
						z));
	torvesta.pos.y = point.y;
	recomputeCarOrientation();

	// recompute both car and camera
	torvesta.recompute();
	recomputeDioLocation();
	recomputeHankLocation();
	recomputeOrientation();

}







void drawTrack();

// renderCoasterCurve() //////////////////////////////////////////////////////////
//
// Responsible for drawing a Bezier Curve as defined by four control points.
//  Breaks the curve into n segments as specified by the resolution.
//
////////////////////////////////////////////////////////////////////////////////
void renderCoasterCurve( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float resolution ) {
	for(float i = 0.0f; i < 1.0f; i+=resolution){
		glm::vec3 point = evaluateBezierCurve(p0, p1, p2, p3, i);

		glm::mat4 rotMtx = getRotMatrix(glm::vec3(1,0,0), tangentBezierCurve(p0, p1, p2, p3, i));
        glm::mat4 transMtx = glm::translate(glm::mat4(1.0f), point);
        glMultMatrixf( &transMtx[0][0] );
        glMultMatrixf( &rotMtx[0][0] );
		drawTrack();
        glMultMatrixf( &( glm::inverse( rotMtx ) )[0][0] );
        glMultMatrixf( &( glm::inverse( transMtx ) )[0][0] );
	}
}

//*************************************************************************************
//
// Event Callbacks

//
//	void error_callback( int error, const char* description )
//
//		We will register this function as GLFW's error callback.
//	When an error within GLFW occurs, GLFW will tell us by calling
//	this function.  We can then print this info to the terminal to
//	alert the user.
//
static void error_callback( int error, const char* description ) {
	fprintf( stderr, "[ERROR]: %s\n", description );
}

// simply if a button is pressed turn it's related 'switch' on and if released off
static void keyboard_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
	if(freeCamOn){
		if( action == GLFW_PRESS ) {
			switch( key ) {
				case GLFW_KEY_ESCAPE:
				case GLFW_KEY_Q:
					exit(EXIT_SUCCESS);
				case GLFW_KEY_W:
					freeCamPos+=freeCamSpeed*freeCamDir;
					break;
				case GLFW_KEY_S:
					freeCamPos-=freeCamSpeed*freeCamDir;
					break;
				case GLFW_KEY_4:
					freeCamOn = false;
					lookAt = &(heroFocus->pos);
					camPos = &arcCamPos;
					break;
			}
		}

		if( action == GLFW_REPEAT ){
			switch( key ) {
				case GLFW_KEY_W:
					freeCamPos+=freeCamSpeed*freeCamDir;
					break;
				case GLFW_KEY_S:
					freeCamPos-=freeCamSpeed*freeCamDir;
					break;
			}
		}
	}else{
		if( action == GLFW_PRESS ) {
			switch( key ) {
				case GLFW_KEY_ESCAPE:
				case GLFW_KEY_Q:
					exit(EXIT_SUCCESS);
				case GLFW_KEY_W:
					torvesta.moveForward = true;
					break;
				case GLFW_KEY_S:
					torvesta.moveBack = true;
					break;
				case GLFW_KEY_A:
					torvesta.turnLeft = true;
					break;
				case GLFW_KEY_D:
					torvesta.turnRight = true;
					break;
				case GLFW_KEY_1:
					heroFocus = heroes[0];
					lookAt=&(heroFocus->pos);
					break;
				case GLFW_KEY_2:
					heroFocus = heroes[1];
					lookAt=&(heroFocus->pos);
					break;
				case GLFW_KEY_3:
					heroFocus = heroes[2];
					lookAt=&(heroFocus->pos);
					break;
				case GLFW_KEY_LEFT_CONTROL:
					zoomOn = true;
					break;
				case GLFW_KEY_4:
					firstPersonOn = false;
					freeCamOn = true;
					lookAt = &freeCamLookAt;
					camPos = &freeCamPos;
					break;
				case GLFW_KEY_5:
					firstPersonOn = !firstPersonOn;
					break;
				
			}
		}

		if( action == GLFW_RELEASE ) {
			switch( key ) {
				case GLFW_KEY_W:
					torvesta.moveForward = false;
					break;
				case GLFW_KEY_S:
					torvesta.moveBack = false;
					break;
				case GLFW_KEY_A:
					torvesta.turnLeft = false;
					break;
				case GLFW_KEY_D:
					torvesta.turnRight = false;
					break;
				case GLFW_KEY_LEFT_CONTROL:
					zoomOn = false;
					break;
				
			}
		}
	}
	

}

// cursor_callback() ///////////////////////////////////////////////////////////
//
//  GLFW callback for mouse movement. We update cameraPhi and/or cameraTheta
//      based on how much the user has moved the mouse in the
//      X or Y directions (in screen space) and whether they have held down
//      the left or right mouse buttons. If the user hasn't held down any
//      buttons, the function just updates the last seen mouse X and Y coords.
//
////////////////////////////////////////////////////////////////////////////////
static void cursor_callback( GLFWwindow *window, double x, double y ) {

	if( leftMouseButton == GLFW_PRESS ) {
		float dx = x-mousePos.x;
		float dy = y-mousePos.y;

		if(freeCamOn){
			freeCameraTheta+=0.005*dx;
			float newPhi = freeCameraPhi-0.005*dy;
			if(newPhi>=0 && newPhi<=M_PI){
				freeCameraPhi = newPhi;
			}
		}else{
			// if zooming then dont calculate camera rotation but the distance the camera is from object
			if(zoomOn){
				cameraP += zoom*dy;
				if(cameraP<2){
					cameraP = 2;
				}
			}else{
				arcCameraTheta+=0.005*dx;
				float newPhi = arcCameraPhi-0.005*dy;
				if(newPhi>=0 && newPhi<=M_PI){
					arcCameraPhi = newPhi;
				}
			}
		}
		
			
		recomputeOrientation();     // update camera (x,y,z) based on (theta,phi)
	}

	mousePos.x = x;
	mousePos.y = y;
}

// mouse_button_callback() /////////////////////////////////////////////////////
//
//  GLFW callback for mouse clicks. We save the state of the mouse button
//      when this is called so that we can check the status of the mouse
//      buttons inside the motion callback (whether they are up or down).
//
////////////////////////////////////////////////////////////////////////////////
static void mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
	if( button == GLFW_MOUSE_BUTTON_LEFT ) {
		leftMouseButton = action;
	}
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

// drawGrid() //////////////////////////////////////////////////////////////////
//
//  Function to draw a grid in the XZ-Plane using OpenGL 2D Primitives (GL_LINES)
//
////////////////////////////////////////////////////////////////////////////////
void drawGrid() {
	/*
     *	We will get to why we need to do this when we talk about lighting,
     *	but for now whenever we want to draw something with an OpenGL
     *	Primitive - like a line, triangle, point - we need to disable lighting
     *	and then reenable it for use with the CSCI441 3D Objects.
     */
	glDisable( GL_LIGHTING );
	glColor3ub(55,55,55);
	glBegin(GL_LINES);
		for(float i = -50.0f; i <= 50.0f; i++){
			glVertex3f(-50.0f, 0.0f, i);
			glVertex3f(50.0f, 0.0f, i);
			glVertex3f(i, 0.0f, -50.0f);
			glVertex3f(i, 0.0f, 50.0f);
		}		
	glEnd();
	/*
     *	As noted above, we are done drawing with OpenGL Primitives, so we
     *	must turn lighting back on.
     */
	glEnable( GL_LIGHTING );
}

// drawCity() //////////////////////////////////////////////////////////////////
//
//  Function to draw a random city using CSCI441 3D Cubes
//
////////////////////////////////////////////////////////////////////////////////
void drawCity() {
	for(int i = -50; i <= 50; i++){
		for(int j = -50; j <= 50; j++){
			if(getRand() < 0.2f && i % 2 == 0 && j%2 == 0){
					float z = getRand() * 10;
					glColor3f(getRand(), getRand(), getRand());
					glm::mat4 trans = glm::translate( glm::mat4(1.0f), glm::vec3( i, z/2, j ) );
					glm::mat4 scale = glm::scale( glm::mat4(1.0f), glm::vec3(1.0f, z, 1.0f));
					glMultMatrixf( &trans[0][0] );
					glMultMatrixf( &scale[0][0]);
					CSCI441::drawSolidCube(1);
					glMultMatrixf( &(glm::inverse( scale))[0][0] );
					glMultMatrixf( &(glm::inverse( trans))[0][0] );

					glColor3f(1.0f,1.0f,1.0f);
					
			}
		}
	}

}

void drawFloor() {
	std::vector<std::vector<glm::vec3>> floorPoints;
	
	glColor3ub(26,151,240);
	for(int i = 0; i < (int)controlSurfacePoints.size(); i++) {
		for(float t = 0; t <= 1.0f; t+= .01){
			std::vector<glm::vec3> points;
			for(float j = 0.0f; j <= 1.0f; j+= .01){
				points.push_back(evaluateBezierCurve(
					evaluateBezierCurve(controlSurfacePoints[i][0], controlSurfacePoints[i][1], controlSurfacePoints[i][2], controlSurfacePoints[i][3], j),
					evaluateBezierCurve(controlSurfacePoints[i][4], controlSurfacePoints[i][5], controlSurfacePoints[i][6], controlSurfacePoints[i][7], j),
					evaluateBezierCurve(controlSurfacePoints[i][8], controlSurfacePoints[i][9], controlSurfacePoints[i][10], controlSurfacePoints[i][11], j),
					evaluateBezierCurve(controlSurfacePoints[i][12], controlSurfacePoints[i][13], controlSurfacePoints[i][14], controlSurfacePoints[i][15], j),
					t));
			}
			floorPoints.push_back(points);
		}
	}	
	glDisable(GL_LIGHTING);
	
		for(int i = 0; i < (int)floorPoints.size() - 1; i++){
			glBegin(GL_TRIANGLE_STRIP);
			for(int j = 0; j < (int)floorPoints[i].size() - 1; j++){
				glVertex3fv(glm::value_ptr(floorPoints[i][j]));
				glVertex3fv(glm::value_ptr(floorPoints[i][j+1]));
				glVertex3fv(glm::value_ptr(floorPoints[i+1][j]));
				glVertex3fv(glm::value_ptr(floorPoints[i+1][j+1]));
			}
			glEnd();
		}
	glEnable(GL_LIGHTING);

}

// generateEnvironmentDL() /////////////////////////////////////////////////////
//
//  This function creates a display list with the code to draw a simple
//      environment for the user to navigate through.
//
//  And yes, it uses a global variable for the display list.
//  I know, I know! Kids: don't try this at home. There's something to be said
//      for object-oriented programming after all.
//
////////////////////////////////////////////////////////////////////////////////
void generateEnvironmentDL() {
	environmentDL = glGenLists(1);
	glNewList(environmentDL, GL_COMPILE);
		drawGrid();
		//drawCity();
		drawFloor();
	glEndList();

}

//Draw coaster track piece
void drawTrack() {
    glColor3f(1.0f,0,0);
    CSCI441::drawSolidCube(1);
}


//
//	void renderScene()
//
//		This method will contain all of the objects to be drawn.
//		Also checks if the car is out of bounds
//
void renderScene(void)  {
	glCallList(environmentDL);
	glCallList(objectsDL);
	
		
		glDisable( GL_LIGHTING);
		for(int i = 0; (i+3)<(int)controlCurvePoints.size(); i+=3){
			renderCoasterCurve(controlCurvePoints[i], controlCurvePoints[i+1],controlCurvePoints[i+2], controlCurvePoints[i+3], .01f);
		}
		
	
	glEnable( GL_LIGHTING );

	dio.Draw();
	hank.Draw();
	torvesta.Draw();
}

//*************************************************************************************
//
// Setup Functions

//
//  void setupGLFW()
//
//      Used to setup everything GLFW related.  This includes the OpenGL context
//	and our window.
//
GLFWwindow* setupGLFW() {
	// set what function to use when registering errors
	// this is the ONLY GLFW function that can be called BEFORE GLFW is initialized
	// all other GLFW calls must be performed after GLFW has been initialized
	glfwSetErrorCallback( error_callback );

	// initialize GLFW
	if( !glfwInit() ) {
		fprintf( stderr, "[ERROR]: Could not initialize GLFW\n" );
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW initialized\n" );
	}

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );	// request OpenGL v2.X
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );	// request OpenGL v2.1
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );		// do not allow our window to be able to be resized

	// create a window for a given size, with a given title
	GLFWwindow *window = glfwCreateWindow( windowWidth, windowHeight, "A3", NULL, NULL );
	if( !window ) {						// if the window could not be created, NULL is returned
		fprintf( stderr, "[ERROR]: GLFW Window could not be created\n" );
		glfwTerminate();
		exit( EXIT_FAILURE );
	} else {
		fprintf( stdout, "[INFO]: GLFW Window created\n" );
	}

	glfwMakeContextCurrent(window);		// make the created window the current window
	glfwSwapInterval(1);				     	// update our screen after at least 1 screen refresh

	glfwSetKeyCallback( window, keyboard_callback );							// set our keyboard callback function
	glfwSetCursorPosCallback( window, cursor_callback );					// set our cursor position callback function
	glfwSetMouseButtonCallback( window, mouse_button_callback );	// set our mouse button callback function

	return window;						       // return the window that was created
}

//
//  void setupOpenGL()
//
//      Used to setup everything OpenGL related.  For now, the only setting
//	we need is what color to make the background of our window when we clear
//	the window.  In the future we will be adding many more settings to this
//	function.
//
void setupOpenGL() {
	// tell OpenGL to perform depth testing with the Z-Buffer to perform hidden
	//		surface removal.  We will discuss this more very soon.
	glEnable( GL_DEPTH_TEST );
	
	//******************************************************************
	// this is some code to enable a default light for the scene;
	// feel free to play around with this, but we won't talk about
	// lighting in OpenGL for another couple of weeks yet.
	GLfloat lightCol[4] = { 1, 1, 1, 1};
	GLfloat ambientCol[4] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat lPosition[4] = { 10, 10, 10, 1 };
	glLightfv( GL_LIGHT0, GL_POSITION,lPosition );
	glLightfv( GL_LIGHT0, GL_DIFFUSE,lightCol );
	glLightfv( GL_LIGHT0, GL_AMBIENT, ambientCol );
	glEnable( GL_LIGHTING );	 
	glEnable( GL_LIGHT0 );

	// tell OpenGL not to use the material system; just use whatever we
	// pass with glColor*()
	glEnable( GL_COLOR_MATERIAL );
	glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
	//******************************************************************

	// tells OpenGL to blend colors across triangles. Once lighting is
	// enabled, this means that objects will appear smoother - if your object
	// is rounded or has a smooth surface, this is probably a good idea;
	// if your object has a blocky surface, you probably want to disable this.
	glShadeModel( GL_SMOOTH );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// set the clear color to black
}

//
//  void setupScene()
//
//      Used to setup everything scene related.  Give our camera an
//	initial starting point and generate the display list for our city
//
void setupScene() {
	// give the camera a scenic starting point.
	arcCamPos.x = 10;
	arcCamPos.y = 10;
	arcCamPos.z = 10;
	arcCameraTheta = M_PI / 4.0f;
	arcCameraPhi = M_PI / 4.0f;
	cameraP = 10;
	zoom = 0.1;

	freeCamPos.x = 60;
	freeCamPos.y = 40;
	freeCamPos.z = 30;
	freeCameraTheta = -M_PI / 3.0f;
	freeCameraPhi = M_PI / 2.8f;
	freeCamSpeed = 0.8;

	heroFocus = &torvesta;
	recomputeOrientation();
	
	lookAt=&(heroFocus->pos);
	camPos = &arcCamPos;

	srand( time(NULL) );	// seed our random number generator
	
}

// set up initial values of the car
void setupHeroes() {

	dio = Dio(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1) );
	hank = HankHill(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1) );
	torvesta = Torvesta(glm::vec3(0, 1, 0), glm::vec3(0, 0, 1) );
	
}

///*************************************************************************************
//
// Our main function

//
//	int main( int argc, char *argv[] )
//
//		Really you should know what this is by now.  We will make use of the parameters later
//
int main( int argc, char *argv[] ) {

	
	// GLFW sets up our OpenGL context so must be done first
	GLFWwindow *window = setupGLFW();	// initialize all of the GLFW specific information releated to OpenGL and our window
	setupOpenGL();										// initialize all of the OpenGL specific information
	setupScene();											// initialize objects in our scene
	setupHeroes();

	std::string file;
	cout<< "Enter Filename: ";
	cin >> file;
	if(loadFile(file)){
		loadDt();
	}else{
		cerr << "Error code: " << strerror(errno);
	}
	generateEnvironmentDL();
	
	//  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
	//	until the user decides to close the window and quit the program.  Without a loop, the
	//	window will display once and then the program exits.
	while( !glfwWindowShouldClose(window) ) {	// check if the window was instructed to be closed
		glDrawBuffer( GL_BACK );				// work with our back frame buffer
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

		// update the projection matrix based on the window size
		// the GL_PROJECTION matrix governs properties of the view coordinates;
		// i.e. what gets seen - use a perspective projection that ranges
		// with a FOV of 45 degrees, for our current aspect ratio, and Z ranges from [0.001, 1000].
		glm::mat4 projMtx = glm::perspective( 45.0f, (GLfloat)windowWidth / (GLfloat)windowHeight, 0.001f, 1000.0f );
		glMatrixMode( GL_PROJECTION );	// change to the Projection matrix
		glLoadIdentity();				// set the matrix to be the identity
		glMultMatrixf( &projMtx[0][0] );// load our orthographic projection matrix into OpenGL's projection matrix state

		// Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
		// when using a Retina display the actual window can be larger than the requested window.  Therefore
		// query what the actual size of the window we are rendering to is.
		GLint framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize( window, &framebufferWidth, &framebufferHeight );

		// update the viewport - tell OpenGL we want to render to the whole window
		glViewport( 0, 0, framebufferWidth, framebufferHeight );

		glMatrixMode( GL_MODELVIEW );	// make the ModelView matrix current to be modified by any transformations
		glLoadIdentity();							// set the matrix to be the identity

// set up our look at matrix to position our camera
		// TODO #6: Change how our lookAt matrix gets constructed
		glm::mat4 viewMtx = glm::lookAt( *camPos,		// camera is located at (10, 10, 10)
										 *lookAt,		// camera is looking at (0, 0, 0,)
										 glm::vec3(  0,  1,  0 ) );		// up vector is (0, 1, 0) - positive Y
		// multiply by the look at matrix - this is the same as our view martix
		glMultMatrixf( &viewMtx[0][0] );




		recomputeAll();
		renderScene();					// draw everything to the window

		if(firstPersonOn){
			glMultMatrixf( &(glm::inverse( viewMtx ))[0][0] );
			glm::vec3 heroPos = heroFocus->pos;
			glm::vec3 heroDir = heroFocus->dir;
			heroPos.y +=4;

			

			glm::mat4 viewMtx = glm::lookAt( heroPos+0.7f*normalize(heroDir),		// camera is located at (10, 10, 10)
										 	 heroPos+2.0f*normalize(heroDir),		// camera is looking at (0, 0, 0,)
										 	 glm::vec3(  0,  1,  0 ) );		// up vector is (0, 1, 0) - positive Y
			// multiply by the look at matrix - this is the same as our view martix
			glMultMatrixf( &viewMtx[0][0] );

			glViewport(10, framebufferHeight-framebufferHeight/4-10, framebufferWidth/4, framebufferHeight/4 );
			glScissor(10-1, framebufferHeight-framebufferHeight/4-10-1, framebufferWidth/4+2, framebufferHeight/4+2 );
			glEnable(GL_SCISSOR_TEST);
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			renderScene();
			glDisable(GL_SCISSOR_TEST);
		}

		glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				// check for any events and signal to redraw screen
	}

	glfwDestroyWindow( window );// clean up and close our window
	glfwTerminate();						// shut down GLFW to clean up our context

	return EXIT_SUCCESS;				// exit our program successfully!
}
