#include "MeshControl.h"

void Point::operator()(float x, float y, float z){
    this->x = x;
    this->y = y;
    this->z = z;
}

void Face::operator()(int ID1, int ID2, int ID3, int ID){
   
    this->verticesID[0] = ID1;
    this->verticesID[1] = ID2;
    this->verticesID[2] = ID3;
    this->ID = ID;
}

Face::Face(int ID1, int ID2, int ID3, int ID){
    
    this->verticesID[0] = ID1;
    this->verticesID[1] = ID2;
    this->verticesID[2] = ID3;
    this->ID = ID;
}

void AuxPoint::operator()(float x, float y, float z, float nz){
    this->x = x;
    this->y = y;
    this->z = z;
    this->nz = nz;
}

/*Statics atributes*/
unsigned int *MeshControl::IDMatrix;
Point MeshControl::pointMin, MeshControl::pointMax;
int MeshControl::SMOOTH_TYPE = 0, MeshControl::SMOOTH_REPEAT = 1;

/*Statics methods for gerate Mesh
 ------------------------------------------------------------*/

Group* MeshControl::gerateGroup(uint16_t *mesh, int var = 1){
    
    IDMatrix = new unsigned int[640*480];
    Group *group = new Group;
    
    pointMin(640,480,1535);
    pointMax(0,0,0);
    
    int form[4];
    
    if(SMOOTH_TYPE == 1){
        form[0] = 3; form[1] = 0; form[2] = 1; form[3] = 2;
    }else if(SMOOTH_TYPE == 2){
        form[0] = 0; form[1] = 4; form[2] = 1; form[3] = 2;
    }else{
        form[0] = 0; form[1] = 1; form[2] = 2; form[3] = -1;
    }
    
    for(int k = 0; k < 4; k++){
        
        switch(form[k]){
            case 0:{
                cout << "   generating Vertices" << endl;
                gerateVertices(group, mesh, var);
            }break;

            case 1:{
                cout << "   generating Faces" << endl;
                gerateFaces(group, var);
            }break;

            case 2:{
                cout << "   generating Normals" << endl;
                gerateNormals(group);
            }break;

            case 3:{
                cout << "   Smoothing" << endl;
                int auxVector[] = {1, 3, 5, 10};

                for(int i = 0; i < auxVector[SMOOTH_REPEAT]; i++)
                    smoothGaussian(&mesh, var);
            }break;

            case 4:{
                cout << "   Smoothing" << endl;
                int auxVector[] = {10, 25, 50, 100};

                for(int i = 0; i < auxVector[SMOOTH_REPEAT]; i++){
                    smoothLaplacian(group, var);
                    for(int j = 0; j < group->vertices.size(); j++)
                        group->vertices[j]->point.z = group->vertices[j]->point.nz;
                }
            }break;
        }
    }

    cout << endl;
    delete IDMatrix;
    return group;
}

void MeshControl::gerateVertices(Group *group, uint16_t *mesh, int var){
    
    Vertice *v;
	
	unsigned int k = 1;
	for(unsigned int i = 0; i < 480; i+=var)
	for(unsigned int j = 0; j < 640; j+=var){
		
		if(mesh[i*640 + j] != 2047){
		
			v = new Vertice;
			
			v->point(j+1, i+1, mesh[i*640 + j] - 320, 0);
            v->normal(0, 0, 0);
            v->valencia = 0;
			
			v->ID = k;
			IDMatrix[i*640 + j] = k;
			
			group->vertices.push_back(v);
			k++;
		}
		else IDMatrix[i*640 + j] = 0;
	}
}

void MeshControl::gerateFaces(Group *group, int var){
    
    Face *f;
    unsigned int square[4], k = 1;
	bool PAR = false;

	for(unsigned int i = 0; i < 480-var; i+=var){
		for(unsigned int j = 0; j < 640-var; j+=var){
            
			PAR = !PAR;
			square[0] = IDMatrix[i*640 + j];				//Current - 0
			square[1] = IDMatrix[(i+var)*640 + j];			//Lower - 1
			square[2] = IDMatrix[(i+var)*640 + (j+var)]; 	//Next Lower - 2
			square[3] = IDMatrix[i*640 + (j+var)];          //Next - 3
            
			if(PAR){
                if(square[3] && square[1]){
					if(square[0]){
                        f = new Face(square[0], square[1], square[3], k);
                        group->faces.push_back(f);
						k++;
					}
					if(square[2]){
						f = new Face(square[1], square[2], square[3], k);
                        group->faces.push_back(f);
						k++;
                        //Calcular XorMore;
					}
				}
			
            }else{
                if(square[2] && square[0]){
					if(square[1]){		
						f = new Face(square[0], square[1], square[2], k);
                        group->faces.push_back(f);
						k++;	
                        //Calcular XorMore;
					}					
					if(square[3]){		
						f = new Face(square[0], square[2], square[3], k);
                        group->faces.push_back(f);
						k++;
                        //Calcular XorMore;
					}
				}
			}
            
            bool cross = PAR;
            for(int k = 0; k < k; i++){
                if(square[k] != 0){
                    group->vertices[square[i]-1]->cross = cross;
                    cross = !cross;
                }
            }
		}
	}
}

