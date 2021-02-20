//adapted from: https://www.udemy.com/course/learn-modern-opengl-programming/

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
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> tempVertices;
	std::vector<glm::vec2> tempUVs;
	std::vector<glm::vec3> tempNormals;

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
			else if (lineBuffer.substr(0, 2) == "vn") {
				std::istringstream vn(lineBuffer.substr(3));
				glm::vec3 norm;
				vn >> norm.x; vn >> norm.y; vn >> norm.z;
				tempNormals.push_back(norm);
			}
			else if (lineBuffer.substr(0, 2) == "f ")
			{
				int p1, p2, p3; //to store mesh index
				int t1, t2, t3; //to store texture index
				int n1, n2, n3; //to store normal index
				const char* face = lineBuffer.c_str();
				int match = sscanf_s(face, "f %i/%i/%i %i/%i/%i %i/%i/%i",
					&p1, &t1, &n1,
					&p2, &t2, &n2,
					&p3, &t3, &n3);
				if (match != 9)
					std::cout << "Failed to parse OBJ file using our very simple OBJ loader" << std::endl;


				normalIndices.push_back(n1);
				normalIndices.push_back(n2);
				normalIndices.push_back(n3);

				vertexIndices.push_back(p1);
				vertexIndices.push_back(p2);
				vertexIndices.push_back(p3);

				uvIndices.push_back(t1);
				uvIndices.push_back(t2);
				uvIndices.push_back(t3);
			}
		}
		fin.close();

		for (unsigned int i = 0; i < vertexIndices.size(); i++) // For each vertex of each triangle
		{
			glm::vec3 vertex = tempVertices[vertexIndices[i] - 1]; // Get the attributes using the indices
			glm::vec2 uv = tempUVs[uvIndices[i] - 1];
			glm::vec3 normal = tempNormals[normalIndices[i] - 1];

			Vertex meshVertex;
			meshVertex.position = vertex;
			meshVertex.normals = normal;
			meshVertex.texCoords = uv;

			mVertices.push_back(meshVertex);
		}

		initBuffers();
		return (mLoaded = true);
	}

	return false;
}


void Mesh::initBuffers() //Must have valid, non-empty std::vector of Vertex objects.
{
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // Vertex Positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

	glEnableVertexAttribArray(1); // Vertex Normals
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));

	glEnableVertexAttribArray(2); // Vertex Texture Coords
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(6 * sizeof(GLfloat)));

	glBindVertexArray(0); // unbind to make sure other code does not change it somewhere else
}

void Mesh::draw()
{
	if (!mLoaded) return;

	glBindVertexArray(mVAO);
	glDrawArrays(GL_TRIANGLES, 0, mVertices.size());
	glBindVertexArray(0);
}





void Mesh::loadVertexData(float vertexData[], int arraySize) { //stride size 8 - vertex x, y, z; normal x, y, z; tex coord x, y;
	for (unsigned int i = 0; i < arraySize; i = i + 8) // For each vertex of each triangle
	{
		glm::vec3 vertex, normal;
		glm::vec2 tex;

		vertex.x = vertexData[i];
		vertex.y = vertexData[i + 1];
		vertex.z = vertexData[i + 2];

		normal.x = vertexData[i + 3];
		normal.y = vertexData[i + 4];
		normal.z = vertexData[i + 5];

		tex.x = vertexData[i + 6];
		tex.y = vertexData[i + 7];

		Vertex meshVertex;
		meshVertex.position = vertex;
		meshVertex.normals = normal;
		meshVertex.texCoords = tex;


		//std::cout <<
		//	meshVertex.position.x << ", " <<
		//	meshVertex.position.y << ", " <<
		//	meshVertex.position.z << ", " <<

		//	meshVertex.normals.x << ", " <<
		//	meshVertex.normals.y << ", " <<
		//	meshVertex.normals.z << ", " <<

		//	meshVertex.texCoords.x << ", " <<
		//	meshVertex.texCoords.y <<
		//	std::endl;

		mVertices.push_back(meshVertex);
	}

	//for (int i = 0; i < mVertices.size(); i++) {
	//	std::cout <<
	//		mVertices[i].position.x << ", " <<
	//		mVertices[i].position.y << ", " <<
	//		mVertices[i].position.z << ", " <<

	//		mVertices[i].normals.x << ", " <<
	//		mVertices[i].normals.y << ", " <<
	//		mVertices[i].normals.z << ", " <<

	//		mVertices[i].texCoords.x << ", " <<
	//		mVertices[i].texCoords.y <<
	//	std::endl;
	//}

	initBuffers();
	mLoaded = true;
}

