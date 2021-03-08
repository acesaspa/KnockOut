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

#include <map>

class Renderer {
public:
	void setUpRendering(glm::vec3 cameraPos, Shader ourShader/*, physx::PxPhysics* gPhysics, physx::PxCooking* gCooking, physx::PxScene* gScene*/);
	void renderGameFrame(physx::PxMat44 pxPlayerTrans,
		std::vector<physx::PxMat44> pxOpponentsTrans,
		glm::vec3 pxLevelPos,
		std::vector<physx::PxTransform> pxObjectsTrans,
		Shader ourShader,
		glm::mat4 view,
		glm::vec3 cameraPos,
		int status,
		bool jump,
		bool attack,
		bool defense,
		float interX,
		float interY);
	std::vector<Mesh*> getGroundMeshes(int index);

private:
	void renderObject(Shader ourShader, Mesh* meshToRender, Texture2D* textureToApply, glm::vec3 translation = glm::vec3(0.f, 0.f, 0.f), glm::vec3 rotationAxis = glm::vec3(0.f, 0.f, 0.f),
		float rotationAmountDeg = 0.f, glm::vec3 scale = glm::vec3(0.f, 0.f, 0.f), physx::PxMat44 pxTransMat = physx::PxIdentity);
	void applyShaderValues(Shader ourShader, glm::vec3 cameraPos, glm::mat4 view);

	std::vector<Mesh> objectMeshes;
	std::vector<Mesh> aiOpponentMeshes;

	std::vector<Texture2D> powerUpTextures;
	std::vector<Texture2D> objectTextures;
	std::vector<Texture2D> aiOpponentTextures;

	Mesh citySurfaceMesh;
	Mesh grassSurfaceMesh;
	Mesh desertSurfaceMesh;

	Texture2D grassTexture;
	Texture2D desertTexture;
	Texture2D cityTexture;

	Mesh cubeMesh;
	Mesh playerMesh;

	Mesh GameOverMesh;
	Texture2D GameOverTexture;
	Mesh YouWinMesh;
	Texture2D YouWinTexture;

	Mesh jmpPowerUpMesh;
	Mesh atkPowerUpMesh;
	Mesh defPowerUpMesh;

	Texture2D playerTexture;
	Texture2D cubeTexture;
	Texture2D JmpPowerUpTexture;
	Texture2D AtkPowerUpTexture;
	Texture2D DefPowerUpTexture;
};

#endif
