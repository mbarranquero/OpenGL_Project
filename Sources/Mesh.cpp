#define _USE_MATH_DEFINES

#include "Mesh.h"

#include <cmath>
#include <algorithm>

//#define M_PI 3.1415926535897932384626433832795

std::vector<int> vertexLeafId;
std::vector<std::vector<int>> treeVector;
int leafNumber;

using namespace std;

Mesh::~Mesh () {
	clear ();
}

void Mesh::computeBoundingSphere (glm::vec3 & center, float & radius) const {
	center = glm::vec3 (0.0);
	radius = 0.f;
	for (const auto & p : m_vertexPositions)
		center += p;
	center /= m_vertexPositions.size ();
	for (const auto & p : m_vertexPositions)
		radius = std::max (radius, distance (center, p));
}

// function for finding the angle
float Mesh::angle(glm::vec3 a, glm::vec3 b, glm::vec3 c){
	glm::vec3 u = a-b;
	glm::vec3 v = c-b;
	return (acos(dot(u,v)/sqrt(dot(u,u)*dot(v,v))));
}


float Mesh::cotangent(float angle){
	return cos(angle)/sin(angle);
}
// Autre maniere de calculer
float Mesh::cotangent(glm::vec3 a, glm::vec3 b, glm::vec3 c){
	return  dot(a-b , c-b) / cross(a-b , c-b).length() ;
}


void Mesh::computeCotangentWeights(){
	cotW.clear();
	cotW.resize( m_vertexPositions.size() );
	for (const glm::uvec3& i : m_triangleIndices) { //pour chaque triangle
		double angleAlpha0 = angle(m_vertexPositions[i[1]],m_vertexPositions[i[0]],m_vertexPositions[i[2]]);
		double angleAlpha1 = angle(m_vertexPositions[i[0]],m_vertexPositions[i[1]],m_vertexPositions[i[2]]);
		double angleAlpha2 = angle(m_vertexPositions[i[1]],m_vertexPositions[i[2]],m_vertexPositions[i[0]]);

	/*	cotW[i[0]][i[1]] += cotangent(angleAlpha2)/2;
		cotW[i[1]][i[0]] += cotangent(angleAlpha2)/2;

		cotW[i[0]][i[2]] += cotangent(angleAlpha1)/2;
		cotW[i[2]][i[0]] += cotangent(angleAlpha1)/2;

		cotW[i[1]][i[2]] += cotangent(angleAlpha0)/2;
		cotW[i[2]][i[1]] += cotangent(angleAlpha0)/2;*/

// Méthode au dessus marchait mais les calculs sont décris de façon explicite en dessous :
		cotW[i[0]][i[1]] += cotangent(m_vertexPositions[i[1]],m_vertexPositions[i[2]],m_vertexPositions[i[0]])/2;
		cotW[i[1]][i[0]] += cotangent(m_vertexPositions[i[1]],m_vertexPositions[i[2]],m_vertexPositions[i[0]])/2;

		cotW[i[0]][i[2]] += cotangent(m_vertexPositions[i[0]],m_vertexPositions[i[1]],m_vertexPositions[i[2]])/2;
		cotW[i[2]][i[0]] += cotangent(m_vertexPositions[i[0]],m_vertexPositions[i[1]],m_vertexPositions[i[2]])/2;

		cotW[i[1]][i[2]] += cotangent(m_vertexPositions[i[1]],m_vertexPositions[i[0]],m_vertexPositions[i[2]])/2;
		cotW[i[2]][i[1]] += cotangent(m_vertexPositions[i[1]],m_vertexPositions[i[0]],m_vertexPositions[i[2]])/2;
	}
}

