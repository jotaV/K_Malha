//============================================================================
// Name        : meshSettings.cpp
// Author      : Jv
// Version     : Pré-Beta
// Description : Gerador de Malha usando o dispositivo Kinect
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <math.h>

#include "externalVar.h"
#include "K_malha.h"

using namespace std;

unsigned int *ID_Matrix;


//=========================================================
// Smoothing functions
//=========================================================

void MeshLaplace(int var){
	
	float newZ;
	unsigned int pos, current;
	int count;
	
	int vetI[] = {var, -var, 0, 	0, 	-var, -var, var, var};
	int vetJ[] = {0, 	0, 	var, -var, 	var, -var, var, -var};
	
	for(unsigned int i = 0; i < 480; i+=var){
	for(unsigned int j = 0; j < 640; j+=var){
		
		if(ID_Matrix[i*640 + j] == 0)
			continue;
			
		current = ID_Matrix[i*640 + j];
		
		newZ = 0;
		
		if(!grupo.vertices[current - 1]->cross)
			count = 8;
		else
			count = 4;
		
		for(int l = 0; l < count; l++){
			pos = ID_Matrix[(i + vetI[l])*640 + j + vetJ[l]];
			if(pos != 0)
				newZ += grupo.vertices[pos - 1]->pto.z - 
							grupo.vertices[current - 1]->pto.z;
		}
		
		grupo.vertices[current - 1]->pto.nz = 
			grupo.vertices[current - 1]->pto.z + 0.01*newZ;
	}}
}

//=========================================================
// Extra
//=========================================================

void defineCross(unsigned int *square, bool fistIsCross){
	for(int i = 0; i < 4; i++){
		if(square[i] != 0){
			grupo.vertices[square[i]-1]->cross = fistIsCross;
			fistIsCross = !fistIsCross;
		}
	}
}

void calcularValencia(Face *f){

	Point v1, v2, normal;

	v1.x = grupo.vertices[f->indV[1]-1]->pto.x -
				grupo.vertices[f->indV[0]-1]->pto.x;
	v1.y = grupo.vertices[f->indV[1]-1]->pto.y -
				grupo.vertices[f->indV[0]-1]->pto.y;
	v1.z = grupo.vertices[f->indV[1]-1]->pto.z -
				grupo.vertices[f->indV[0]-1]->pto.z;

	v2.x = grupo.vertices[f->indV[2]-1]->pto.x -
				grupo.vertices[f->indV[0]-1]->pto.x;
	v2.y = grupo.vertices[f->indV[2]-1]->pto.y -
				grupo.vertices[f->indV[0]-1]->pto.y;
	v2.z = grupo.vertices[f->indV[2]-1]->pto.z -
				grupo.vertices[f->indV[0]-1]->pto.z;

	normal.x = ( v1.y * v2.z - v2.y * v1.z);
	normal.y = ( v1.z * v2.x - v2.z * v1.x);
	normal.z = ( v1.x * v2.y - v2.x * v1.y);

	grupo.vertices[f->indV[0]-1]->normal.x += normal.x;
	grupo.vertices[f->indV[0]-1]->normal.y += normal.y;
	grupo.vertices[f->indV[0]-1]->normal.z += normal.z;
	grupo.vertices[f->indV[0]-1]->valencia++;

	grupo.vertices[f->indV[1]-1]->normal.x += normal.x;
	grupo.vertices[f->indV[1]-1]->normal.y += normal.y;
	grupo.vertices[f->indV[1]-1]->normal.z += normal.z;
	grupo.vertices[f->indV[1]-1]->valencia++;

	grupo.vertices[f->indV[2]-1]->normal.x += normal.x;
	grupo.vertices[f->indV[2]-1]->normal.y += normal.y;
	grupo.vertices[f->indV[2]-1]->normal.z += normal.z;
	grupo.vertices[f->indV[2]-1]->valencia++;
}

//=========================================================
// Generating Functions
//=========================================================

void gerarVertices(uint16_t *depth, int var){

	Vertice *v;
	
	unsigned int k = 1;
	for(unsigned int i = 0; i < 480; i+=var)
	for(unsigned int j = 0; j < 640; j+=var){
		
		if(depth[i*640 + j] != 2047){
		
			v = new Vertice;
			
			v->pto.x = j+1; 
			v->pto.y = i+1;
			v->pto.z = depth[i*640 + j] - 330;
			
			v->valencia =
			v->pto.nz 	= 0;
			
			v->ID = k;
			ID_Matrix[i*640 + j] = k;
			
			grupo.vertices.push_back(v);
			k++;
		}
		else ID_Matrix[i*640 + j] = 0;
	}
}

