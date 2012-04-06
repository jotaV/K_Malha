#ifndef CAMOUT_H
#define	CAMOUT_H

#include <pthread.h>
#include <string.h>
#include <math.h>
#include <sstream>

#include "libfreenect.h"
#include "AbstractOut.h"

class CamOut;

class CamOut : public AbstractOut {
private:
    float MDist;
	float mDist;
    
    GLuint gl_depth_tex;
    pthread_t freenect_thread;
    
    freenect_context *f_ctx;
    freenect_device *f_dev;
    
    uint8_t *depth_front; 
    uint8_t *depth_mid;
    uint8_t *depth_copy;
    uint16_t *depth;
    
    string timestamp[2];
      
    bool test;
    bool die;
    bool got_depth;
    bool copyActive;
    
protected:
    static CamOut *kinectCam;
    
public:
    CamOut(bool test, void (*func)(void));
    virtual ~CamOut();
    
    void initTest();
    void initKinect();
    
    static void* freenect_threadfunc(void *arg);
    static void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp);
    
    void initGL();
    void display();
    void idle();
    void reshape(int width, int heinght);
    
    void maxDistChange(float value);
    void minDistChange(float value);
    void pauseDisplay();
    void lockThread();
    void unlockThread();
    void waintThread();
    uint16_t* getMesh();
    string getTimeName();
    
    static void setKinectCam(CamOut *cam);
};

#endif	/* CAMOUT_H */