void Mesh::laplacianFilter(float alpha,  bool  cotangentWeights) {
//1ere méthode : trop longue dans les boucles pour avoir un résultat viable
/*std::vector<glm::vec3> newVertex ;
	newVertex.reserve(m_vertexPositions.size());
	glm::vec3 barycentre = glm::vec3(0,0,0);
	float compteur = 0;
	glm::vec3 somme = glm::vec3(0,0,0);
	for(const glm::vec3& sommet : m_vertexPositions){
		for (const glm::uvec3& triPtr : m_triangleIndices) {//pour chaque triangle
		compteur += 2;
			if(sommet == m_vertexPositions[triPtr[0]]){
				somme += m_vertexPositions[triPtr[1]] + m_vertexPositions[triPtr[2]] ;
			}
			else if(sommet == m_vertexPositions[triPtr[1]]){
				somme += m_vertexPositions[triPtr[0]] + m_vertexPositions[triPtr[2]] ;
			}
			else if(sommet == m_vertexPositions[triPtr[2]]){
				somme += m_vertexPositions[triPtr[1]] + m_vertexPositions[triPtr[0]] ;
			}
		}
		barycentre = somme/compteur + sommet;
		newVertex.push_back(barycentre);
		somme = glm::vec3(0,0,0);
		compteur = 0;
	}
	m_vertexPositions = newVertex;	*/

//2eme méthode : directement comme dans recomputePerVertexNormals
	std::vector<glm::vec3> somme;
	somme.resize (m_vertexPositions.size(), glm::vec3(0.0, 0.0, 0.0));

	std::vector<int> compteur;
	compteur.resize (m_vertexPositions.size(), 0);

	if(cotangentWeights==false){
		for (const glm::uvec3& i : m_triangleIndices) { //pour chaque triangle
			somme[i[0]] = somme[i[0]] + m_vertexPositions[i[1]] + m_vertexPositions[i[2]];
			somme[i[1]] = somme[i[1]] + m_vertexPositions[i[0]] + m_vertexPositions[i[2]];
			somme[i[2]] = somme[i[2]] + m_vertexPositions[i[1]] + m_vertexPositions[i[0]];
			compteur[i[0]] = compteur[i[0]] + 2;
			compteur[i[1]] = compteur[i[1]] + 2;
			compteur[i[2]] = compteur[i[2]] + 2;
		}
		for (int i = 0; i < m_vertexPositions.size(); i++){
			somme[i] = somme[i] * (1.f / compteur[i]);
		}
	}

// Laplacien géométrique
	if(cotangentWeights==true){
		for (int i = 0; i < m_vertexPositions.size(); i++){
			float sommeW = 0;
			for(map<int, float>::iterator it = cotW[i].begin(); it != cotW[i].end(); ++it) {
				int j = it->first;
				float w_ij = it->second;
				sommeW += w_ij;
				somme[i] += w_ij * m_vertexPositions[j];
			}
			somme[i] = somme[i] / sommeW;
		}
	}

	for (int i = 0; i < m_vertexPositions.size(); i++){
		m_vertexPositions[i] = alpha*somme[i] + (1 - alpha)*m_vertexPositions[i];
	}

	recomputePerVertexNormals(true);
	clearGPU();
	init();
}

std::pair<glm::vec3,glm::vec3> Mesh::computeBoundingBox(){
	const float inf = std::numeric_limits<float>::infinity();
	glm::vec3 max = glm::vec3(-inf, -inf, -inf);
	glm::vec3 min = glm::vec3(inf, inf, inf);

	for (int i = 0; i < m_vertexPositions.size(); i++){
		glm::vec3 u = m_vertexPositions[i];
		for (int j = 0; j < 3; j++){
			min[j] = std::min(min[j], u[j]);
			max[j] = std::max(max[j], u[j]);
		}
	}
	return {min, max};
}

