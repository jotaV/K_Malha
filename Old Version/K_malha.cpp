//============================================================================
// Name        : K_Malha.cpp
// Author      : Jv
// Version     : Pré-Beta
// Description : Gerador de Malha usando o dispositivo Kinect
//============================================================================

#if defined(__APPLE__) || defined(MACOSX)
	#include <GLUT/glut.h>
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/glut.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include <iostream>

#include "libView/ViewModel.h"
#include "MeshSettings.h"
#include "CamSettings.h"
#include "meshStructure.h"

uint8_t *depth_mid, *depth_front, *depth_copy;
uint16_t  *depth, *depth_mesh;

bool got_depth = false, copyActive = false;

Group grupo;

using namespace std;

GLuint gl_depth_tex;
GLenum pMode = GL_LINE;

ViewModel vm;

int camOut, 
	meshOut;

Point 	exSW, 
		exNE, 
		center, 
		camViewX,
		camViewY,
		camViewZ;

int old_x,
	old_y,
	lastX, 
	lastY,
	curr_modifiers, 
	curr_button;
					
int winWdth = 640,
	winHeight = 480,
	outs = 0;


//=========================================================
// Keyboard Functions
//=========================================================

void redisplayAll(){
	glutSetWindow(meshOut);
	glutPostRedisplay();
	glutSetWindow(camOut);
	glutPostRedisplay();
}
void KeyPressed(unsigned char key, int x, int y){

	switch(key){
		case 27:	exitKinect();
					break;

		case 'p':
		case 'P':	takePicture();
					break;

		case 'g':
		case 'G': 	if(!copyActive) meshcpy(depth_mesh, depth, 640, 480);
					generateMesh(1);
					
					break;

		case 'f':
		case 'F': 	if(!copyActive) meshcpy(depth_mesh, depth, 640, 480);
					generateMesh(1, false);
					
					break;
					
		case 's':
		case 'S':	if(GLUT_ACTIVE_SHIFT){
						saveOBJ("out_" + outs);
						outs++;
					}else{
						string arquivo;
						cout << "Digite o nome: " << endl;
						cin >> arquivo;
						saveOBJ(arquivo);
					}
					break;

		case 'o':
		case 'O':   MaxDistChange(0.05f);
					break;

		case 'i':
		case 'I':	MaxDistChange(-0.05f);
					break;

		case 'l':
		case 'L':	MinDistChange(0.05f);
					break;

		case 'k':
		case 'K':	MinDistChange(-0.05f);
					break;
				
		case '1':	pMode = GL_POINT;
					break;
	
		case '2':	pMode = GL_LINE;
					break;
					
		case '3':	pMode = GL_FILL;
					break;
	}
	redisplayAll();

}

//=========================================================
// Cam View Functions
//=========================================================

void initVariables(){
	
	exSW.x = 640;
	exSW.y = 480;
	exSW.z = 1535;
	
	exNE.x = 
	exNE.y = 
	exNE.z = 
	
	camViewX.x =
	camViewX.y =
	camViewX.z = 0;
	
	center.x = 320;
	center.y = 240;
	center.z = 767.5;
}

void calcBoundBoxUnit(Vertice *v){

	if (v->pto.x < exSW.x)
		exSW.x = v->pto.x;
			
	if (v->pto.y < exSW.y)
		exSW.y = v->pto.y;
			
	if (v->pto.z < exSW.z)
		exSW.z = v->pto.z;
				
	if (v->pto.x > exNE.x)
		exNE.x = v->pto.x;
			
	if (v->pto.y > exNE.y)
		exNE.y = v->pto.y;
			
	if (v->pto.z > exNE.z)
		exNE.z = v->pto.z;
}

void calcPosCamera(){
	
	exSW.x *= 1.2;
	exSW.y *= 1.2;
	exSW.z *= 1.2;
	
	exNE.x *= 1.2;
	exNE.y *= 1.2;
	exNE.z *= 1.2;
	
	center.x = (exSW.x + exNE.x) / 2;
	center.y = (exSW.y + exNE.y) / 2;
	center.z = (exSW.z + exNE.z) / 2;
	
	float tanFOV = tan(60.0 * 3.1416 / 180.0);
	float bigDimension = MAX((exNE.y - exSW.y), (exNE.z - exSW.z));
	
	camViewX.x = (exNE.x + bigDimension / tanFOV);
	camViewX.y = center.y;
	camViewX.z = center.z;
	
	bigDimension = MAX((exNE.x - exSW.x), (exNE.z - exSW.z));
	
	camViewY.x = center.x;
	camViewY.y = (exNE.y + bigDimension / tanFOV);
	camViewY.z = center.z;
	
	bigDimension = MAX((exNE.x - exSW.x), (exNE.y - exSW.y));
	
	camViewZ.x = center.x;
	camViewZ.y = center.y;
	camViewZ.z = (exNE.z + bigDimension / tanFOV);
}

