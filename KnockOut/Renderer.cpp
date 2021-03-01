#include "Renderer.h"
#include <iostream>
#include <ctype.h>
#include "Utils.h"

glm::vec3 modelScale = glm::vec3(0.5f, 0.5f, 0.5f);

void Renderer::setUpRendering(glm::vec3 cameraPos, Shader ourShader) { //call once before entering the game loop
	glEnable(GL_DEPTH_TEST); //to make sure the fragment shader takes into account that some geometry has to be drawn in front of another
	ourShader.use(); //tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	ourShader.setInt("material.diffuse", 0);
	ourShader.setInt("material.specular", 1);

	//MARK: Object Setup
	jmpPowerUpMesh.loadOBJ("Powerup.obj");
	JmpPowerUpTexture.loadTexture("jumpUV.png", true);

	atkPowerUpMesh.loadOBJ("Powerup.obj");
	AtkPowerUpTexture.loadTexture("attackUV.png", true);

	defPowerUpMesh.loadOBJ("Powerup.obj");
	DefPowerUpTexture.loadTexture("shieldUV.png", true);

	playerMesh.loadOBJ("blueCar.obj");
	playerTexture.loadTexture("greenCar.png", true, true);

	levelMesh.loadVertexData(Utils::planeVertexData, Utils::planeArrayLen);
	levelTexture.loadTexture("grass.jpg", true);

	cubeMesh.loadVertexData(Utils::cubeVertexData, Utils::cubeArrayLen);
	objectMeshes.push_back(cubeMesh);
	cubeTexture.loadTexture("container_texture.jpg", true);
	objectTextures.push_back(cubeTexture);

	//MARK: Camera Setup
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)800, 0.1f, 500.0f); //how to show perspective (fov, aspect ratio)
	ourShader.setMat4("projection", projection); //pass the projection matrix to the fragment shader
}

void Renderer::renderGameFrame(physx::PxMat44 pxPlayerTrans,
	std::vector<physx::PxMat44> pxOpponentsTrans,
	glm::vec3 pxLevelPos,
	std::vector<physx::PxTransform> pxObjectsTrans,
	Shader ourShader,
	glm::mat4 view,
	glm::vec3 cameraPos){ //render a single frame of the game

	//SHADER
	ourShader.use();
	ourShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
	ourShader.setVec3("viewPos", cameraPos);
	ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	ourShader.setVec3("light.diffuse", 0.6f, 0.6f, 0.6f);
	ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	ourShader.setFloat("material.shininess", 256.0f);
	ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader

	//TODO: what are different texture units for?

	//PLAYER (drawing a vehicle)
	playerTexture.bind(0);
	glm::mat4 model = glm::mat4(1.0f); //identity matrix
	model = Utils::getGlmMatFromPxMat(pxPlayerTrans);
	model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.0f, 1.0f, 0.0f)); //fix model orientation
	model = glm::scale(model, modelScale);
	ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)	
	playerMesh.draw();

	//OPPONENTS
	for (int i = 0; i < pxOpponentsTrans.size(); i++) {
		playerTexture.bind(0);
		model = Utils::getGlmMatFromPxMat(pxOpponentsTrans[i]);
		model = glm::scale(model, modelScale);
		model[3][1] = model[3][1];
		ourShader.setMat4("model", model);
		playerMesh.draw();
	}

	//GROUND (drawing a static object)
	levelTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, pxLevelPos);
	ourShader.setMat4("model", model);
	levelMesh.draw();

	//OBJECTS (draw a dynamic object)
	for (int i = 0; i < pxObjectsTrans.size(); i++) { //TODO: boxes are either under the plane or not loaded at all
		objectTextures[0].bind(0);
		model = glm::mat4(1.0f);
		model = Utils::getGlmMatFromPxMat(physx::PxMat44(pxObjectsTrans[i]));
		ourShader.setMat4("model", model);
		objectMeshes[0].draw();
	}

	//POWERUPS
	JmpPowerUpTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.0f, 10.0f));
	ourShader.setMat4("model", model);
	jmpPowerUpMesh.draw();

	AtkPowerUpTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(4.0f, 1.0f, 10.0f));
	ourShader.setMat4("model", model);
	atkPowerUpMesh.draw();

	DefPowerUpTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-5.0f, 1.0f, 10.0f));
	ourShader.setMat4("model", model);
	defPowerUpMesh.draw();
}