void Mesh::simplify (unsigned int resolution){
	//Boite englobante
	std::pair<glm::vec3,glm::vec3> bornes = computeBoundingBox();

	glm::vec3 minimum = bornes.first - glm::vec3(0.1,0.1,0.1);
	glm::vec3 maximum = bornes.second + glm::vec3(0.1,0.1,0.1);

	glm::vec3 dist = (maximum - minimum) * (1.f / resolution);

	if (dist[0]==0 ||dist[1]==0 ||dist[2]==0){
		std::cout << "ERROR there's a length equal to 0 in Mesh.cpp" <<std::endl;
	}

	final_vertexPositions.clear();
	final_vertexPositions.resize(resolution*resolution*resolution);

	std::vector<int> numberInBloc; //nombre de sommets par blocs
	numberInBloc.resize(resolution*resolution*resolution);

	std::vector<glm::vec3> normList;
	normList.resize(resolution*resolution*resolution);

	for (int i = 0; i < m_vertexPositions.size(); i++){
		glm::vec3 bl =  (m_vertexPositions[i] - minimum); //bl = bloc
		bl.x = floor(bl.x / dist.x);
		bl.y = floor(bl.y / dist.y);
		bl.z = floor(bl.z / dist.z);

		int blId = bl.x + bl.y*resolution + bl.z*resolution*resolution; //blId = identifiant du bloc
		numberInBloc[blId] += 1;

		final_vertexPositions[blId] += m_vertexPositions[i];
		normList[blId] += m_vertexNormals[i];
	}

	for (int i = 0;i < resolution*resolution*resolution; i++){
		if (numberInBloc[i]!=0){
			final_vertexPositions[i] = final_vertexPositions[i]/( (float) numberInBloc[i]);
			normList[i] = glm::normalize(normList[i]);
		}
	}

	for(const glm::uvec3& t : m_triangleIndices){
		glm::vec3 u0 = m_vertexPositions[t[0]] - minimum;
		glm::vec3 u1 = m_vertexPositions[t[1]] - minimum;
		glm::vec3 u2 = m_vertexPositions[t[2]] - minimum;

		u0.x = floor(u0.x / dist.x);
		u0.y = floor(u0.y / dist.y);
		u0.z = floor(u0.z / dist.z);

		u1.x = floor(u1.x / dist.x);
		u1.y = floor(u1.y / dist.y);
		u1.z = floor(u1.z / dist.z);

		u2.x = floor(u2.x / dist.x);
		u2.y = floor(u2.y / dist.y);
		u2.z = floor(u2.z / dist.z);

		//on recalcule les identifiants pour voir s'ils sont dans le même bloc
		int id0 = u0.x + u0.y*resolution + u0.z*resolution*resolution;
		int id1 = u1.x + u1.y*resolution + u1.z*resolution*resolution;
		int id2 = u2.x + u2.y*resolution + u2.z*resolution*resolution;

		//S'ils sont chacun dans un bloc différent, on les associera au barycentre de leur bloc respectif
		if(id0 != id1 && id0 != id2 && id1 != id2){
			final_vertexIndices.push_back(glm::vec3(id0,id1,id2));
		}
		//sinon on ne les ajoute pas a la nouvelle liste de sommets
	}
	m_vertexPositions.resize(resolution*resolution*resolution);
	m_vertexNormals.resize(resolution*resolution*resolution);

	m_triangleIndices = final_vertexIndices;
	m_vertexPositions = final_vertexPositions;
	m_vertexNormals = normList;

	clearGPU();
	recomputePerVertexNormals(true);
	init();
}


