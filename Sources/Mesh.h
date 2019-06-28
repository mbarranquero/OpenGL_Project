#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <vector>
#include <memory>
#include <iostream>
#include <map>
#include <list>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Transform.h"
//#include "OctreeNode.h"

class Mesh : public Transform {
public:
	virtual ~Mesh ();

	inline const std::vector<glm::vec3> & vertexPositions () const { return m_vertexPositions; }
	inline std::vector<glm::vec3> & vertexPositions () { return m_vertexPositions; }
	inline const std::vector<glm::vec3> & vertexNormals () const { return m_vertexNormals; }
	inline std::vector<glm::vec3> & vertexNormals () { return m_vertexNormals; }
	inline const std::vector<glm::vec2> & vertexTexCoords () const { return m_vertexTexCoords; }
	inline std::vector<glm::vec2> & vertexTexCoords () { return m_vertexTexCoords; }
	inline const std::vector<glm::uvec3> & triangleIndices () const { return m_triangleIndices; }
	inline std::vector<glm::uvec3> & triangleIndices () { return m_triangleIndices; }

	/// Compute the parameters of a sphere which bounds the mesh
	void computeBoundingSphere (glm::vec3 & center, float & radius) const;

	void recomputePerVertexNormals (bool angleBased = true);
	void computePlanarParameterization ();

	float angle(glm::vec3 x1, glm::vec3 x2, glm::vec3 x3);
	float cotangent(float a);
  float cotangent(glm::vec3 a, glm::vec3 b, glm::vec3 c);
	void laplacianFilter(float alpha  =  0.5,  bool  cotangentWeights =  true);
	void computeCotangentWeights();
	void encompass();

	std::pair<glm::vec3,glm::vec3> computeBoundingBox();
	void simplify (unsigned int resolution);
	void adaptiveSimplify (unsigned int numberOfPerLeafVertices);

	void subdivide();
	std::pair<glm::vec3,std::pair<int, int>> voisinage(const glm::uvec3& t, const glm::uvec3& s);
	std::pair<glm::vec3,std::pair<int, int>> computeOdd(int k, int l, std::vector<int> liste);

	void reinitialisation(){m_vertexPositions=initialPositions; recomputePerVertexNormals();init();}

	std::vector<std::map<int,float> > cotW;
	std::vector<std::map<int,glm::vec3> > subdV;

	void init ();
	void render ();
	void clear ();
	void clearGPU();


private:
	std::vector<glm::vec3> m_vertexPositions;
	std::vector<glm::uvec3> new_vertexPositions;
	std::vector<glm::vec3> final_vertexPositions;
	std::vector<glm::uvec3> final_vertexIndices;
	std::vector<glm::vec3> m_vertexNormals;
	std::vector<glm::vec2> m_vertexTexCoords;
	std::vector<glm::uvec3> m_triangleIndices;
	std::vector<glm::vec3> initialPositions;
	bool isInitial = true;

	std::vector<std::vector<glm::vec3>> L;

	glm::vec3 nul = glm::vec3 (0.0,0.0,0.0);

	std::vector<glm::vec3> newVertex;
	GLuint m_vao = 0;
	GLuint m_posVbo = 0;
	GLuint m_normalVbo = 0;
	GLuint m_texCoordVbo = 0;
	GLuint m_ibo = 0;
	std::list<float> block;
};

#endif // MESH_H
