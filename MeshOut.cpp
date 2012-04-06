#include "MeshOut.h"

MeshOut::MeshOut(void (*func)(void) = NULL) : AbstractOut(), winWidth(640), 
        winHeinght(480), pMode(GL_LINE), start(false){
        
    MeshControl::SMOOTH_REPEAT = 1;
    MeshControl::SMOOTH_TYPE = 2;
    
    initOut(1, "MeshOut");
    
    glutSetWindow(getID()); func();
}

/*                           View Model Functions
 ----------------------------------------------------------------------------*/

void MeshOut::initVariables(){
    
    pointMax(640, 480, 1535);
    pointMin(0, 0, 0);
    pointCenter(320, 240, 767.5);
    camView(0, 0, 0);
}

void MeshOut::renderStart(){
    
    calcPosCamera();
    initViewModel();
    if(!start) initLight();
    outRedisplay();
    start = true;
}

void MeshOut::calcPosCamera(){
    
    pointCenter.x = (pointMin.x + pointMax.x) / 2;
	pointCenter.y = (pointMin.y + pointMax.y) / 2;
	pointCenter.z = (pointMin.z + pointMax.z) / 2;
    
    pointMax.x += pointCenter.x*0.3;
	pointMax.y += pointCenter.y*0.3;
	pointMax.z += pointCenter.z*0.3;
	
	pointMin.x -= pointCenter.x*0.3;
	pointMin.y -= pointCenter.y*0.3;
	pointMin.z -= pointCenter.z*0.3;	    
    
	//float tanFOV = tan(120.0 * 3.1416 / 180.0);
	float bigDimension = MAX((pointMax.x - pointMin.x), (pointMax.y - pointMin.y));
	
	camView.x = pointCenter.x;
	camView.y = pointCenter.y;
	camView.z = pointCenter.z - (bigDimension);
}

void MeshOut::initViewModel(){
    vm.reset();
	vm.eye.set(camView.x, camView.y, camView.z );
	vm.lookat.set(pointCenter.x, pointCenter.y, pointCenter.z);
	vm.set_up(0.0, 1.0, 0.0);
	vm.update();
}

void MeshOut::initLight(){
    glutSetWindow(getID());
	
	GLfloat mat_diffuse[] 		= {1.0, 1.0, 1.0, 1.0};
	GLfloat mat_specular[] 		= {0.5, 0.5, 0.5, 1.0};
	GLfloat mat_ambient[] 		= {0.3, 0.3, 0.3, 1.0};
	GLfloat mat_shininess[] 	= {150.0};
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, 	mat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, 	mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, 	mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS,	mat_shininess);

	GLfloat light0_position[] 	= {pointCenter.x, pointCenter.y, 
                                    camView.z, 1.0f};
    
	GLfloat light0_ambient[] 	= {0.25f, 0.0f, 0.0f, 1.0f};
	GLfloat light0_diffuse[] 	= {1.0f, 0.0f, 0.0f, 1.0f};
	GLfloat light0_specular[] 	= {0.5f, 0.0f, 0.0f, 1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, 	light0_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, 	light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, 	light0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, 	light0_specular);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

   	glEnable (GL_DEPTH_TEST);
   	glEnable (GL_NORMALIZE);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glShadeModel(GL_SMOOTH);
}

/*                              OpenGL Function
 ----------------------------------------------------------------------------*/

void MeshOut::initGL(){
    glClearColor(0.0, 0.0, 0.0, 0.0);
    initVariables();
    initViewModel();
}

void MeshOut::drawAxis(){

	glDisable (GL_LIGHTING);
	
	glBegin(GL_LINES);        
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(pointMax.x, pointCenter.y, pointCenter.z);
		glVertex3f(pointCenter.x, pointCenter.y, pointCenter.z);
		
		if(pointMin.x < pointCenter.x){
			glColor3f(0.5, 0.0, 0.0);
			glVertex3f(pointCenter.x, pointCenter.y, pointCenter.z);
			glVertex3f(pointMin.x, pointCenter.y, pointCenter.z);
		}

		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(pointCenter.x, pointMax.y, pointCenter.z);
		glVertex3f(pointCenter.x, pointCenter.y, pointCenter.z);
		
		if(pointMin.y < pointCenter.y){
			glColor3f(0.0, 0.5, 0.0);
			glVertex3f(pointCenter.x, pointCenter.y, pointCenter.z);
			glVertex3f(pointCenter.x, pointMin.y, pointCenter.z);
			}

		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(pointCenter.x, pointCenter.y, pointMax.z);
		glVertex3f(pointCenter.x, pointCenter.y, pointCenter.z);
		
		if(pointMin.z < pointCenter.z ){
			glColor3f(0.0, 0.0, 0.5);
			glVertex3f(pointCenter.x, pointCenter.y, pointCenter.z);
			glVertex3f(pointCenter.x, pointCenter.y, pointMin.z);
			}

	glEnd();

	glEnable (GL_LIGHTING);
}