void MeshControl::gerateNormals(Group *group){
    
    for (unsigned int i = 0 ; i < group->faces.size(); i++)
		calcValencia(group, group->faces[i]);
	
	for (unsigned int i = 0 ; i < group->vertices.size(); i++){
		group->vertices[i]->normal.x /= group->vertices[i]->valencia;
		group->vertices[i]->normal.y /= group->vertices[i]->valencia;
		group->vertices[i]->normal.z /= group->vertices[i]->valencia;

		float norma = sqrt(pow(group->vertices[i]->normal.x, 2) +
							pow(group->vertices[i]->normal.y, 2) +
							pow(group->vertices[i]->normal.z, 2));

		group->vertices[i]->normal.x /= norma;
		group->vertices[i]->normal.y /= norma;
		group->vertices[i]->normal.z /= norma;
		
		calcBoungingBox(group->vertices[i]);
	}
}

void MeshControl::calcValencia(Group *group, Face *f){
    
    Point v1, v2, normal;

	v1( group->vertices[f->verticesID[1]-1]->point.x -
        group->vertices[f->verticesID[0]-1]->point.x,
        
        group->vertices[f->verticesID[1]-1]->point.y -
		group->vertices[f->verticesID[0]-1]->point.y,
	
        group->vertices[f->verticesID[1]-1]->point.z -
		group->vertices[f->verticesID[0]-1]->point.z);

	v2( group->vertices[f->verticesID[2]-1]->point.x -
		group->vertices[f->verticesID[0]-1]->point.x,
        
        group->vertices[f->verticesID[2]-1]->point.y -
		group->vertices[f->verticesID[0]-1]->point.y,
	
        group->vertices[f->verticesID[2]-1]->point.z -
		group->vertices[f->verticesID[0]-1]->point.z);

	normal( (v1.y * v2.z - v2.y * v1.z),
            (v1.z * v2.x - v2.z * v1.x),
            (v1.x * v2.y - v2.x * v1.y));

	group->vertices[f->verticesID[0]-1]->normal.x += normal.x;
	group->vertices[f->verticesID[0]-1]->normal.y += normal.y;
	group->vertices[f->verticesID[0]-1]->normal.z += normal.z;
	group->vertices[f->verticesID[0]-1]->valencia++;

	group->vertices[f->verticesID[1]-1]->normal.x += normal.x;
	group->vertices[f->verticesID[1]-1]->normal.y += normal.y;
	group->vertices[f->verticesID[1]-1]->normal.z += normal.z;
	group->vertices[f->verticesID[1]-1]->valencia++;

	group->vertices[f->verticesID[2]-1]->normal.x += normal.x;
	group->vertices[f->verticesID[2]-1]->normal.y += normal.y;
	group->vertices[f->verticesID[2]-1]->normal.z += normal.z;
	group->vertices[f->verticesID[2]-1]->valencia++;
}

void MeshControl::calcBoungingBox(Vertice *v){
    
    pointMin.x = v->point.x < pointMin.x ? v->point.x : pointMin.x;
    pointMax.x = v->point.x > pointMax.x ? v->point.x : pointMax.x;
    
    pointMin.y = v->point.y < pointMin.y ? v->point.y : pointMin.y;
    pointMax.y = v->point.y > pointMax.y ? v->point.y : pointMax.y;
    
    pointMin.z = v->point.z < pointMin.z ? v->point.z : pointMin.z;
    pointMax.z = v->point.z > pointMax.z ? v->point.z : pointMax.z;
}

/*Statics methods for save Mesh
 ------------------------------------------------------------*/

void MeshControl::saveGroups(vector <Group*> groups, string arquivo){
    
    Vertice *v;
	Face 	*f;
    Group   *g;
    
    string comando;

	arquivo += ".obj";
	ofstream fout(arquivo.c_str());
    
    for(int i = 0; i < groups.size(); i++){
        g = groups[0];
        
        fout << "g " << i << endl;
        fout << endl;
        
        cout << "\nGravando vertices...\n";

        for(unsigned int j = 0; j < g->vertices.size(); j++){
            v = g->vertices[j];
            fout << "v " << v->point.x << " " << v->point.y << " " 
                    << v->point.z << endl;
        }
        
        fout << endl;

        cout << "Gravando normais...\n";

        for(unsigned int j = 0; j < g->vertices.size(); j++){
            v = g->vertices[j];
            fout << "vn " << v->normal.x << " " << v->normal.y << " " 
                    << v->normal.z << endl;
        }
        
        fout << endl;

        cout << "Gravando Faces...\n";
        
        for(unsigned int j = 0; j < g->faces.size(); j++){
            f = g->faces[j];
            fout << "f " << f->verticesID[0] << "//" << f->verticesID[0] << " "
                         << f->verticesID[1] << "//" << f->verticesID[1] << " "
                         << f->verticesID[2] << "//" << f->verticesID[2] << endl;
        }

        comando.append("mv ");
        comando.append(arquivo);
        comando.append(" OBJs/");
        comando.append(arquivo);

        system("mkdir -p OBJs");
        system(comando.c_str());
        
        cout << "\nFINALIZADO\n";
    }
}