void Mesh::adaptiveSimplify (unsigned int numOfPerLeafVertices){
//Calcul de la Boite englobante
	std::pair<glm::vec3,glm::vec3> bornes = computeBoundingBox();

	glm::vec3 minimum = bornes.first - glm::vec3(0.1,0.1,0.1);
	glm::vec3 maximum = bornes.second + glm::vec3(0.1,0.1,0.1);

// Création d'une classe type OctreeNode
	struct OctreeNode{
		std::shared_ptr<OctreeNode> children[8];
		std::vector<int> verticesIdInside;
		glm::vec3 minBox;
		glm::vec3 maxBox;

// Création de la fonction récursive telle que vue en cours
		std::shared_ptr<OctreeNode> buildOctree(unsigned int numberOfPerLeafVertices, std::vector<int> verticesIdInside, glm::vec3 minBox, glm::vec3 maxBox,const std::vector<glm::vec3> & m_vertexPositions, const std::vector<glm::vec3> & m_vertexNormals, std::vector<int> &representativeOfVertex, int &numberOfLeavesYet){

			std::shared_ptr<OctreeNode> node(new OctreeNode);

			if(verticesIdInside.size() < numberOfPerLeafVertices){ // il faut s'arreter
				node->verticesIdInside = verticesIdInside;
				node->minBox = minBox;
				node->maxBox = maxBox;
				for(int i = 0; i < verticesIdInside.size();i++){
					representativeOfVertex[verticesIdInside[i]] = numberOfLeavesYet;
				}
				numberOfLeavesYet = numberOfLeavesYet + 1;
			}
			else{ // il faut continuer

				std::vector<int>* childVerticesIdInside = new std::vector<int>[8];
				glm::vec3 childMinBox[8];
				glm::vec3 childMaxBox[8];
				float minX = minBox[0];
				float minY = minBox[1];
				float minZ = minBox[2];
				float maxX = maxBox[0];
				float maxY = maxBox[1];
				float maxZ = maxBox[2];
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
					int cellX = (int)(2*(m_vertexPositions[verticesIdInside[i]][0]-minX)/(maxX-minX));
					int cellY = (int)(2*(m_vertexPositions[verticesIdInside[i]][1]-minY)/(maxY-minY));
					int cellZ = (int)(2*(m_vertexPositions[verticesIdInside[i]][2]-minZ)/(maxZ-minZ));
					childVerticesIdInside[indiceOfCell[cellX][cellY][cellZ]].push_back(verticesIdInside[i]);
				}
				for(int i = 0; i < 8; i++){
						node->children[i] = buildOctree(numberOfPerLeafVertices,childVerticesIdInside[i],childMinBox[i],childMaxBox[i], m_vertexPositions, m_vertexNormals, representativeOfVertex,numberOfLeavesYet);
				}
			}
			return node;
		}
	};

	OctreeNode octree;
	std::vector<int> allVertexIndices;
	for(int i = 0; i < m_vertexPositions.size(); i++){
		allVertexIndices.push_back(i);
	}

	std::vector<glm::vec3> final_vertexPositions;
	final_vertexPositions.resize(m_vertexPositions.size(), nul);

	std::vector<glm::vec3> simplifiedNormals;
	simplifiedNormals.resize(m_vertexPositions.size(),nul);

	std::vector<int> representativeOfVertex;
	representativeOfVertex.resize(m_vertexPositions.size(), 0);

	int numberOfLeavesYet = 0;

	std::shared_ptr<OctreeNode> octreePtr = octree.buildOctree(numOfPerLeafVertices, allVertexIndices, minimum, maximum, m_vertexPositions, m_vertexNormals,representativeOfVertex,numberOfLeavesYet);
	std::vector<int> numberOfVerticesInLeaf;
	numberOfVerticesInLeaf.resize(m_vertexPositions.size(), 0);

	for(int i = 0; i < m_vertexPositions.size(); i++){
		final_vertexPositions[representativeOfVertex[i]] = final_vertexPositions[representativeOfVertex[i]]+m_vertexPositions[i];
		simplifiedNormals[representativeOfVertex[i]] = simplifiedNormals[representativeOfVertex[i]]+m_vertexNormals[i];
		numberOfVerticesInLeaf[representativeOfVertex[i]] = numberOfVerticesInLeaf[representativeOfVertex[i]]+1;
	}

	for(int i = 0; i < m_vertexPositions.size(); i++){
		if(numberOfVerticesInLeaf[i] != 0){
			final_vertexPositions[i] = final_vertexPositions[i]/(float)numberOfVerticesInLeaf[i];
			simplifiedNormals[i] = glm::normalize(simplifiedNormals[i]);
		}
	}

	for(int i = 0; i < m_triangleIndices.size(); i++){
		int i0 = representativeOfVertex[m_triangleIndices[i][0]];
		int i1 = representativeOfVertex[m_triangleIndices[i][1]];
		int i2 = representativeOfVertex[m_triangleIndices[i][2]];

		if(i0 != i1 && i0 != i2 && i1 != i2){
			final_vertexIndices.push_back(glm::uvec3(i0,i1,i2));
		}
	}
	m_triangleIndices = final_vertexIndices;
	m_vertexPositions = final_vertexPositions;
	m_vertexNormals = simplifiedNormals;

	clearGPU();
	init();
}


// Fonction pour retourner le odd vertex en fonction du voisinage :
 		// il y a quelques parties inutiles dans cette fonction, mais présentes du fait de la premiere version de mon code
