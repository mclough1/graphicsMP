//Keeps track of the teapot scale 


//Draws a single wheel
void drawWheel(){
	CSCI441::drawSolidDisk(0.1f,0.3f,10,10);
}

//Handles drawing, rotating, translating and scaling of wheels
void drawWheels(){
	glm::mat4 trans = glm::translate( glm::mat4(1.0f), glm::vec3( -1.0f, 0.0f, 0.6f ) );
	glMultMatrixf( &trans[0][0] );
	glMultMatrixf( &rotateMtx[0][0] );
	drawWheel();
	glMultMatrixf( &(glm::inverse( trans))[0][0] );

	trans = glm::translate( glm::mat4(1.0f), glm::vec3( 1.0f, 0.0f, -0.6f ) );
	glMultMatrixf( &trans[0][0] );
	drawWheel();
	glMultMatrixf( &(glm::inverse( trans))[0][0] );

	trans = glm::translate( glm::mat4(1.0f), glm::vec3( -1.0f, 0.0f, -0.6f ) );
	glMultMatrixf( &trans[0][0] );
	drawWheel();
	glMultMatrixf( &(glm::inverse( trans))[0][0] );

	trans = glm::translate( glm::mat4(1.0f), glm::vec3( 1.0f, 0.0f, 0.6f ) );
	glMultMatrixf( &trans[0][0] );
	drawWheel();
	glMultMatrixf( &(glm::inverse( trans))[0][0] );
}

void drawCar(){
	glColor3ub(255, 0, 0);
	glm::mat4 trans = glm::translate( glm::mat4(1.0f), glm::vec3( 0.0f, 0.0f, 0.0f ) );
	glm::mat4 scale = glm::scale( glm::mat4(1.0f), glm::vec3(2.0f, 0.25f, 1.0f));
	glMultMatrixf( &trans[0][0] );
	glMultMatrixf( &scale[0][0]);
	CSCI441::drawSolidCube(1);
	glMultMatrixf( &(glm::inverse( scale))[0][0] );
	glMultMatrixf( &(glm::inverse( trans))[0][0] );

	glColor3ub(0, 0, 255);
	drawWheels();

	if(teapotScale > 1.5f){
		teapotScaleFactor = -0.01f;
	} else if(teapotScale < 1.0f){
		teapotScaleFactor = 0.01f;
	}

	glm::mat4 rotateMtx = glm::rotate(glm::mat4(1.0f),  glm::radians(180.0f) , glm::vec3(0.0f, 1.0f, 0.0f));
	trans = glm::translate( glm::mat4(1.0f), glm::vec3( 0.0f, 0.50f, 0.0f ) );
	scale = glm::scale( glm::mat4(1.0f), glm::vec3(teapotScale, teapotScale, teapotScale));
	glMultMatrixf( &trans[0][0] );
	glMultMatrixf( &rotateMtx[0][0] );
	glMultMatrixf( &scale[0][0] );
	drawFront();
	glMultMatrixf( &(glm::inverse( scale))[0][0] );
	glMultMatrixf( &(glm::inverse( rotateMtx))[0][0] );
	glMultMatrixf( &(glm::inverse( trans))[0][0] );
	teapotScale += teapotScaleFactor;
}


void draw() {
	glm::mat4 rotateMtx = glm::rotate(glm::mat4(1.0f), carTheta, glm::vec3( 0.0f, 1.0f, 0.0f));
	glm::mat4 trans = glm::translate( glm::mat4(1.0f), carPos );
	
	glMultMatrixf( &trans[0][0] );
	glMultMatrixf( &rotateMtx[0][0] );
	drawCar();
	glMultMatrixf( &(glm::inverse( rotateMtx))[0][0] );
	glMultMatrixf( &(glm::inverse( trans))[0][0] );
}