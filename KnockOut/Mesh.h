#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>

#include "GL/glew.h"	
#include "glm/glm.hpp"


struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoords;
};

class Mesh
{
public:

	Mesh();
	~Mesh();

	bool loadOBJ(const std::string& filename);
	void draw();

private:

	void initBuffers();

	bool mLoaded;
	std::vector<Vertex> mVertices;
	GLuint mVBO, mVAO;
};
#endif 
