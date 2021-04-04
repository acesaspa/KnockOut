#ifndef RENDERER_H
#define RENDERER_H

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <../include/physx/PxPhysicsAPI.h>

#include "Mesh.h"
#include "Texture2D.h"
#include "Shader.cpp"
#include <list>
#include "PowerUp.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>


struct Character {
	unsigned int TextureID; // ID handle of the glyph texture
	glm::ivec2   Size;      // Size of glyph
	glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	unsigned int Advance;   // Horizontal offset to advance to next glyph
};

class Renderer {
public:
	int getUIBoost();
	void setUIBoost(int x);
	void setUpRendering(glm::vec3 cameraPos, Shader ourShader, Shader textShader, Shader skyboxShader, Shader depthShader);
	void renderGameFrame(physx::PxMat44 pxPlayerTrans,
		physx::PxMat44 pxUITrans,
		std::vector<physx::PxMat44> pxOpponentsTrans,
		glm::vec3 pxLevelPos,
		std::vector<physx::PxTransform> pxObjectsTrans,
		Shader ourShader,
		Shader textShader,
		Shader skyboxShader,
		Shader depthShader,
		glm::mat4 view,
		glm::vec3 cameraPos,
		int status,
		std::list<PowerUp*>& powerups,
		int gameStatus);
	std::vector<Mesh*> getGroundMeshes(int index);
	std::vector<glm::vec3> getLevelBB();
	void flashSegment(bool keepFlashing);

	std::vector<glm::vec3> testLocs;

private:
	void renderObject(Shader ourShader, Mesh* meshToRender, Texture2D* textureToApply, glm::vec3 translation = glm::vec3(0.f, 0.f, 0.f), glm::vec3 rotationAxis = glm::vec3(0.f, 0.f, 0.f),
		float rotationAmountDeg = 0.f, glm::vec3 scale = glm::vec3(0.f, 0.f, 0.f), physx::PxMat44 pxTransMat = physx::PxIdentity);
	void setMainShader(Shader ourShader, glm::vec3 cameraPos, glm::mat4 view, int status);
	void renderScene(Shader& shader,
		physx::PxMat44 pxPlayerTrans,
		physx::PxMat44 pxUITrans,
		std::vector<physx::PxMat44> pxOpponentsTrans,
		glm::vec3 pxLevelPos,
		std::vector<physx::PxTransform> pxObjectsTrans,
		glm::mat4 view,
		glm::vec3 cameraPos,
		int carsRemoved,
		std::list<PowerUp*>& powerups,
		int gameStatus);
	void prepShadows(Shader depthShader, glm::mat4 projection);
	void setDepthShader(Shader depthShader, glm::vec3 lightPos);

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
	Mesh treeMesh;

	Mesh MainMenuScreen;
	Texture2D MainMenuTexture;
	Mesh GameOverMesh;
	Texture2D GameOverTexture;
	Mesh YouWinMesh;
	Texture2D YouWinTexture;

	Mesh NoBoostUI;
	Texture2D NoBoostTxt;
	Mesh JumpUI;
	Texture2D JumpTxt;
	Mesh AttackUI;
	Texture2D AttackTxt;
	Mesh DefendUI;
	Texture2D DefendTxt;

	Mesh jmpPowerUpMesh;
	Mesh atkPowerUpMesh;
	Mesh defPowerUpMesh;

	Texture2D playerTexture;
	Texture2D cubeTexture;
	Texture2D treeTexture;
	Texture2D JmpPowerUpTexture;
	Texture2D AtkPowerUpTexture;
	Texture2D DefPowerUpTexture;
};

#endif