void MeshControl::saveGroups(Group* group, string arquivo){
    
    Vertice *v;
	Face 	*f;
    
    string comando;

	arquivo += ".obj";
	ofstream fout(arquivo.c_str());
    
    cout << "\nGravando vertices...\n";

    for(unsigned int j = 0; j < group->vertices.size(); j++){
        v = group->vertices[j];
        fout << "v " << v->point.x << " " << v->point.y << " " 
                << v->point.z << endl;
    }

    fout << endl;

    cout << "Gravando normais...\n";

    for(unsigned int j = 0; j < group->vertices.size(); j++){
        v = group->vertices[j];
        fout << "vn " << v->normal.x << " " << v->normal.y << " " 
                << v->normal.z << endl;
    }

    fout << endl;

    cout << "Gravando Faces...\n";

    for(unsigned int j = 0; j < group->faces.size(); j++){
        f = group->faces[j];
        fout << "f " << f->verticesID[0] << "//" << f->verticesID[0] << " "
                     << f->verticesID[1] << "//" << f->verticesID[1] << " "
                     << f->verticesID[2] << "//" << f->verticesID[2] << endl;
    }

    comando.append("mv ");
    comando.append(arquivo);
    comando.append(" OBJs/");
    comando.append(arquivo);

    system("mkdir -p OBJs");
    system(comando.c_str());

    cout << "\nFINALIZADO\n";
}

/*Statics methods to smoothing
 ------------------------------------------------------------*/

void MeshControl::smoothGaussian(uint16_t **mesh, int var){
    
    float map[] = { 1,  2,  3,  2,  1,
                    2,  4,  5,  4,  2,
                    3,  5,  7,  5,  3, 
                    2,  4,  5,  4,  2,
                    1,  2,  3,  2,  1,};
    
    uint16_t  *newMesh = new uint16_t[480*640];
    uint32_t  soma;
    int quant;
    
    for(int i = 0; i <= 480; i+= var)
    for(int j = 0; j <= 640; j+= var){
        
        soma = 0;
        
        if((*mesh)[i*640 + j] == 2047){
            newMesh[i*640 + j] = 2047;
            continue;
        }
        
        for(int k = -2; k <= 2; k++)
        for(int l = -2; l <= 2; l++){
            
            if((i+(k*var)) * 640 + j+(l*var) >= 0 &&
               (i+(k*var)) * 640 + j+(l*var) < 640*480 &&
               (*mesh)[(i+(k*var)) * 640 + j+(l*var)] != 2047){
                
                soma += (*mesh)[(i+(k*var)) * 640 + j+(l*var)] * map[(k+1)*3 +(l+1)];
                quant += map[(k+1)*3 +(l+1)];
            }else{
                soma += (*mesh)[i*640 + j] * map[(k+1)*3 +(l+1)];

            }      
        }
        
        newMesh[i*640 + j] = soma/75;
        
    }
    
    delete *mesh;
    *mesh = newMesh;
}

void MeshControl::smoothLaplacian(Group *group, int var){
    
    float newZ;
	unsigned int pos, current;
	int count;
	
	int I[] = {var, -var, 0, 	0, 	-var, -var, var, var};
	int J[] = {0, 	0, 	var, -var, 	var, -var, var, -var};
	
	for(unsigned int i = 0; i < 480; i+=var)
	for(unsigned int j = 0; j < 640; j+=var){
		
		if(IDMatrix[i*640 + j] == 0)
			continue;
			
		current = IDMatrix[i*640 + j];
		
		newZ = 0;
		
		if(group->vertices[current - 1]->cross)
			count = 8;
		else
			count = 4;
		
		for(int k = 0; k < count; k++){
			pos = IDMatrix[(i + I[k])*640 + j + J[k]];
			if(pos != 0)
				newZ += group->vertices[pos - 1]->point.z - 
							group->vertices[current - 1]->point.z;
 		}
		
		group->vertices[current - 1]->point.nz = 
			group->vertices[current - 1]->point.z + 0.02*newZ;
	}
}
