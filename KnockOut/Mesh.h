#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>

#include "GL/glew.h"	
#include "glm/glm.hpp"
#include "../include/physx/PxPhysicsAPI.h"


struct Vertex
{
	glm::vec3 position;
	glm::vec3 normals;
	glm::vec2 texCoords;
};

class Mesh
{
public:
	Mesh();
	~Mesh();

	bool loadOBJ(const std::string& filename);
	void loadVertexData(float vertexData[], int arraySize);
	void draw();

	std::vector<physx::PxU32>* getVertexIndices();
	std::vector<physx::PxVec3>* getActualVertices();
	std::vector<glm::vec3> getBoundingBoxVertices();
	void setIsMostOuterLevel(bool isOuter);

private:
	void initBuffers();
	void generateBoundingBox();
	bool mLoaded;
	bool isMostOuterLevel = false;
	std::vector<Vertex> mVertices;
	std::vector<glm::vec3> actualVertices;
	GLuint mVBO, mVAO;
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> boundingBox;

	std::vector<physx::PxVec3> adjustedVertices;
};
#endif 
