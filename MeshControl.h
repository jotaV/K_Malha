#ifndef MESHCONTROL_H
#define	MESHCONTROL_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#define OP_SMOOTH_OFF       01
#define OP_SMOOTH_GAUSSIAN  02
#define OP_SMOOTH_LAPLACIAN 03
#define OP_SMOOTH_REPEAT_0  10
#define OP_SMOOTH_REPEAT_1  11
#define OP_SMOOTH_REPEAT_2  12
#define OP_SMOOTH_REPEAT_3  13
#define OP_SAVE_OBJ         20
#define OP_QUIT             30


using namespace std;

class Point{
public:
	float x;
	float y;
	float z;
    void operator() (float x, float y, float z);
};

class AuxPoint: public Point{
public:
	float nz;
    void operator() (float x, float y, float z, float nz);
};

class Vertice{
public:
    unsigned int ID;
	AuxPoint     point;
	Point        normal;
    int          valencia;
	bool         cross;
};

class Face{
public:
	unsigned int ID;
	int          verticesID[3];
    Face(int ID1, int ID2, int ID3, int ID);
    void operator() (int ID1, int ID2, int ID3, int ID);
};

class Group{
public:
	unsigned int ID;
	vector <Vertice*> vertices;
	vector <Face*>    faces;
};

class MeshControl{
public:
    static unsigned int *IDMatrix;
    static Point pointMin, pointMax;
    static int SMOOTH_TYPE, SMOOTH_REPEAT;
    
    static Group* gerateGroup(uint16_t *mesh, int var);
    static void gerateVertices(Group *group, uint16_t *mesh, int var);
    static void gerateFaces(Group *group, int var);
    static void gerateNormals(Group *group);
        static void calcValencia(Group *group, Face *f);
    
    static void saveGroups(vector <Group*> g, string arquvio);
    static void saveGroups(Group* group, string arquivo);
    
    static void smoothGaussian(uint16_t **mesh, int var);
    static void smoothLaplacian(Group* group, int var);
        
    static void calcBoungingBox(Vertice *v);
};

#endif	/* MESHCONTROL_H */