void gerarFace(unsigned int v_1, unsigned int v_2, unsigned int v_3,
				unsigned int ID){
	Face *f;

	f = new Face;
	f->indV[0] = v_1;
	f->indV[1] = v_2;
	f->indV[2] = v_3;
	f->ID = ID;
	grupo.faces.push_back(f);
}

void gerarFaces(int var){

	unsigned int square[4], k = 1;
	bool PAR = false;

	for(unsigned int i = 0; i < 480-var; i+=var){
		for(unsigned int j = 0; j < 640-var; j+=var){
			
			PAR = !PAR;
			square[0] = ID_Matrix[i*640 + j];				//Atual - 0
			square[1] = ID_Matrix[(i+var)*640 + j];			//Inferior - 1
			square[2] = ID_Matrix[(i+var)*640 + (j+var)]; 	//Proximo ao Inferior - 2
			square[3] = ID_Matrix[i*640 + (j+var)]; 		//Proximo - 3


			if(PAR){
				if(square[3] && square[1]){
					if(square[0]){				
						gerarFace(square[0], square[1], square[3], k);
						k++;
					}
					if(square[2]){
						gerarFace(square[1], square[2], square[3], k);
						k++;
					}
					defineCross(square, PAR);
				}

			}else{
				if(square[2] && square[0]){
					if(square[1]){		
						gerarFace(square[0], square[1], square[2], k);
						k++;	
					}					
					if(square[3]){		
						gerarFace(square[0], square[2], square[3], k);
						k++;
					}
					defineCross(square, PAR);
				}
			}
		}
	}
}

void gerarNormais(){
	
	for (unsigned int i = 0 ; i < grupo.faces.size(); i++)
		calcularValencia(grupo.faces[i]);
	
	for (unsigned int i = 0 ; i < grupo.vertices.size(); i++){
		grupo.vertices[i]->normal.x /= grupo.vertices[i]->valencia;
		grupo.vertices[i]->normal.y /= grupo.vertices[i]->valencia;
		grupo.vertices[i]->normal.z /= grupo.vertices[i]->valencia;

		float norma = sqrt(pow(grupo.vertices[i]->normal.x, 2) +
							pow(grupo.vertices[i]->normal.y, 2) +
							pow(grupo.vertices[i]->normal.z, 2));

		grupo.vertices[i]->normal.x /= norma;
		grupo.vertices[i]->normal.y /= norma;
		grupo.vertices[i]->normal.z /= norma;
		
		calcBoundBoxUnit(grupo.vertices[i]);
	}
}

//=========================================================
// generate Mesh
//=========================================================

void generateMesh(int var, bool enableSmoo = true){
	
	ID_Matrix = new unsigned int[640*480];
	
	initVariables();
		
	grupo.vertices.clear();
	grupo.faces.clear();
	
	cout << "\nGerando vertices...\n";
	gerarVertices(depth_mesh, var);

	cout << "Gerando faces... \n";
	gerarFaces(var);
	
	if(enableSmoo){
		cout << "Suavizando...\n";
		for(int i = 0; i < 150; i++){
			MeshLaplace(var);
			for (unsigned int j = 0; j < grupo.vertices.size(); j++)
			grupo.vertices[j]->pto.z = grupo.vertices[j]->pto.nz;
		}
	}
	
	cout << "Gerando normais...\n";
	gerarNormais();
	
	cout << "\nGERACAO FINALIZADA\n\n";
	
	renderStart();
	DrawCamFoced();
}

//=========================================================
// Saiving Function
//=========================================================

void saveOBJ(string arquivo){
	
	Vertice *v;
	Face 	*f;

	string comando;

	arquivo += ".obj";
	ofstream fout(arquivo.c_str());

	cout << "\nGravando vertices...\n";

		for(unsigned int i = 0; i < grupo.vertices.size(); i++){
			v = grupo.vertices[i];
			fout << "v " << v->pto.x << " " << v->pto.y << " " << v->pto.z << endl;
		}
	fout << endl;

	cout << "Gravando normais...\n";

		for(unsigned int i = 0; i < grupo.vertices.size(); i++){
			v = grupo.vertices[i];
			fout << "vn " << v->normal.x << " " << v->normal.y << " " << v->normal.z << endl;
		}
	fout << endl;

	cout << "Gravando Faces...\n";
		for(unsigned int i = 0; i < grupo.faces.size(); i++){
			f = grupo.faces[i];
			fout << "f " << f->indV[0] << "//" << f->indV[0] << " "
						 << f->indV[1] << "//" << f->indV[1] << " "
						 << f->indV[2] << "//" << f->indV[2] << endl;
		}


	comando.append("mv ");
	comando.append(arquivo);
	comando.append(" OBJs/");
	comando.append(arquivo);

	system("mkdir -p OBJs");
	system(comando.c_str());
	cout << "\nFINALIZADO\n";
}
