#include "AbstractOut.h"

vector <AbstractOut*> AbstractOut::instance;

void AbstractOut::initOut(int num, const char *name){
    glutInitWindowSize(640, 480);
	glutInitWindowPosition(50 + 640*num + 15*num, 50);
	
    ID = glutCreateWindow(name);
    if(!instance.size()) instance.push_back(NULL);
    instance.insert(instance.begin()+ID, this);
    
    glutSetWindow(ID);    
    
    initGL();
    
    controlFunction();
}

/*control function
 -------------------------------------------------*/
void AbstractOut::initGL(){
    glClearColor (0.0, 0.0, 0.0, 0.0);
   
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
}

void AbstractOut::controlFunction(){
    glutSetWindow(ID);
    glutDisplayFunc(abstractDisplay);
    glutIdleFunc(&abstractIdle);
    glutReshapeFunc(&abstractReshape);
    glutKeyboardFunc(&abstractKeyboard);
	glutMouseFunc(&abstractMouse);
	glutMotionFunc(&abstractMotion);
}

void AbstractOut::outRedisplay(){
    glutSetWindow(ID);
    glutPostRedisplay();
}

/*abstract Functions
 -------------------------------------------------*/
void AbstractOut::abstractDisplay(){
    instance[glutGetWindow()]->display();
}

void AbstractOut::abstractIdle(){
    instance[glutGetWindow()]->idle();
}

void AbstractOut::abstractReshape(int width, int henght){
    instance[glutGetWindow()]->reshape(width, henght);
}

void AbstractOut::abstractKeyboard(unsigned char key, int x, int y){
    instance[glutGetWindow()]->keyboard(key, x, y);
}

void AbstractOut::abstractMouse(int button, int state, int x, int y){
    instance[glutGetWindow()]->mouse(button, state, x, y);
}

void AbstractOut::abstractMotion(int x, int y){
    instance[glutGetWindow()]->motion(x, y);
}

/*OpenGL Defaults
 -------------------------------------------------*/
void AbstractOut::display(){
    glClear(GL_COLOR_BUFFER_BIT);
    
    glColor3f(1.0, 1.0, 1.0);
    
    glBegin(GL_POLYGON);
        glVertex3f(0.5, 0.9, 0.0);
        glVertex3f(0.1, 0.1, 0.0);
        glVertex3f(0.9, 0.1, 0.0);
    glEnd();
    
    glutSwapBuffers();
}

void AbstractOut::idle(){}
void AbstractOut::reshape(int width, int henght){}
void AbstractOut::keyboard(unsigned char key, int x, int y){}
void AbstractOut::mouse(int button, int state, int x, int y){}
void AbstractOut::motion(int x, int y){}


/*                               GET & SET
 ****************************************************************************/

void AbstractOut::setKeyboardFunction(void (*func)(unsigned char, int, int)){
    glutSetWindow(ID);
    glutKeyboardFunc(func);
}

int AbstractOut::getID(){
    return ID;
}

AbstractOut* AbstractOut::getInstance(){
    return instance[glutGetWindow()];
}