std::pair<glm::vec3,std::pair<int, int>> Mesh::voisinage(const glm::uvec3& t, const glm::uvec3& s){
	glm::vec3 odd = glm::vec3(0,0,0);
	std::pair<int,int> boundary = {0, 0};

	int MX ; // max
	int mn ; // min

	if (t != s){
// On récupère les 3 sommets de chacun des 2 triangles, forcément différents sinon inutile
		glm::vec3 u0 = m_vertexPositions[t[0]] ;
		glm::vec3 u1 = m_vertexPositions[t[1]] ;
		glm::vec3 u2 = m_vertexPositions[t[2]] ;

		glm::vec3 v0 = m_vertexPositions[s[0]] ;
		glm::vec3 v1 = m_vertexPositions[s[1]] ;
		glm::vec3 v2 = m_vertexPositions[s[2]] ;

// Calcul des côtés
		glm::vec3 c1 = abs(u0 - u1);
		glm::vec3 c2 = abs(u0 - u2);
		glm::vec3 c3 = abs(u2 - u1);

		glm::vec3 d1 = abs(v0 - v1);
		glm::vec3 d2 = abs(v0 - v2);
		glm::vec3 d3 = abs(v2 - v1);

// Vérification d'un côté en commun et si oui, ajout d'un odd vertex sur ce dernier
		if(c1 == d1){
			odd = 3.f / 8.f * (u0 + u1) + 1.f / 8.f * (u2 + v2);
			mn = std::min(t[0] , t[1]);
			MX = std::max(t[0] , t[1]);
			boundary = {mn , MX};
		}
		else if(c1 == d2){
			odd = 3.f / 8.f * (u0 + u1) + 1.f / 8.f*(u2 + v1);
			mn = std::min(t[0] , t[1]);
			MX = std::max(t[0] , t[1]);
			boundary = {mn , MX};
		}
		else if(c1 == d3){
			odd = 3.f / 8.f*(u0 + u1) + 1.f / 8.f*(u2 + v0);
			mn = std::min(t[0] , t[1]);
			MX = std::max(t[0] , t[1]);
			boundary = {mn , MX};
		}
// 2eme cas : 2eme arrete du triangle en commun
		else if(c2 == d1){
			odd = 3.f / 8.f*(u0 + u2) + 1.f / 8.f*(u1 + v2);
			mn = std::min(t[0] , t[2]);
			MX = std::max(t[0] , t[2]);
			boundary = {mn , MX};
		}
		else if(c2 == d2){
			odd = 3.f / 8.f*(u0 + u2) + 1.f / 8.f*(u1 + v1);
			mn = std::min(t[0] , t[2]);
			MX = std::max(t[0] , t[2]);
			boundary = {mn , MX};
		}
		else if(c2 == d3){
			odd = 3.f / 8.f*(u0 + u2) + 1.f / 8.f*(u1 + v0);
			mn = std::min(t[0] , t[2]);
			MX = std::max(t[0] , t[2]);
			boundary = {mn , MX};
		}
//  3eme cas : 3eme arrete du triangle en commun
		else if(c3 == d1){
			odd = 3.f / 8.f*(u2 + u1) + 1.f / 8.f*(u0 + v2);
			mn = std::min(t[1] , t[2]);
			MX = std::max(t[1] , t[2]);
			boundary = {mn , MX};
		}
		else if(c3 == d2){
			odd = 3.f / 8.f*(u2 + u1) + 1.f / 8.f*(u0 + v1);
			mn = std::min(t[1] , t[2]);
			MX = std::max(t[1] , t[2]);
			boundary = {mn , MX};
		}
		else if(c3 == d3){
			odd = 3.f / 8.f*(u2 + u1) + 1.f / 8.f*(u0 + v0);
			mn = std::min(t[1] , t[2]);
			MX = std::max(t[1] , t[2]);
			boundary = {mn , MX};
		}
	}
	//std::cout<<mn<<" "<<MX<<" "<<odd.x<<" "<<odd.y<<" "<<odd.z<<std::endl;
	return {odd, boundary} ;
}


std::pair<glm::vec3,std::pair<int, int>> Mesh::computeOdd(int k, int l, std::vector<int> liste){
	std::pair<glm::vec3,std::pair<int, int>> essai;

	for(int i= 0; i < liste.size(); i++){
			if(i!=liste.size()){
				int j = liste[i];
				int jBis = liste[i + 1];
				glm::uvec3 t = glm::uvec3(k, l, j);
				glm::uvec3 s = glm::uvec3(k, l, jBis);
				essai = voisinage(t, s);
		}
		if(i = liste.size()){
			int j = liste[i];
			int jBis = liste[i];
			glm::uvec3 t = glm::uvec3(k, l, j);
			glm::uvec3 s = glm::uvec3(k, l, jBis);
			std::pair<glm::vec3,std::pair<int, int>> essai1 = voisinage(t, s);
		}
	}
	return essai;
}



