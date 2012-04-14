#include "CamOut.h"

pthread_cond_t gl_frame_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER;

CamOut *CamOut::kinectCam;

CamOut::CamOut(bool test, void (*func)(void) = NULL) : AbstractOut(), MDist(0.3f), 
        mDist(0.1f), got_depth(false), copyActive(false){
    
    setKinectCam(this);
    timestamp[0] = "test";
    
    this->test = test;
    
    if(test){
        depth_front = new uint8_t[640*480*3];
        depth = new uint16_t[640*480];
        initTest();
    
    }else{
        depth_front = new uint8_t[640*480*3];
        depth_mid 	= new uint8_t[640*480*3];
        initKinect(); 
    }
    
    initOut(0, "camOut");
    //glutSetWindow(getID()); func();
    
    
}

/*                             FUNÇÔES INICIAÇÃO
 ****************************************************************************/

void CamOut::initTest(){
    if(!copyActive){
        for(int i = 0; i < 640*480; i++)
            depth[i] = (uint16_t) 2047;

        float calc = 0;
        for(int i = 160; i < 320; i++){
        for(int j = 100; j < 540; j++){
            calc = float(j)/639 - 0.5;
            depth[i*640+j] = (uint16_t) 400 + ceil(pow(calc*33,2));
        }}

        for (int i = 0; i < 640*480; i++){
            if(depth[i] <= 1535){
                depth_front[3*i+0] =
                depth_front[3*i+1] =
                depth_front[3*i+2] = (uint8_t) 255 - 255*(depth[i] - 320)/1215;
            }else depth_front[3*i+0] =  depth_front[3*i+1] =  depth_front[3*i+2] = 0;

            if(depth[i] < 1535*mDist || depth[i] > 1535*MDist){
                depth_front[3*i+1] -= depth_front[3*i+0]*0.75;
                depth_front[3*i+2] -= depth_front[3*i+0]*0.75;
            }
        }
    }
}

