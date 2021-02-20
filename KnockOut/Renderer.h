#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <../include/physx/PxPhysicsAPI.h>

#include "Mesh.h"
#include "Texture2D.h"
#include "Shader.cpp"


class Renderer {
public:
	void setUpRendering(glm::vec3 cameraPos, Shader ourShader);
	void renderGameFrame(physx::PxMat44 pxPlayerTrans,
		std::vector<physx::PxMat44> pxOpponentsTrans,
		glm::vec3 pxLevelPos,
		std::vector<physx::PxTransform> pxObjectsTrans,
		Shader ourShader,
		glm::mat4 view,
		glm::vec3 cameraPos);

private:
	std::vector<Mesh> powerUpMeshes;
	std::vector<Mesh> objectMeshes;
	std::vector<Mesh> aiOpponentMeshes;

	std::vector<Texture2D> powerUpTextures;
	std::vector<Texture2D> objectTextures;
	std::vector<Texture2D> aiOpponentTextures;

	Mesh playerMesh;
	Mesh levelMesh;
	Mesh cubeMesh;

	Texture2D cubeTexture;
	Texture2D playerTexture;
	Texture2D levelTexture;

	glm::vec3 playerScale;
};

#endif
