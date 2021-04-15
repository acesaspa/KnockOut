//adapted from: https://www.udemy.com/course/learn-modern-opengl-programming/

#include "Mesh.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>

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
				actualVertices.push_back(vertex);
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
			glm::vec3 vertex = actualVertices[vertexIndices[i] - 1]; // Get the attributes using the indices
			glm::vec2 uv = tempUVs[uvIndices[i] - 1];
			glm::vec3 normal = tempNormals[normalIndices[i] - 1];

			Vertex meshVertex;
			meshVertex.position = vertex;
			meshVertex.normals = normal;
			meshVertex.texCoords = uv;

			mVertices.push_back(meshVertex);
		}


		adjustedVertices.push_back(physx::PxVec3(0.0, 0.0, 0.0)); //actual vertices in PxVec3 format and zero adjusted
		for (int i = 0; i < actualVertices.size(); i++) {
			physx::PxVec3 newVertex;
			newVertex.x = actualVertices[i].x;
			newVertex.y = actualVertices[i].y;
			newVertex.z = actualVertices[i].z;
			adjustedVertices.push_back(newVertex);
		}
		generateBoundingBox();

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

		mVertices.push_back(meshVertex);
	}

	initBuffers();
	mLoaded = true;
}

std::vector<physx::PxVec3>* Mesh::getActualVertices() {
	return &adjustedVertices;
}

std::vector<physx::PxU32>* Mesh::getVertexIndices() {
	return &vertexIndices;
}

std::vector<glm::vec3> Mesh::getBoundingBoxVertices() {
	//if(boundingBox.size() < 1) generateBoundingBox();
	return boundingBox;
}

void Mesh::generateBoundingBox() { //populates the boundingBox vector with bounding box vertices based on scale and position on the map
	int xMaxIndex, xMinIndex, zMaxIndex, zMinIndex, yMaxIndex, yMinIndex; //indices for the corresponding mins and maxes
	float xMax = FLT_MIN, xMin = FLT_MAX, zMax = FLT_MIN, zMin = FLT_MAX, yMax = FLT_MIN, yMin = FLT_MAX;

	for (int i = 0; i < actualVertices.size(); i++) {
		if (actualVertices[i].x > xMax) { //X coord
			xMax = actualVertices[i].x;
			xMaxIndex = i;
		}

		if (actualVertices[i].x < xMin) {
			xMin = actualVertices[i].x;
			xMinIndex = i;
		}


		if (actualVertices[i].y > yMax) { //Y coord
			yMax = actualVertices[i].y;
			yMaxIndex = i;
		}

		if (actualVertices[i].y < yMin) {
			yMin = actualVertices[i].y;
			yMinIndex = i;
		}


		if (actualVertices[i].z > zMax) { //Z coord
			zMax = actualVertices[i].z;
			zMaxIndex = i;
		}

		if (actualVertices[i].z < zMin) {
			zMin = actualVertices[i].z;
			zMinIndex = i;
		}
	}

	if (isMostOuterLevel) {
		yMax = 50.f;
		yMin = 0.f;
	}

	boundingBox.push_back(glm::vec3(xMin, yMin, zMin)); //1
	boundingBox.push_back(glm::vec3(xMin, yMax, zMin));
	boundingBox.push_back(glm::vec3(xMax, yMin, zMin));

	boundingBox.push_back(glm::vec3(xMax, yMax, zMin)); //2
	boundingBox.push_back(glm::vec3(xMax, yMin, zMin));
	boundingBox.push_back(glm::vec3(xMin, yMax, zMin));

	boundingBox.push_back(glm::vec3(xMax, yMax, zMin)); //3
	boundingBox.push_back(glm::vec3(xMax, yMin, zMin));
	boundingBox.push_back(glm::vec3(xMax, yMin, zMax));

	boundingBox.push_back(glm::vec3(xMax, yMax, zMax)); //4
	boundingBox.push_back(glm::vec3(xMax, yMin, zMax));
	boundingBox.push_back(glm::vec3(xMin, yMax, zMin));

	boundingBox.push_back(glm::vec3(xMax, yMax, zMax)); //5
	boundingBox.push_back(glm::vec3(xMax, yMin, zMax));
	boundingBox.push_back(glm::vec3(zMin, yMin, zMax));

	boundingBox.push_back(glm::vec3(xMin, yMin, zMax)); //6
	boundingBox.push_back(glm::vec3(xMin, yMax, zMax));
	boundingBox.push_back(glm::vec3(xMax, yMax, zMax));

	boundingBox.push_back(glm::vec3(xMin, yMin, zMax)); //7
	boundingBox.push_back(glm::vec3(xMin, yMax, zMax));
	boundingBox.push_back(glm::vec3(xMin, yMin, zMin));

	boundingBox.push_back(glm::vec3(xMin, yMax, zMin)); //8
	boundingBox.push_back(glm::vec3(xMin, yMin, zMin));
	boundingBox.push_back(glm::vec3(xMin, yMax, zMax));
}


void Mesh::setIsMostOuterLevel(bool isOuter) {
	isMostOuterLevel = isOuter;
}




