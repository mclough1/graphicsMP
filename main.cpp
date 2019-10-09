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

#include <math.h>				// for cos(), sin() functionality
#include <stdio.h>			// for printf functionality
#include <stdlib.h>			// for exit functionality
#include <time.h>			  // for time() functionality
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//*************************************************************************************
//
// Global Parameters



// global variables to keep track of window width and height.
// set to initial values for convenience, but we need variables
// for later on in case the window gets resized.
int windowWidth = 640, windowHeight = 480;

int leftMouseButton;    	 						// status of the mouse button
glm::vec2 mousePos;			              		  	// last known X and Y of the mouse

glm::vec3 camPos;            						// camera position in cartesian coordinates
float cameraTheta, cameraPhi, camRad = 5.0f;        // camera DIRECTION in spherical coordinates
glm::vec3 camDir; 			                    	// camera DIRECTION in cartesian coordinates

std::vector<glm::vec3> controlPoints;

bool ctrlPressed = false;

GLuint environmentDL;                       		// display list for the 'city'

//*************************************************************************************
//
// Helper Functions

// getRand() ///////////////////////////////////////////////////////////////////
//
//  Simple helper function to return a random number between 0.0f and 1.0f.
//
////////////////////////////////////////////////////////////////////////////////

bool loadControlPoints( std::string filename ) {
	std::ifstream in = std::ifstream(filename);
	int numPoints = 0;
	if(in) {
		in >> numPoints;
		for(int i = 0; i < numPoints; i++){
			glm::vec3 point;
			char waste;
			if(in >> point.x >> waste >> point.y >> waste >> point.z) {
				controlPoints.push_back(point);
			} else {
				return false;
			}
		}
		
		return true;
	} else {
		return false;
	}
	
}

float getRand() { return rand() / (float)RAND_MAX; }

// recomputeOrientation() //////////////////////////////////////////////////////
//
// This function updates the camera's direction in cartesian coordinates based
//  on its position in spherical coordinates. Should be called every time
//  cameraTheta or cameraPhi is updated.
//
////////////////////////////////////////////////////////////////////////////////
void recomputeOrientation() {
	
	float x = sin(cameraPhi) * sin(cameraTheta);
	float y = -1 * cos(cameraPhi);
	float z = -1 * cos(cameraTheta) * sin(cameraPhi);
	float distance = sqrt((x*x) + (z*z) + (y * y));
	x /= distance;
	y /= distance;
	z /= distance;
	camPos = camRad * glm::vec3(x, y, z);
}