void Mesh::subdivide(){ // Subdivision de Loop
	std::pair<glm::vec3,std::pair<int, int>> oddV;

	subdV.clear();
	subdV.resize(m_vertexPositions.size()*6);

	std::vector<glm::vec3> M;
	M.resize(m_vertexPositions.size()*3);
	L.clear();
	L.resize(m_vertexPositions.size()*3);

	final_vertexPositions.clear();
	final_vertexPositions.resize(m_vertexPositions.size()*3,nul);

	final_vertexIndices.clear();

	std::vector<glm::vec3> positionList;
	positionList.resize(m_vertexPositions.size()*3, nul);

	std::vector<glm::vec3> normList;
	normList.resize(m_vertexPositions.size()*3);

	glm::vec3 oddvertex = glm::vec3(0,0,0);

// On enregistre les odd vertices et les nouvelles frontieres dans des 2 listes
// Méthode trop longue car on met trop de temps à comparer les différents triangles
	/*for(const glm::uvec3& t : m_triangleIndices){
		for(const glm::uvec3& s : m_triangleIndices){
			oddV = voisinage(t, s);
			glm::vec3 odd = oddV.first;
			std::pair<int, int> boundary = oddV.second;
			int index1 = boundary.first;
			int index2 = boundary.second;
// On remplit un vector de map avec un identifiant qui correspond à l'axe et une valeur qui est l'odd vertex
			if(odd != glm::vec3(0,0,0)){
				subdV[index1][index2] = odd;
				subdV[index2][index1] = odd;
			}
		}
	}*/

// Utilisation de la map tableTriangle pour règler ce probleme de temps de calcul

// tableTriangle contient en identifiant 2 sommets reliés par une arrete et en résultat une liste de 3emes sommets opposés
	std::map<pair<int,int>, std::vector<int>> tableTriangle;

// on met toujours dans le même ordre les indices dans la table pour que l'arrête soit toujours identifiée de la même maniere
	for(const glm::uvec3& t : m_triangleIndices){
		int mn0 = std::min(t[0] , t[1]);
		int MX0 = std::max(t[0] , t[1]);

		std::vector<int> liste0 = tableTriangle[{mn0, MX0}];
		liste0.push_back(t[2]);
		tableTriangle[{mn0, MX0}] = liste0;

		int mn1 = std::min(t[1] , t[2]);
		int MX1 = std::max(t[1] , t[2]);

		std::vector<int> liste1 = tableTriangle[{mn1, MX1}];
		liste1.push_back(t[0]);
		tableTriangle[{mn1, MX1}] = liste1;

		int mn2 = std::min(t[0] , t[2]);
		int MX2 = std::max(t[0] , t[2]);

		std::vector<int> liste2 = tableTriangle[{mn2, MX2}];
		liste2.push_back(t[1]);
		tableTriangle[{mn2, MX2}] = liste2;
		//std::cout<<mn0<<" "<< mn1<<" "<<mn2<<" "<<MX0<<" "<<MX1<<" "<<MX2<<std::endl;
	}
// Calcul des OddVertex grâce à la tableTriangle
	for(auto const& it: tableTriangle){
		pair<int,int> paire = it.first;

		int k = paire.first;
		int l = paire.second;

		std::vector<int> liste = it.second;
		oddV = computeOdd(k, l, liste);
		glm::vec3 odd = oddV.first;
		std::pair<int, int> boundary = oddV.second;

		int index1 = boundary.first;
		int index2 = boundary.second;
		//std::cout<<index1<<" "<< index2<<" "<<odd.x<<" "<<odd.y<<" "<<odd.z<<std::endl;

// On remplit un vector de map avec un identifiant qui correspond à l'axe et une valeur qui est l'odd vertex
		if(odd != glm::vec3(0,0,0)){
			subdV[index1][index2] = odd;
			subdV[index2][index1] = odd;
		}
	}

	int compteur = 0;
	for (int i = 0; i < m_vertexPositions.size(); i++){
		for(map<int, glm::vec3>::iterator it = subdV[i].begin(); it != subdV[i].end(); ++it) {
			glm::vec3 odd = it->second;
			oddvertex += odd;
			compteur += 1;
			//std::cout<<i<<" "<<odd.x<<" "<<odd.y<<" "<<odd.z<<" "<<compteur<<std::endl;
		}
		float alpha_compteur = 1.f / 64 * (40 - (3 + 2 * cos(2 * M_PI / compteur ))*(3 + 2 * cos(2 * M_PI / compteur)));
		positionList[i] = m_vertexPositions[i]*(1 - alpha_compteur) + alpha_compteur * oddvertex * (1.f / compteur);
	}

// On bouge les even vertices par rapport aux odes vertices
	for(const glm::uvec3& t : m_triangleIndices){
		final_vertexPositions[t[0]] = positionList[t[0]];
		final_vertexPositions[t[1]] = subdV[t[0]][t[1]];
		final_vertexPositions[t[2]] = subdV[t[0]][t[2]];

		final_vertexIndices.push_back(glm::vec3(t[0], t[1], t[2]));

		final_vertexPositions[t[1] + 1] = positionList[t[1]];
		final_vertexPositions[t[2] + 1] = subdV[t[1]][t[2]];
		final_vertexPositions[t[1]] = subdV[t[1]][t[0]];

		final_vertexIndices.push_back(glm::vec3(t[1] + 1,t[2] + 1, t[1]));

		final_vertexPositions[t[2] + 2] = positionList[t[2]];
		final_vertexPositions[t[2]] = subdV[t[2]][t[0]];
		final_vertexPositions[t[2] + 1] = subdV[t[2]][t[1]];

		final_vertexIndices.push_back(glm::vec3(t[2] + 2,t[2], t[2] + 1));
		final_vertexIndices.push_back(glm::vec3(t[1], t[2] + 1, t[2]));
	}
	m_triangleIndices.clear();
	m_vertexPositions.clear();

	m_triangleIndices = final_vertexIndices;
	m_vertexPositions = final_vertexPositions;

	clearGPU();
	recomputePerVertexNormals(false);
	init();
}


