#ifndef CAMSETTINGS_H_
#define CAMSETTINGS_H_

void exitKinect();
void takePicture();
void initKinect(int argc, char **argv);
void MaxDistChange(float value);
void MinDistChange(float value);
void lockThread();
void unlockThread();
void waintThread();
void meshcpy(uint16_t depth_mesh[], uint16_t depth[], int w, int h);

#endif /* CAMSETTINGS_H_ */