void MeshOut::display(){
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluPerspective(60.0, glutGet(GLUT_WINDOW_WIDTH)/glutGet(GLUT_WINDOW_HEIGHT), 
                                                        0.01, 20.0*pointMax.z);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glViewport(0, 0, winWidth, winHeinght);    
	
	vm.load_to_openGL_noident();
	
	drawAxis();
	drawObjects();
		
	glutSwapBuffers();
}

void MeshOut::drawObjects(){
    glPolygonMode(GL_FRONT_AND_BACK, pMode);
	glBegin(GL_TRIANGLES);
	
    for(int g = 0; g < groups.size(); g++){
    for(unsigned int f = 0; f < groups[g]->faces.size(); f++){ 
    for(unsigned int iv = 0; iv < 3; iv++){
        glNormal3f(groups[g]->vertices[groups[g]->faces[f]->verticesID[iv] -1]->normal.x,
                   groups[g]->vertices[groups[g]->faces[f]->verticesID[iv] -1]->normal.y,
                   groups[g]->vertices[groups[g]->faces[f]->verticesID[iv] -1]->normal.z
                  );
        glVertex3f(groups[g]->vertices[groups[g]->faces[f]->verticesID[iv] -1]->point.x,
                   groups[g]->vertices[groups[g]->faces[f]->verticesID[iv] -1]->point.y,
                   groups[g]->vertices[groups[g]->faces[f]->verticesID[iv] -1]->point.z
                  );
    }}}
    
    glEnd();
}

void MeshOut::reshape(int width, int henght){
	winWidth = width;
	winHeinght = henght;
	glViewport(0, 0, winWidth, winHeinght); 
}

void MeshOut::mouse(int button, int state, int x, int y){
	if ( button == GLUT_LEFT_BUTTON  ) {
		if 	(state == GLUT_DOWN ){
			old_x          = x;
			old_y          = y;
			curr_button    = button;
			curr_modifiers = glutGetModifiers();
        }
    }
	
    outRedisplay();
}

void MeshOut::motion(int x, int y){
	float move_x, move_y;
	int   delta_x, delta_y;
		
	if (curr_button == GLUT_LEFT_BUTTON){
		delta_x = x - old_x;
		delta_y = y - old_y;
		move_x = (float) -delta_x;
		move_y = (float) -delta_y;
		
		if(curr_modifiers & GLUT_ACTIVE_SHIFT){
                vm.move(move_x, -move_y, 0.0);
		}else{ 
			if(curr_modifiers & GLUT_ACTIVE_CTRL){
				vm.roll(move_x * 0.05);	  
				vm.move(0.0, 0.0, -move_y);	
			}else{
				vm.eye_yaw(move_x);
				vm.eye_pitch(move_y);
			}
		}
        
		old_x = x;
		old_y = y;
		
        outRedisplay();
	}
}

/*                             MeshControl Calls
 ----------------------------------------------------------------------------*/
void MeshOut::gerateMesh(uint16_t *mesh){
    
    
    if(groups.size() == 0)
        groups.push_back(MeshControl::gerateGroup(mesh, 1));
    else{
        delete groups[0];
        groups[0] = MeshControl::gerateGroup(mesh, 1);
    }
    
    setLimitPoints();
    
    renderStart();
}

void MeshOut::saveMesh(){
    string arquivo;
	cout << "Digite o nome: ";
	cin >> arquivo;
    
    saveMesh(arquivo);
}

void MeshOut::saveMesh(string name){
    if(groups.size() > 0) MeshControl::saveGroups(groups[0], name);
}

void MeshOut::setLimitPoints(){
    
    pointMin(MeshControl::pointMin.x,
             MeshControl::pointMin.y,
             MeshControl::pointMin.z);
    
    pointMax(MeshControl::pointMax.x,
             MeshControl::pointMax.y,
             MeshControl::pointMax.z);
}

void MeshOut::calcBoundingBox(){

    if(MeshControl::pointMin.x < pointMin.x){
        pointMin.x = MeshControl::pointMin.x;
    
    }else pointMax.x = MeshControl::pointMin.x < pointMax.x ? 
                        MeshControl::pointMin.x : pointMax.x; 
    
    if(MeshControl::pointMin.y < pointMin.y){
        pointMin.y = MeshControl::pointMin.y;
    
    }else pointMax.y = MeshControl::pointMin.y < pointMax.y ? 
                        MeshControl::pointMin.y : pointMax.y;
    
    if(MeshControl::pointMin.z < pointMin.z){
        pointMin.z = MeshControl::pointMin.z;
    
    }else pointMax.z = MeshControl::pointMin.z < pointMax.z ? 
                        MeshControl::pointMin.z : pointMax.z;

}

/*                              Seter & Geter
 ----------------------------------------------------------------------------*/

void MeshOut::setPMode(){
    switch(pMode){
        case GL_LINE:
            pMode = GL_FILL;
            break;
        case GL_FILL:
            pMode = GL_POINT;
            break;
        case GL_POINT:
            pMode = GL_LINE;
            break;
    }
    outRedisplay();
}
