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
	void renderGameFrame(physx::PxVehicleDrive4W* pxPlayer,
		std::vector<physx::PxVehicleDrive4W*> pxOpponents,
		//std::vector<physx::PxRigidDynamic*> pxObjects,
		physx::PxRigidDynamic* box,
		physx::PxRigidStatic* pxLevel,
		Shader ourShader,
		glm::mat4 view);

private:
	glm::vec3 getRigidDynamicPos(physx::PxRigidDynamic* pxRigid);
	glm::vec3 getRigidStaticPos(physx::PxRigidStatic* pxRigid);

	std::vector<Mesh> powerUpMeshes;
	std::vector<Mesh> objectMeshes;
	std::vector<Mesh> aiOpponentMeshes;

	std::vector<Texture2D> powerUpTextures;
	std::vector<Texture2D> objectTextures;
	std::vector<Texture2D> aiOpponentTextures;

	Mesh playerMesh;
	Mesh levelMesh;

	Texture2D playerTexture;
	Texture2D levelTexture;
};

#endif
