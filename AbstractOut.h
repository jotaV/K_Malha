#ifndef ABSTRACTOUT_H
#define	ABSTRACTOUT_H

#include <iostream>
#include <string>
#include <vector>

#if defined(__APPLE__) || defined(MACOSX)
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

using namespace std;

class AbstractOut {
    
private:
    int ID;

protected:
    static vector <AbstractOut*> instance;

public:
    
    void initOut(int num, const char *name);
    
    void controlFunction();
    void outRedisplay();
    
    static void abstractDisplay();
    static void abstractIdle();
    static void abstractReshape(int width, int henght);
    static void abstractKeyboard(unsigned char key, int x, int y);
    static void abstractMouse(int button, int state, int x, int y);
    static void abstractMotion(int x, int y);
    
    virtual void initGL();
    virtual void display();
    virtual void idle();
    virtual void reshape(int width, int henght);
    virtual void keyboard(unsigned char key, int x, int y);
    virtual void mouse(int button, int state, int x, int y);
    virtual void motion(int x, int y);
    
    void setKeyboardFunction(void (*func)(unsigned char, int, int));
    int getID();
    static AbstractOut* getInstance();
};

#endif	/* ABSTRACTOUT_H */

