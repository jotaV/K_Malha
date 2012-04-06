#include <iostream>
#include <string.h>

#if defined(__APPLE__) || defined(MACOSX)
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "CamOut.h"
#include "MeshOut.h"

using namespace std;

CamOut *camOut;
MeshOut *meshOut;

void initAllAction(int *argc, char** argv);
void Keyboard(unsigned char key, int x, int y);
void initGUI();

int menu2;

int main(int argc, char** argv){
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
   
   initAllAction(&argc, argv);
   
   glutMainLoop();
   return 0; 
}

void initAllAction(int *argc, char** argv){    
    camOut = (*argc >= 2 && !strcmp(argv[1], "-t")) ?
        new CamOut(true, initGUI) : new CamOut(false, initGUI);

    meshOut = new MeshOut(initGUI);
 
    camOut->setKeyboardFunction(Keyboard);
    meshOut->setKeyboardFunction(Keyboard);
}

void Keyboard(unsigned char key, int x, int y){
    
    switch(key){
        case 27:
            delete camOut;
            delete meshOut;
            exit(0);
            break;
        
        /*Controle de malha
         -------------------------------------*/
        case 'g': case 'G': //Geração de malha
            meshOut->gerateMesh(camOut->getMesh());
            break;
            
        case 'f': case 'F': //Geração de malha sem suavisação
            meshOut->gerateMesh(camOut->getMesh());
            break;
        
       case 19: // Crtl + S - Valvamento automatico
            meshOut->saveMesh(camOut->getTimeName());
            break;
        
        case 's': case 'S':{ //Salvar malha
            meshOut->saveMesh();
            break;
        }
        
        case 'q': case 'Q': //Trocar modo de desenho da malha
            meshOut->setPMode();
            break;
            
        /*Controle de camera
         -------------------------------------*/
        case 'i': case 'I':
            camOut->maxDistChange(-0.05f);
            break;
        
        case 'o': case 'O':
            camOut->maxDistChange(0.05f);
            break;
        
        case 'k': case 'K':
            camOut->minDistChange(-0.05f);
            break;
        
        case 'l': case 'L':
            camOut->minDistChange(0.05f);
            break;
        
        case 'p': case 'P':
            camOut->pauseDisplay();
            break;
    }
}

void menu(int OP){
	switch(OP){
		case OP_SMOOTH_OFF:
			if(MeshControl::SMOOTH_TYPE != 0){
                MeshControl::SMOOTH_TYPE = 0;
                
                glutChangeToMenuEntry(1, "ON  - Off      ",  OP_SMOOTH_OFF);
                glutChangeToMenuEntry(2, "OFF - Gaussian ",  OP_SMOOTH_GAUSSIAN);
                glutChangeToMenuEntry(3, "OFF - Laplacian",  OP_SMOOTH_LAPLACIAN);
            }
            break;		
		
		case OP_SMOOTH_GAUSSIAN:
            if(MeshControl::SMOOTH_TYPE != 1){
                MeshControl::SMOOTH_TYPE = 1;
                
                glutChangeToMenuEntry(1, "OFF - Off      ",   OP_SMOOTH_OFF);
                glutChangeToMenuEntry(2, "ON  - Gaussian ",   OP_SMOOTH_GAUSSIAN);
                glutChangeToMenuEntry(3, "OFF - Laplacian",   OP_SMOOTH_LAPLACIAN);
            }
            break;

		case OP_SMOOTH_LAPLACIAN:
			if(MeshControl::SMOOTH_TYPE != 2){
                MeshControl::SMOOTH_TYPE = 2;
                
                glutChangeToMenuEntry(1, "OFF - Off      ",   OP_SMOOTH_OFF);
                glutChangeToMenuEntry(2, "OFF - Gaussian ",   OP_SMOOTH_GAUSSIAN);
                glutChangeToMenuEntry(3, "ON  - Laplacian",   OP_SMOOTH_LAPLACIAN);}
            break;

		case OP_SMOOTH_REPEAT_0:
			MeshControl::SMOOTH_REPEAT = 0;
			break;

		case OP_SMOOTH_REPEAT_1:
			MeshControl::SMOOTH_REPEAT = 1;
			break;

		case OP_SMOOTH_REPEAT_2:
			MeshControl::SMOOTH_REPEAT = 2;
			break;

		case OP_SMOOTH_REPEAT_3:
			MeshControl::SMOOTH_REPEAT = 3;
			break;

		case OP_SAVE_OBJ:
			meshOut->saveMesh();
			break;

		case OP_QUIT:
			delete camOut;
            delete meshOut;
            exit(0);
			break;
	}
}


void initGUI(void){
    
	int mainMenu, 
		smSmoothType,
		smSmoothRepetition;

	smSmoothType = glutCreateMenu(menu);
	glutAddMenuEntry("OFF - Off      ",   OP_SMOOTH_OFF);
	glutAddMenuEntry("OFF - Gaussian ",   OP_SMOOTH_GAUSSIAN);
	glutAddMenuEntry("ON  - Laplacian",    OP_SMOOTH_LAPLACIAN);

	smSmoothRepetition = glutCreateMenu(menu);
	glutAddMenuEntry("tipo 1",  OP_SMOOTH_REPEAT_0);
	glutAddMenuEntry("tipo 2",  OP_SMOOTH_REPEAT_1);
	glutAddMenuEntry("tipo 3",  OP_SMOOTH_REPEAT_2);
	glutAddMenuEntry("tipo 4",  OP_SMOOTH_REPEAT_3);

	mainMenu = glutCreateMenu(menu);
	glutAddSubMenu("Smooth type",        smSmoothType);
	glutAddSubMenu("Smooth repetitions", smSmoothRepetition);
	glutAddMenuEntry("OBJ file save",    OP_SAVE_OBJ);
	glutAddMenuEntry("Quit",             OP_QUIT);
    
    menu2 = smSmoothRepetition;
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
}