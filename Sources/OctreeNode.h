#ifndef OCTREENODE_H
#define OCTREENODE_H

#include <glad/glad.h>
#include <vector>
#include <memory>
#include <iostream>
#include <map>
#include <list>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class OctreeNode {
public:

  /*std::shared_ptr<OctreeNode> children[8];

  glm::vec3 minB;
  glm::vec3 maxB;
  glm::vec3 center;
  glm::vec3 halfSize;
  bool IsLeaf = false;
  std::vector<int> verticesIdInside;
  */

  std::shared_ptr<OctreeNode> children[8];
	float x1,x2,y1,y2,z1,z2;
	std::vector<int>  verticesIdTab;
	std::vector<glm::vec3> * vPosPtr;
	bool isLeaf = false;
  int leafNumber;
  
  std::vector<int> vertexLeafId;
  std::vector<std::vector<int>> treeVector;
  int leafId;

  //std::shared_ptr<OctreeNode> buildOctree(unsigned int numOfPerLeafVertices, std::vector<int> verticesIdInside, glm::vec3 minBox, glm::vec3 maxBox,const std::vector<glm::vec3> & m_vertexPositions, const std::vector<glm::vec3> & m_vertexNormals, std::vector<int> &representativeOfVertex, int &numberOfLeavesYet);

  std::shared_ptr<OctreeNode> buildOctree(float x10,float x20,float y10,float y20,float z10,float z20,std::vector<int> verticesIdTab0,std::vector<glm::vec3> * vPosPtr0,int numOfPerLeafVertices);
  void simplifyOctree(std::shared_ptr<OctreeNode> tree, std::vector<glm::vec3> * vPosPtr,std::vector<glm::uvec3>* triPtr);

};
#endif // OCTREENODE_H