//=========================================================
// Mesh OpenGL Functions
//=========================================================

void drawAxis() {

	glDisable (GL_LIGHTING);
	
	glBegin(GL_LINES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(exNE.x, center.y, center.z);
		glVertex3f(center.x, center.y, center.z);
		
		if (exSW.x < center.x ) {
			glColor3f(0.5, 0.0, 0.0);
			glVertex3f(center.x, center.y, center.z);
			glVertex3f(exSW.x, center.y, center.z);
			}

		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(center.x, exNE.y, center.z);
		glVertex3f(center.x, center.y, center.z);
		
		if (exSW.y < center.y) {
			glColor3f(0.0, 0.5, 0.0);
			glVertex3f(center.x, center.y, center.z);
			glVertex3f(center.x, exSW.y, center.z);
			}

		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(center.x, center.y, exNE.z);
		glVertex3f(center.x, center.y, center.z);
		
		if (exSW.z < center.z ){
			glColor3f(0.0, 0.0, 0.5);
			glVertex3f(center.x, center.y, center.z);
			glVertex3f(center.x, center.y, exSW.z);
			}

	glEnd();

	glEnable (GL_LIGHTING);
	
}

void drawObject() {
	glPolygonMode(GL_FRONT_AND_BACK, pMode);
	glBegin(GL_TRIANGLES);
	
	for (unsigned int f = 0; f < grupo.faces.size(); f++){ 
	for (unsigned int iv = 0; iv < 3; iv++){  
			glNormal3f	(	grupo.vertices[grupo.faces[f]->indV[iv] -1]->normal.x,
							grupo.vertices[grupo.faces[f]->indV[iv] -1]->normal.y,
							grupo.vertices[grupo.faces[f]->indV[iv] -1]->normal.z
						);
			glVertex3f	(	grupo.vertices[grupo.faces[f]->indV[iv] -1]->pto.x,
							grupo.vertices[grupo.faces[f]->indV[iv] -1]->pto.y,
							grupo.vertices[grupo.faces[f]->indV[iv] -1]->pto.z
						);
	}}
	glEnd();
}

void DrawMesh(void){
	
	float Yc; float Xc; float Zc;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluPerspective(60.0, glutGet(GLUT_WINDOW_WIDTH)/glutGet(GLUT_WINDOW_HEIGHT), 0.01, 20.0*exNE.z);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glViewport(0, 0, winWdth, winHeight);    
	
	vm.load_to_openGL_noident();
	
	drawAxis();
	drawObject();
		
	glutSwapBuffers();
}

void MoveMesh(int x, int y ) {
  
	float move_x, move_y;
	int   delta_x, delta_y;
		
	if ( curr_button == GLUT_LEFT_BUTTON ) {
		delta_x = x - old_x;
		delta_y = y - old_y;
		move_x = (float) -delta_x;
		move_y = (float) -delta_y;
		
		if(curr_modifiers & GLUT_ACTIVE_SHIFT){
			vm.lookat_yaw( move_x );
			vm.lookat_pitch( move_y );
		}else{ 
			if(curr_modifiers & GLUT_ACTIVE_CTRL){
				vm.roll(-move_x * 0.05);	  
				vm.move(0.0, 0.0, -(move_y) );	
			}else{
				vm.eye_yaw( move_x );
				vm.eye_pitch( move_y );
			}
		}
		old_x = x;
		old_y = y;
		glutPostRedisplay();
	}
}

void MouseMesh(int button, int state, int x, int y){
	if ( button == GLUT_LEFT_BUTTON  ) {
		if 	(state == GLUT_DOWN )
			old_x          = x;
			old_y          = y;
			curr_button    = button;
			curr_modifiers = glutGetModifiers();
	}
	glutPostRedisplay();
}

void ReSizeMesh(int w, int h){
	winWdth = w;
	winHeight = h;
	glViewport(0, 0, winWdth, winHeight); 
}

void initViewModel(){
	vm.reset();
	vm.eye.set(camViewZ.x, camViewZ.y, -camViewZ.z);
	vm.lookat.set(center.x, center.y, center.z);
	vm.set_up( 0.0, -1.0, 0.0 );
	vm.update();
}

void initLight(void) {
	glutSetWindow(meshOut);
	
	GLfloat mat_diffuse[] 		= { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_specular[] 		= { 0.5, 0.5, 0.5, 1.0 };
	GLfloat mat_ambient[] 		= { 0.3, 0.3, 0.3, 1.0 };
	GLfloat mat_shininess[] 	= { 150.0 };
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, 	mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, 	mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, 	mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS,	mat_shininess);

	GLfloat light0_position[] 	= {exSW.x, center.y, center.z, 1.0f};
	GLfloat light0_ambient[] 	= {0.0f, 0.0f, 0.1f, 1.0f};
	GLfloat light0_diffuse[] 	= {0.0f, 0.0f, 1.0f, 1.0f};
	GLfloat light0_specular[] 	= {0.0f, 0.7f, 0.0f, 1.0f};

	GLfloat light1_position[] 	= {exNE.x, center.y, center.z, 1.0f};
	GLfloat light1_ambient[] 	= {0.0f, 0.1f, 0.0f, 1.0f};
	GLfloat light1_diffuse[] 	= {0.0f, 1.0f, 0.0f, 1.0f};
	GLfloat light1_specular[] 	= {0.0f, 0.0f, 0.7f, 1.0f};

	GLfloat light2_position[] 	= {center.x, exSW.y, -exSW.z, 1.0f};
	GLfloat light2_ambient[] 	= {0.1f, 0.0f, 0.0f, 1.0f};
	GLfloat light2_diffuse[] 	= {1.0f, 0.0f, 0.0f, 1.0f};
	GLfloat light2_specular[] 	= {0.7f, 0.0f, 0.0f, 1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, 	light0_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, 	light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, 	light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, 	light0_specular);

	glLightfv(GL_LIGHT1, GL_POSITION, 	light1_position);
	glLightfv(GL_LIGHT1, GL_AMBIENT, 	light1_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, 	light1_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, 	light1_specular);

	glLightfv(GL_LIGHT2, GL_POSITION, 	light2_position);
	glLightfv(GL_LIGHT2, GL_AMBIENT, 	light2_ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, 	light2_diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, 	light2_specular);

	glEnable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glEnable(GL_LIGHT2);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

   	glEnable (GL_DEPTH_TEST);
   	glEnable (GL_NORMALIZE);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glShadeModel (GL_SMOOTH);
}

void camRedisplay(){
	glutSetWindow(meshOut);
	glutPostRedisplay();
}

void renderStart(){
	calcPosCamera();
	initViewModel();
	initLight();
	camRedisplay();
}

void meshCallbacks(){
	
	glutSetWindow(meshOut);
	
	glutKeyboardFunc(&KeyPressed);
	glutReshapeFunc(&ReSizeMesh);
	glutDisplayFunc(&DrawMesh);	
	glutMouseFunc(&MouseMesh);
	glutMotionFunc(&MoveMesh);	
}

void initMesh(){
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	initVariables();
	initViewModel();
}

//=========================================================
// Cam OpenGL Functions
//=========================================================

void DrawCam(){

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	lockThread();

	while (!got_depth && !isTest())
		waintThread();

	uint8_t *tmp;

	if (got_depth && !isTest()){
		tmp = depth_front;
		depth_front = depth_mid;
		depth_mid = tmp;
		got_depth = false;
	}

	unlockThread();

	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);

	if(!copyActive)
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, depth_front);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, depth_copy);

	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glTexCoord2f(0, 0); glVertex3f(0,0,0);
	glTexCoord2f(1, 0); glVertex3f(640,0,0);
	glTexCoord2f(1, 1); glVertex3f(640,480,0);
	glTexCoord2f(0, 1); glVertex3f(0,480,0);
	glEnd();

	glutSwapBuffers();
}

