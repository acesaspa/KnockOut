#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <../include/physx/PxPhysicsAPI.h>

#include "../include/physx/snippetcommon/SnippetPrint.h"
#include "../include/physx/snippetcommon/SnippetPVD.h"
#include "../include/physx/snippetutils/SnippetUtils.h"
#include "../include/physx/vehicle/PxVehicleUtil.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleSceneQuery.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleFilterShader.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleTireFriction.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleCreate.h"
#include "../include/physx/snippetcommon/SnippetPrint.h"
#include "../include/physx/snippetcommon/SnippetPVD.h"
#include "../include/physx/snippetutils/SnippetUtils.h"

#include "Mesh.h"
#include "Texture2D.h"
#include "Shader.cpp"


class Renderer {
public:
	void setUpRendering(glm::vec3 cameraPos, Shader ourShader/*, physx::PxPhysics* gPhysics, physx::PxCooking* gCooking, physx::PxScene* gScene*/);
	void renderGameFrame(physx::PxMat44 pxPlayerTrans,
		std::vector<physx::PxMat44> pxOpponentsTrans,
		glm::vec3 pxLevelPos,
		std::vector<physx::PxTransform> pxObjectsTrans,
		Shader ourShader,
		glm::mat4 view,
		glm::vec3 cameraPos);
	void cookMeshes(physx::PxPhysics* gPhysics, physx::PxCooking* gCooking, physx::PxScene* gScene);

private:
	void cookMesh(physx::PxPhysics* gPhysics, physx::PxCooking* gCooking, physx::PxScene* gScene, Mesh* meshToCook);

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

	Mesh jmpPowerUpMesh;
	Mesh atkPowerUpMesh;
	Mesh defPowerUpMesh;

	Texture2D playerTexture;
	Texture2D cubeTexture;
	Texture2D JmpPowerUpTexture;
	Texture2D AtkPowerUpTexture;
	Texture2D DefPowerUpTexture;

	glm::vec3 modelScalel;
};

#endif