void Mesh::recomputePerVertexNormals (bool angleBased) {
	m_vertexNormals.clear ();
	m_vertexNormals.resize (m_vertexPositions.size (), glm::vec3 (0.0, 0.0, 0.0));
	glm::vec3 normalVect;

	if(angleBased) {
		for (const glm::uvec3& triPtr : m_triangleIndices) {//pour chaque triangle
			normalVect = glm::normalize(glm::cross(m_vertexPositions[triPtr[1]]-m_vertexPositions[triPtr[0]],(m_vertexPositions[triPtr[2]]-m_vertexPositions[triPtr[0]])));
			m_vertexNormals[triPtr[0]]+=angle(m_vertexPositions[triPtr[1]],m_vertexPositions[triPtr[0]],m_vertexPositions[triPtr[2]])*normalVect;
			m_vertexNormals[triPtr[1]]+=angle(m_vertexPositions[triPtr[0]],m_vertexPositions[triPtr[1]],m_vertexPositions[triPtr[2]])*normalVect;
			m_vertexNormals[triPtr[2]]+=angle(m_vertexPositions[triPtr[0]],m_vertexPositions[triPtr[2]],m_vertexPositions[triPtr[1]])*normalVect;
		}
	}

	else{
		for (const glm::uvec3& triPtr : m_triangleIndices) {//pour chaque triangle
			normalVect = glm::normalize(glm::cross(m_vertexPositions[triPtr[1]]-m_vertexPositions[triPtr[0]],(m_vertexPositions[triPtr[2]]-m_vertexPositions[triPtr[0]])));
			m_vertexNormals[triPtr[0]] += normalVect;
			m_vertexNormals[triPtr[1]] += normalVect;
			m_vertexNormals[triPtr[2]] += normalVect;
		}
	}
	for (glm::vec3 n : m_vertexNormals) {
		n = glm::normalize(n);
	}
}


