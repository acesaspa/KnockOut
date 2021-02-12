#include "Mesh.h"
#include <iostream>
#include <sstream>
#include <fstream>

Mesh::Mesh()
	:mLoaded(false)
{
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
}


bool Mesh::loadOBJ(const std::string& filename)
{
	std::vector<unsigned int> vertexIndices, uvIndices;
	std::vector<glm::vec3> tempVertices;
	std::vector<glm::vec2> tempUVs;


	if (filename.find(".obj") != std::string::npos)
	{
		std::ifstream fin(filename, std::ios::in);
		if (!fin)
		{
			std::cerr << "Cannot open " << filename << std::endl;
			return false;
		}

		std::cout << "Loading OBJ file " << filename << " ..." << std::endl;

		std::string lineBuffer;
		while (std::getline(fin, lineBuffer))
		{
			if (lineBuffer.substr(0, 2) == "v ")
			{
				std::istringstream v(lineBuffer.substr(2));
				glm::vec3 vertex;
				v >> vertex.x; v >> vertex.y; v >> vertex.z;
				tempVertices.push_back(vertex);
			}
			else if (lineBuffer.substr(0, 2) == "vt")
			{
				std::istringstream vt(lineBuffer.substr(3));
				glm::vec2 uv;
				vt >> uv.s; vt >> uv.t;
				tempUVs.push_back(uv);
			}
			else if (lineBuffer.substr(0, 2) == "f ")
			{
				int p1, p2, p3; //to store mesh index
				int t1, t2, t3; //to store texture index
				int n1, n2, n3;
				const char* face = lineBuffer.c_str();
				int match = sscanf_s(face, "f %i/%i/%i %i/%i/%i %i/%i/%i",
					&p1, &t1, &n1,
					&p2, &t2, &n2,
					&p3, &t3, &n3);
				if (match != 9)
					std::cout << "Failed to parse OBJ file using our very simple OBJ loader" << std::endl;

				// We are ignoring normals (for now)

				vertexIndices.push_back(p1);
				vertexIndices.push_back(p2);
				vertexIndices.push_back(p3);

				uvIndices.push_back(t1);
				uvIndices.push_back(t2);
				uvIndices.push_back(t3);
			}
		}

		// Close the file
		fin.close();


		// For each vertex of each triangle
		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{
			// Get the attributes using the indices
			glm::vec3 vertex = tempVertices[vertexIndices[i] - 1];
			glm::vec2 uv = tempUVs[uvIndices[i] - 1];

			Vertex meshVertex;
			meshVertex.position = vertex;
			meshVertex.texCoords = uv;

			mVertices.push_back(meshVertex);
		}

		// Create and initialize the buffers
		initBuffers();

		return (mLoaded = true);
	}

	// We shouldn't get here so return failure
	return false;
}

//-----------------------------------------------------------------------------
// Create and initialize the vertex buffer and vertex array object
// Must have valid, non-empty std::vector of Vertex objects.
//-----------------------------------------------------------------------------
void Mesh::initBuffers()
{
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices[0], GL_STATIC_DRAW);

	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

	// Vertex Texture Coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));

	// unbind to make sure other code does not change it somewhere else
	glBindVertexArray(0);
}

//-----------------------------------------------------------------------------
// Render the mesh
//-----------------------------------------------------------------------------
void Mesh::draw()
{
	if (!mLoaded) return;

	glBindVertexArray(mVAO);
	glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
	glBindVertexArray(0);
}

