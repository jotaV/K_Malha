//============================================================================
// Name        : camSettings.cpp
// Author      : Jv
// Version     : Pré-Beta
// Description : Gerador de Malha usando o dispositivo Kinect
//============================================================================

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include "libfreenect.h"
	
#include "externalVar.h"
#include "K_malha.h"

using namespace std;

pthread_cond_t gl_frame_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t freenect_thread;

freenect_context *f_ctx;
freenect_device *f_dev;

float 	MDist = 1.0f,
		mDist = 0.0f;

bool 	test = false,
		die = false;

//=========================================================
// Other
//=========================================================

void exitKinect(){

	freenect_set_led(f_dev,LED_BLINK_GREEN);
	die = true;

	pthread_join(freenect_thread, NULL);
	
	destroyOpenGL();
	
	delete depth_mid;
	delete depth_front;
	delete depth_mesh;
	delete depth_copy;

	exit(0);
}

void meshcpy(uint16_t depth_mesh[], uint16_t depth[], int w, int h){
	for(int i = 0; i < h; i++)
	for(int j = 0; j < w; j++){

		if(depth[i*w +j] >= (uint16_t)(1215*(mDist) + 320) &&
			depth[i*w +j] <= (uint16_t)(1215*(MDist) + 320)){
				
			depth_mesh[i*w +j] = depth[i*w +j];
				
		}else depth_mesh[i*w +j] = 2047;
	}
}

void fallKinect(){

	cout << "Falha ao carregar o Kinect" << endl;
	exit(0);
}

void takePicture(){

	if(!copyActive){
		memcpy(depth_copy, depth_front, 640*480*3);
		meshcpy(depth_mesh, depth, 640, 480);
		copyActive = true;

	}else{
		copyActive = false;

	}
}

//=========================================================
// Test Kinect
//=========================================================

void carregarTest(){
	for(int i = 0; i < 640*480; i++)
		depth[i] = (uint16_t) 2047;

	float calc;
	for(int i = 159; i < 320; i++)
	for(int j = 99; j < 540; j++){
			calc = 72*j/640 -36;
			depth[i*640+j] = 400 + pow(calc, 2);
	}

	test = true;
	for (int i = 0; i<640*480; i++){
		if(depth[i] <= 1535){
			depth_front[3*i+0] =
			depth_front[3*i+1] =
			depth_front[3*i+2] = (uint8_t) 255 - 255*(depth[i] - 320)/1215;
		}else depth_front[3*i+0] =  depth_front[3*i+1] =  depth_front[3*i+2] = 0;

		if(depth[i] < (1215*(mDist) + 320) || depth[i] > (1215*(MDist) + 320)){
			depth_front[3*i+1] -= depth_front[3*i+0]*0.75;
			depth_front[3*i+2] -= depth_front[3*i+0]*0.75;
		}
	}
}

//=========================================================
// Thread Functions
//=========================================================

void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp){

	depth = (uint16_t*)v_depth;

	pthread_mutex_lock(&gl_backbuf_mutex);
		for (int i = 0; i < 640*480; i++){

			if(depth[i] <= 1535){
				depth_mid[3*i+0] =
				depth_mid[3*i+1] =
				depth_mid[3*i+2] = (uint8_t) 255 - 255*(depth[i] - 320)/1215;

			}else
				depth_mid[3*i+0] =  depth_mid[3*i+1] =  depth_mid[3*i+2] = 0;

			if(depth[i] < (1215*(mDist) + 320) || depth[i] > (1215*(MDist) + 320)){
				depth_mid[3*i+1] -= depth_mid[3*i+0]*0.75;
				depth_mid[3*i+2] -= depth_mid[3*i+0]*0.75;
			}
		}

		got_depth = true;

	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}


void *freenect_threadfunc(void *arg){

	freenect_set_tilt_degs(f_dev, 0);
	freenect_set_led(f_dev,LED_RED);
	freenect_set_depth_callback(f_dev, depth_cb);
	freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
	freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));

	freenect_start_depth(f_dev);
	freenect_start_video(f_dev);

	while (!die && freenect_process_events(f_ctx) >= 0){
		DrawCamAgain();
	}

	cout << "\nshutting down streams..." << endl;

	freenect_stop_depth(f_dev);
	freenect_stop_video(f_dev);

	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);

	cout << "-- done!" << endl;
	return NULL;
}

//=========================================================
//Gets & Sets & Other
//=========================================================

void MaxDistChange(float value){
	if(MDist + value <= 1.0f || MDist + value > mDist)
		MDist += value;

	if(test) carregarTest();
}

void MinDistChange(float value){
	if(mDist + value >= 0.0f || mDist + value < MDist)
		MDist += value;

	if(test) carregarTest();
}

void lockThread(){
	if(!test) pthread_mutex_lock(&gl_backbuf_mutex);
}

void unlockThread(){
	if(!test) pthread_mutex_unlock(&gl_backbuf_mutex);
}

void waintThread(){
	if(!test) pthread_cond_wait(&gl_frame_cond, &gl_backbuf_mutex);
}

void kinectFalha(){

	cout << "Falha ao carregar o Kinect\n";
	exit(0);
}

//=========================================================
// "Main"
//=========================================================

void initKinect(int argc, char **argv){

	MDist = 0.3f;
	mDist = 0.1f;
	
	depth_mid 	= new uint8_t[640*480*3];
	depth_front = new uint8_t[640*480*3];
	depth_copy 	= new uint8_t[640*480*3];
	depth_mesh 	= new uint16_t[640*480];

	got_depth =
	copyActive = false;
	
	if(argc >= 2 && !strcmp(argv[1], "-t")){
		
		depth = new uint16_t[640*480];
		carregarTest();
	
	}else{
		
		cout << "Kinect camera test" << endl;

		if (freenect_init(&f_ctx, NULL) < 0)
			kinectFalha();

		freenect_set_log_level(f_ctx, FREENECT_LOG_DEBUG);
		freenect_select_subdevices(f_ctx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

		if ((freenect_open_device(f_ctx, &f_dev, 0) < 0) ||
			(pthread_create(&freenect_thread, NULL, freenect_threadfunc, NULL)))
			kinectFalha();

	}
}