void Mesh::computePlanarParameterization () {
	float mx = m_vertexPositions[0][0];
	float my = m_vertexPositions[0][1];
	float Mx = m_vertexPositions[0][0];
	float My = m_vertexPositions[0][1];
	for (glm::vec3 n : m_vertexPositions) {
		float x = n[0];
		float y = n[1];
		if (x <= mx){
			mx = x;
		}
		else if(y <= my){
			my = y;
		}
		else if (x >= Mx){
			Mx = x;
		}
		else if (y >= My){
			My = y;
		}
	}
	for (int i = 0;i < m_vertexPositions.size(); i++){
		glm::vec3 pos = m_vertexPositions[i];
		glm::vec2 p(pos[0], pos[1]);
		p[0] = p[0]/(Mx-mx) - mx/(Mx - mx);
		p[1] = p[1]/(My-my) - my/(My - my);
		m_vertexTexCoords[i] = p;
	}
}


void Mesh::init () {

	if (isInitial) {
			initialPositions = m_vertexPositions;
			isInitial = false;
	}

	computePlanarParameterization();
	glCreateBuffers (1, &m_posVbo); // Generate a GPU buffer to store the positions of the vertices
	size_t vertexBufferSize = sizeof (glm::vec3) * m_vertexPositions.size (); // Gather the size of the buffer from the CPU-side vector
	glNamedBufferStorage (m_posVbo, vertexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT); // Create a data store on the GPU
	glNamedBufferSubData (m_posVbo, 0, vertexBufferSize, m_vertexPositions.data ()); // Fill the data store from a CPU array

	glCreateBuffers (1, &m_normalVbo); // Same for normal
	glNamedBufferStorage (m_normalVbo, vertexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_normalVbo, 0, vertexBufferSize, m_vertexNormals.data ());

	glCreateBuffers (1, &m_texCoordVbo); // Same for texture coordinates
	size_t texCoordBufferSize = sizeof (glm::vec2) * m_vertexTexCoords.size ();
	glNamedBufferStorage (m_texCoordVbo, texCoordBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_texCoordVbo, 0, texCoordBufferSize, m_vertexTexCoords.data ());

	glCreateBuffers (1, &m_ibo); // Same for the index buffer, that stores the list of indices of the triangles forming the mesh
	size_t indexBufferSize = sizeof (glm::uvec3) * m_triangleIndices.size ();
	glNamedBufferStorage (m_ibo, indexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_ibo, 0, indexBufferSize, m_triangleIndices.data ());

	glCreateVertexArrays (1, &m_vao); // Create a single handle that joins together attributes (vertex positions, normals) and connectivity (triangles indices)
	glBindVertexArray (m_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, m_posVbo);

	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray (1);

	glBindBuffer (GL_ARRAY_BUFFER, m_normalVbo);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray (2);

	glBindBuffer (GL_ARRAY_BUFFER, m_texCoordVbo);
	glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), 0);
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBindVertexArray (0); // Desactive the VAO just created. Will be activated at rendering time.
}

void Mesh::render () {
	glBindVertexArray (m_vao); // Activate the VAO storing geometry data
	glDrawElements (GL_TRIANGLES, static_cast<GLsizei> (m_triangleIndices.size () * 3), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}


void Mesh::clearGPU(){
	if (m_vao) {
		glDeleteVertexArrays (1, &m_vao);
		m_vao = 0;
	}
	if(m_posVbo) {
		glDeleteBuffers (1, &m_posVbo);
		m_posVbo = 0;
	}
	if (m_normalVbo) {
		glDeleteBuffers (1, &m_normalVbo);
		m_normalVbo = 0;
	}
	if (m_texCoordVbo) {
		glDeleteBuffers (1, &m_texCoordVbo);
		m_texCoordVbo = 0;
	}
	if (m_ibo) {
		glDeleteBuffers (1, &m_ibo);
		m_ibo = 0;
	}
}


void Mesh::clear () {
	m_vertexPositions.clear ();
	m_vertexNormals.clear ();
	m_vertexTexCoords.clear ();
	m_triangleIndices.clear ();

	new_vertexPositions.clear();
	final_vertexPositions.clear();
	final_vertexIndices.clear();

	clearGPU();
}