void ReSizeCam(int Width, int Height){

	glViewport(0, 0, Width, Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 632, 480, 0, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void DrawCamFoced(){
	glutSetWindow(camOut);
	glutPostRedisplay();
}

void DrawCamAgain(){
	if(glutGetWindow() == camOut)
		glutPostRedisplay();
}

void camCallbacks(){
	
	glutSetWindow(camOut);
	
	glutKeyboardFunc(&KeyPressed);
	glutDisplayFunc(&DrawCam);
	glutReshapeFunc(&ReSizeCam);
	
	glutIdleFunc(DrawCamAgain);
}

void initCam(){

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_FLAT);

	glGenTextures(1, &gl_depth_tex);
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

//=========================================================
//
//=========================================================

void destroyOpenGL(){
	glutDestroyWindow(camOut);
	glutDestroyWindow(meshOut);
}
 
void *initOpenGL(int argc, char **argv){

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);

	glutInitWindowSize(640, 480);
	glutInitWindowPosition(707, 50);
	meshOut = glutCreateWindow("Saida da Malha");
	glutSetWindow(meshOut);

	initMesh();
	meshCallbacks();
	
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(50, 50);
	camOut = glutCreateWindow("Saida da Camera");
	glutSetWindow(camOut);

	initCam();
	camCallbacks();

	glutMainLoop();
	return NULL;
}

//=========================================================
//
//=========================================================

int main(int argc, char **argv){

	initKinect(argc, argv);
	initOpenGL(argc, argv);

	return 0;
}



