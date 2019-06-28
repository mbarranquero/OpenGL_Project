#define _USE_MATH_DEFINES

#include "OctreeNode.h"

#include <cmath>
#include <algorithm>

using namespace std;

/*std::shared_ptr<OctreeNode> OctreeNode::buildOctree(unsigned int numOfPerLeafVertices, std::vector<int> verticesIdInside, glm::vec3 minB, glm::vec3 maxB,const std::vector<glm::vec3> & m_vertexPositions, const std::vector<glm::vec3> & m_vertexNormals, std::vector<int> &representativeOfVertex, int &numberOfLeavesYet){
// Implementation recursive du Cours par la fonction de la classe OctreeNode
  std::shared_ptr<OctreeNode> node(new OctreeNode);

  if(verticesIdInside.size() < numOfPerLeafVertices){
    node->verticesIdInside = verticesIdInside;
    node->minB = minB;
    node->maxB = maxB;

    for(int i = 0; i<verticesIdInside.size();i++){
      representativeOfVertex[verticesIdInside[i]]=numberOfLeavesYet;
    }

    numOfPerLeafVertices = numOfPerLeafVertices + 1;
  }
  else{
    std::vector<int>* childVerticesIdInside = new std::vector<int>[8];
    glm::vec3 childMinBox[8];
    glm::vec3 childMaxBox[8];
    float minX = minB[0];
    float minY = minB[1];
    float minZ = minB[2];
    float maxX = maxB[0];
    float maxY = maxB[1];
    float maxZ = maxB[2];
    childMinBox[0] = glm::vec3 (minX,minY,minZ);
    childMaxBox[0] = glm::vec3 (0.5*(minX+maxX),0.5*(minY+maxY),0.5*(minZ+maxZ));
    childMinBox[1] = glm::vec3 (0.5*(minX+maxX),minY,minZ);
    childMaxBox[1] = glm::vec3 (maxX,0.5*(minY+maxY),0.5*(minZ+maxZ));
    childMinBox[2] = glm::vec3 (minX,0.5*(minY+maxY),minZ);
    childMaxBox[2] = glm::vec3 (0.5*(minX+maxX),maxY,0.5*(minZ+maxZ));
    childMinBox[3] = glm::vec3 (minX,minY,0.5*(minZ+maxZ));
    childMaxBox[3] = glm::vec3 (0.5*(minX+maxX),0.5*(minY+maxY),maxZ);
    childMinBox[4] = glm::vec3 (0.5*(minX+maxX),0.5*(minY+maxY),minZ);
    childMaxBox[4] = glm::vec3 (maxX,maxY,0.5*(minZ+maxZ));
    childMinBox[5] = glm::vec3 (0.5*(minX+maxX),minY,0.5*(minZ+maxZ));
    childMaxBox[5] = glm::vec3 (maxX,0.5*(minY+maxY),maxZ);
    childMinBox[6] = glm::vec3 (minX,0.5*(minY+maxY),0.5*(minZ+maxZ));
    childMaxBox[6] = glm::vec3 (0.5*(minX+maxX),maxY,maxZ);
    childMinBox[7] = glm::vec3 (0.5*(minX+maxX),0.5*(minY+maxY),0.5*(minZ+maxZ));
    childMaxBox[7] = glm::vec3 (maxX,maxY,maxZ);
    int indiceOfCell[2][2][2];
    indiceOfCell[0][0][0] = 0;
    indiceOfCell[1][0][0] = 1;
    indiceOfCell[0][1][0] = 2;
    indiceOfCell[0][0][1] = 3;
    indiceOfCell[1][1][0] = 4;
    indiceOfCell[1][0][1] = 5;
    indiceOfCell[0][1][1] = 6;
    indiceOfCell[1][1][1] = 7;
    for(int i = 0; i<verticesIdInside.size();i++){
      int cellX = (int)(2*(m_vertexPositions[verticesIdInside[i]][0] - minX)/(maxX - minX));
      int cellY = (int)(2*(m_vertexPositions[verticesIdInside[i]][1] - minY)/(maxY - minY));
      int cellZ = (int)(2*(m_vertexPositions[verticesIdInside[i]][2] - minZ)/(maxZ - minZ));
      childVerticesIdInside[indiceOfCell[cellX][cellY][cellZ]].push_back(verticesIdInside[i]);
    }
    for(int i = 0; i<8; i++){
        node->children[i] = buildOctree(numOfPerLeafVertices,childVerticesIdInside[i],childMinBox[i],childMaxBox[i], m_vertexPositions, m_vertexNormals, representativeOfVertex,numberOfLeavesYet);
    }
  }
  return node;
}*/