// tangentBezierCurve() //////////////////////////////////////////////////////////
//
//	Returns the tangent at a point on the curve
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 tangentBezierCurve( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t ) {
    return -3.0f * (float)pow(1-t, 2) * p0 + 3.0f * (float)pow(1-t, 2) * p1 - 6.0f * t * (1-t) * p1 - 3.0f * (float)pow(t, 2) * p2 + 6.0f * t * (1-t) * p2 + 3.0f * (float)pow(t, 2) * p3;
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

// evaluateBezierCurve() //////////////////////////////////////////////////////////
//
//	Returns a point on the bezier curve
//
////////////////////////////////////////////////////////////////////////////////
glm::vec3 evaluateBezierCurve( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t ) {
	return (float)pow(1-t, 3) * p0 + 3.0f * (float)pow(1-t, 2) * t * p1 + 3.0f * (1-t) * (float)pow(t,2) * p2 + (float)pow(t, 3) * p3;
}

void drawTrack();

// renderBezierCurve() //////////////////////////////////////////////////////////
//
// Responsible for drawing a Bezier Curve as defined by four control points.
//  Breaks the curve into n segments as specified by the resolution.
//
////////////////////////////////////////////////////////////////////////////////
void renderBezierCurve( glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float resolution ) {
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

//Moves the car and the wheels and checks if ctrl is pressed
static void keyboard_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
	ctrlPressed = false;
	if( action == GLFW_PRESS ) {
		switch( key ) {
			case GLFW_KEY_ESCAPE:
			case GLFW_KEY_Q:
				exit(EXIT_SUCCESS);
			case GLFW_KEY_W:
				break;
			case GLFW_KEY_S:
				break;
			case GLFW_KEY_A:
				break;
			case GLFW_KEY_D:
				break;
		}
	}
	if(action == GLFW_REPEAT){
		switch(key) {
			case GLFW_KEY_W:
				break;
			case GLFW_KEY_S:
				break;
			case GLFW_KEY_A:
				break;
			case GLFW_KEY_D:
				break;
			case GLFW_KEY_LEFT_CONTROL:
				ctrlPressed = true;
				break;

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
//		Also zooms in and out if CTRL is pressed
//
////////////////////////////////////////////////////////////////////////////////
static void cursor_callback( GLFWwindow *window, double x, double y ) {
	if( leftMouseButton == GLFW_PRESS && !ctrlPressed) {
		cameraTheta += .005 * (x - mousePos.x);
		cameraPhi += .005 * (mousePos.y - y);
		recomputeOrientation();     // update camera (x,y,z) based on (theta,phi)
	} else if( leftMouseButton == GLFW_PRESS && ctrlPressed){
		camRad += .05 * (mousePos.y - y);
		recomputeOrientation();
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
	
		glBegin(GL_LINE_STRIP);
		glColor3ub(255,255,0);
		for(int i = 0; i < controlPoints.size(); i++){
			
			glVertex3f(controlPoints[i].x, controlPoints[i].y, controlPoints[i].z);
		}
		glEnd();
		glEnable(GL_LIGHTING);
		glColor3ub(0,255,0);
		for(int i = 0; i < controlPoints.size(); i++){
			glm::mat4 circTrans = glm::translate( glm::mat4(1.0f), glm::vec3(controlPoints[i].x, controlPoints[i].y, controlPoints[i].z));
			glMultMatrixf( &circTrans[0][0] );
			CSCI441::drawSolidSphere(.2,10,10);
			glMultMatrixf( &(glm::inverse( circTrans))[0][0] );
		}
		glDisable( GL_LIGHTING);
	
		//glColor3ub(0, 0, 255);
		//glBegin(GL_LINE_STRIP);
		
		for(int i = 0; i < controlPoints.size() - 1; i+=4){
			renderBezierCurve(controlPoints[i], controlPoints[i+1],controlPoints[i+2], controlPoints[i+3], .01f);
		}
		//glEnd();
	
	glEnable( GL_LIGHTING );
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
	camDir.x = 0;
	camDir.y = 0;
	camDir.z = 0;
	cameraTheta = -M_PI / 3.0f;
	cameraPhi = M_PI / 2.8f;
	recomputeOrientation();

	srand( time(NULL) );	// seed our random number generator
	generateEnvironmentDL();
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
	std::string file;
	std::cout << "File of control points";
	std::cin >> file;
	if(!loadControlPoints(file)) {
		std::cerr << "Error code: " << strerror(errno);
	}
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
		glm::mat4 viewMtx = glm::lookAt( camPos,		// camera is located at (10, 10, 10)
										 camDir,	// camera is looking at (0, 0, 0,)
										 glm::vec3(  0,  1,  0 ) );		// up vector is (0, 1, 0) - positive Y
		// multiply by the look at matrix - this is the same as our view martix
		glMultMatrixf( &viewMtx[0][0] );
		
		renderScene();					// draw everything to the window

		glfwSwapBuffers(window);// flush the OpenGL commands and make sure they get rendered!
		glfwPollEvents();				// check for any events and signal to redraw screen
	}

	glfwDestroyWindow( window );// clean up and close our window
	glfwTerminate();						// shut down GLFW to clean up our context

	return EXIT_SUCCESS;				// exit our program successfully!
}
