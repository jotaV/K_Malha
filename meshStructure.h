#ifndef MESHSTRUCTURE_H_
#define MESHSTRUCTURE_H_

#include <vector>

#ifndef ABS
	#define ABS(a)		(((a) < 0  ) ? -(a) : (a))
#endif

#ifndef MAX
	#define MAX(a,b)	(((a) > (b)) ?  (a) : (b))
#endif

using namespace std;

typedef struct{
	float x;
	float y;
	float z;
}Point;

typedef struct{
	float x;
	float y;
	float z;
	float nz;
}AuxPoint;

typedef struct{
	AuxPoint pto;
	Point normal;
	unsigned int ID;
	int	valencia;
	bool cross;
}Vertice;

typedef struct{
	unsigned int ID;
	int	indV[3];
}Face;

typedef struct{
	unsigned int ID;
	vector <Vertice*> vertices;
	vector <Face*>	faces;
}Group;

#endif /* MESHSTRUCTURE_H_ */