std::shared_ptr<OctreeNode> OctreeNode::buildOctree(float x10,float x20,float y10,float y20,float z10,float z20,std::vector<int> verticesIdTab0,std::vector<glm::vec3> * vPosPtr0,int numOfPerLeafVertices){
// Implementation recursive du Cours par la fonction de la classe OctreeNode
    std::shared_ptr<OctreeNode> node(new OctreeNode);

		node->x1=x10;
		node->x2,node->y1,node->y2,node->z1,node->z2=x20,y10,y20,z10,z20;
		node->vPosPtr=vPosPtr0;

		if (verticesIdTab0.size()<= numOfPerLeafVertices){ //it's time to STOP
			node->verticesIdTab=verticesIdTab0;
			node->isLeaf=true;
			leafId = leafNumber;

      cout <<"Creating leaf "<< leafId<<endl;
			leafNumber +=1;

			treeVector.push_back(node->verticesIdTab);
			if (treeVector.size() != leafNumber){
        cout << "Error Mesh.cpp : stored leaf in the wrong slot";
      }
			for (int i=0;i<node->verticesIdTab.size();i++){
        vertexLeafId[node->verticesIdTab[i]]=leafNumber-1;
      }

			for (int i=0;i<8;i++){
				node->children[i] = nullptr;
			}
			cout << "Building leaf in " << x10 <<","<<x20<<"," <<y10<<"," <<y20<<","<<z10<<","<<z20 << endl;
		}


	else { //still dividing
		// we'll split the vertices in the 8 regions :
		// lowest bit for x, highest for z
		// 000 is below all thresholds, 111 above all
		node->isLeaf=false;
		std::vector< std::vector<int> > regions;
		float midx = (x10+x20)/2;
		float midy = (y10+y20)/2;
		float midz = (z10+z20)/2;

		for (int i=0;i<8;i++){
			std::vector<int> reg;
			regions.push_back(reg);
		}


		for (int id : verticesIdTab0){
			int nb = 0;
			if ((*vPosPtr0)[id].x>midx) nb += 1;
			if ((*vPosPtr0)[id].y>midy) nb += 2;
			if ((*vPosPtr0)[id].z>midz) nb += 4;
			regions[nb].push_back(id);
		}


		//*
		node->children[0] = buildOctree(x10,midx,y10,midy,z10,midz,regions[0],vPosPtr0,numOfPerLeafVertices);
		node->children[1] = buildOctree(midx,x20,y10,midy,z10,midz,regions[1],vPosPtr0,numOfPerLeafVertices);
		node->children[2] = buildOctree(x10,midx,midy,y20,z10,midz,regions[2],vPosPtr0,numOfPerLeafVertices);
		node->children[3] = buildOctree(midx,x20,midy,y20,z10,midz,regions[3],vPosPtr0,numOfPerLeafVertices);
		node->children[4] = buildOctree(x10,midx,y10,midy,midz,z20,regions[4],vPosPtr0,numOfPerLeafVertices);
		node->children[5] = buildOctree(midx,x20,y10,midy,midz,z20,regions[5],vPosPtr0,numOfPerLeafVertices);
		node->children[6] = buildOctree(x10,midx,midy,y20,midz,z20,regions[6],vPosPtr0,numOfPerLeafVertices);
		node->children[7] = buildOctree(midx,x20,midy,y20,midz,z20,regions[7],vPosPtr0,numOfPerLeafVertices);
		//*/

	}
	return node;
}

void OctreeNode::simplifyOctree(std::shared_ptr<OctreeNode> tree, std::vector<glm::vec3> * vPosPtr,std::vector<glm::uvec3>* triPtr){
				if (tree==nullptr) cout <<"oups (Mesh cpp error no 1415)" <<endl;
				else{

					std::vector<glm::vec3> resultV;
					std::vector<glm::uvec3> resultTri;

					//comment outputter la moyenne ?
					//comment recalculer les triangles ?

					//Préalablement, quand on construit l'octree,
					//donner un id aux feuilles, et à chaque fois qu'on ajoute un sommet à une feuille,
					//on ajoute le vertexId à un vector treeVector, et compléter vertexLeafId

					//Calcul représentants:
					//Dans treeVector, itérer sur chaque sous vector
					//   faire la moyenne des éléments et la placer dans la position correspondante au vector

					for (int i=0;i<treeVector.size();i++){
						glm::vec3 moyenne = glm::vec3(0.f);
						for (int j=0;j<treeVector[i].size();j++){
							moyenne = moyenne + (*vPosPtr)[treeVector[i][j]];
						}
						moyenne = moyenne / (float) treeVector[i].size();
						resultV.push_back(moyenne);
					}

					//Calcul triangles :
					//Parcourir les triangles :
					//   Si les trois vertices sont sur des feuilles différentes, ajouter le triangle sur les feuilles

					//resultTri.push_back(glm::uvec3(1,2,3)); //TODO delete this !!

					for (int i=0;i<(*triPtr).size();i++){ //REMARQUE attention est ce que le * est juste ?
						int v0=(*triPtr)[i][0];
						int v1=(*triPtr)[i][1];
						int v2=(*triPtr)[i][2];
						if (vertexLeafId[v0] != vertexLeafId[v1] && vertexLeafId[v0] != vertexLeafId[v2] && vertexLeafId[v1] != vertexLeafId[v2]){
							cout << "found a triangle"<<endl;
							//*/
							glm::uvec3 newT = glm::uvec3(vertexLeafId[v0],vertexLeafId[v1],vertexLeafId[v2]);
							resultTri.push_back(newT);
							glm::uvec3 newT2 = glm::uvec3(vertexLeafId[v1],vertexLeafId[v0],vertexLeafId[v2]); //je ne peux pas savoir dans quelle sens le triangle devrait être
							resultTri.push_back(newT2);
							//*/
						}
					}

					//*/
					*vPosPtr=resultV;
					*triPtr=resultTri;
					//*/
				}
			}