void CamOut::initKinect(){
    cout << "Kinect camera test" << endl;

	if (freenect_init(&f_ctx, NULL) < 0){
		cout << "Failed to load the Kinect\n" << endl;
        exit(0);
    }
	freenect_set_log_level(f_ctx, FREENECT_LOG_DEBUG);
	freenect_select_subdevices(f_ctx, (freenect_device_flags)
            (FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

	if ((freenect_open_device(f_ctx, &f_dev, 0) < 0) ||
		(pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL))){
		cout << "Failed to load the Kinect\n" << endl;
        exit(0);
    }
}

/*                             FUNÇÕES THREAD
 ****************************************************************************/

void* CamOut::freenect_threadfunc(void *arg){
    freenect_set_tilt_degs(kinectCam->f_dev, 0);
	freenect_set_led(kinectCam->f_dev,LED_RED);
	freenect_set_depth_callback(kinectCam->f_dev, depth_cb);
	/*freenect_set_video_mode(kinectCam->f_dev, freenect_find_video_mode(
            FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));*/
	freenect_set_depth_mode(kinectCam->f_dev, freenect_find_depth_mode(
            FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));

	freenect_start_depth(kinectCam->f_dev);
	freenect_start_video(kinectCam->f_dev);

	while (!(kinectCam->die) && freenect_process_events(kinectCam->f_ctx) >= 0){
		kinectCam->outRedisplay();
	}

	cout << "\nshutting down streams..." << endl;

	freenect_stop_depth(kinectCam->f_dev);
	freenect_stop_video(kinectCam->f_dev);

	freenect_close_device(kinectCam->f_dev);
	freenect_shutdown(kinectCam->f_ctx);

	cout << "-- done!" << endl;
	return NULL;
}

void CamOut::depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp){
    if(!kinectCam->copyActive){
        kinectCam->depth = (uint16_t*)v_depth;
        kinectCam->timestamp[0] = (unsigned int)timestamp;
    }
    
	pthread_mutex_lock(&gl_backbuf_mutex);
		for (int i = 0; i < 640*480; i++){

			if(kinectCam->depth[i] <= 1535){
				kinectCam->depth_mid[3*i+0] =
				kinectCam->depth_mid[3*i+1] =
				kinectCam->depth_mid[3*i+2] = (uint8_t) 255 - 255*(kinectCam->depth[i] - 320)/1215;

			}else
				kinectCam->depth_mid[3*i+0] =  kinectCam->depth_mid[3*i+1] =  kinectCam->depth_mid[3*i+2] = 0;

			if(kinectCam->depth[i] < (1215*(kinectCam->mDist) + 320) || kinectCam->depth[i] > (1215*(kinectCam->MDist) + 320)){
				kinectCam->depth_mid[3*i+1] -= kinectCam->depth_mid[3*i+0]*0.75;
				kinectCam->depth_mid[3*i+2] -= kinectCam->depth_mid[3*i+0]*0.75;
			}
		}

		kinectCam->got_depth = true;

	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

/*                             FUNÇÕES OPEMGL
 ****************************************************************************/

void CamOut::initGL(){
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

void CamOut::idle(){
    if(glutGetWindow() == getID())
        glutPostRedisplay();
}

void CamOut::reshape(int width, int heinght){
    glViewport(0, 0, width, heinght);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 640, 480, 0, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void CamOut::display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	lockThread();

	while (!(got_depth) && !(test))
		waintThread();

	uint8_t *tmp;

	if (got_depth && !(test)){
		tmp = depth_front;
		depth_front = depth_mid;
		depth_mid = tmp;
		got_depth = false;
	}

	unlockThread();

	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, depth_front);
	
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(0, 0); glVertex3f(0,0,0);
        glTexCoord2f(1, 0); glVertex3f(640,0,0);
        glTexCoord2f(1, 1); glVertex3f(640,480,0);
        glTexCoord2f(0, 1); glVertex3f(0,480,0);
	glEnd();

	glutSwapBuffers();
}

/*                             GET & SETS
 ****************************************************************************/

void CamOut::maxDistChange(float value){
	if(MDist + value <= 1.0f && MDist + value > mDist)
		MDist += value;
    
	if(test){
        initTest();
        outRedisplay();
    }
}

void CamOut::minDistChange(float value){
	if(mDist + value >= 0.0f && mDist + value < MDist)
		mDist += value;

	if(test){
        initTest();
        outRedisplay();
    }
}

void CamOut::pauseDisplay(){
    if(!copyActive){
        if(!test) lockThread();     
        //depth_copy = new u_int8_t[640*480*3];
        //memcpy(depth_copy, depth_front, 640*480*3);
        copyActive = true;
    }else{
        if(!test) unlockThread();
        //delete depth_copy;
        copyActive = false;
    }
    //if(test) outRedisplay();
}

void CamOut::lockThread(){
	if(!test){ 
        pthread_mutex_lock(&gl_backbuf_mutex);
        cout << "Thread travado" << endl;
    }
}

void CamOut::unlockThread(){
	if(!test){
        pthread_mutex_unlock(&gl_backbuf_mutex);
        cout << "Thread travado" << endl;
    }
}

void CamOut::waintThread(){
	if(!test){
        pthread_cond_wait(&gl_frame_cond, &gl_backbuf_mutex);
        cout << "Thread travado" << endl;
    }
}

void CamOut::setKinectCam(CamOut *cam){
    CamOut::kinectCam = cam;
}

uint16_t* CamOut::getMesh(){
    
    uint16_t* mesh = new uint16_t[640*480];
    timestamp[1] = timestamp[0];
            
    lockThread();
            memcpy(mesh, depth, 640*480);
    unlockThread();        
    
    for(unsigned int i = 0; i < 640*480; i++){
        if(mesh[i] < 1535*mDist || mesh[i] > 1535*MDist){
            mesh[i] = 2047;
        }
    }
    
    return mesh;
}

string CamOut::getTimeName(){
    return timestamp[1];
}

/*                             DESTRUIDOR
 ****************************************************************************/

CamOut::~CamOut(){
    glutDestroyWindow(getID());
    
    delete depth_front; 
    if(depth_mid) depth_mid;
    if(depth_copy) depth_copy;
    if(test) delete depth;
}
