#ifndef MESHOUT_H
#define	MESHOUT_H

#include "AbstractOut.h"
#include "MeshControl.h"
#include "libView/ViewModel.h"

class MeshOut : public AbstractOut{
private:
    ViewModel vm; 
   
    vector <Group*> groups;
    Point pointMin, pointMax, pointCenter, camView;
    
    GLint winWidth, winHeinght;
    GLenum pMode;
            
    int old_x, old_y, curr_modifiers, curr_button;
    
    bool start;
public:
    MeshOut(void (*func)(void));
    
    void initVariables();
    void initViewModel();
    void renderStart();
    void calcPosCamera();
    void initLight();
    
    void initGL();
    void display();
    void reshape(int width, int henght);
    void mouse(int button, int state, int x, int y);
    void motion(int x, int y);
    
    void drawAxis();
    void drawObjects();
    
    void gerateMesh(uint16_t *mesh);
    void saveMesh();
    void saveMesh(string name);
    void setLimitPoints();
    void resetLigth();
    void calcBoundingBox();
    
    void setPMode();
    
    //void calcBoundingBox();

};

#endif	/* MESHOUT_H */

