#include "Renderer.h"
#include <iostream>
#include <ctype.h>
#include "Utils.h"
#include <map>

glm::vec3 vehicleScale = glm::vec3(0.5f, 0.6f, 0.5f);
glm::vec3 powerUpScale = glm::vec3(0.5f, 0.5f, 0.5f);
glm::vec3 levelScale = glm::vec3(1.f, 1.f, 1.f);
glm::vec3 defaultScale = glm::vec3(1.f, 1.0f, 1.f);
glm::vec3 defaultRotation = glm::vec3(0.f, 1.0f, 0.f);
glm::vec3 worldOrigin = glm::vec3(0.f, 0.0f, 0.f);
float defaultRotAmountDeg = 0.f;



void Renderer::setUpRendering(glm::vec3 cameraPos, Shader ourShader) { //call once before entering the game loop
	glEnable(GL_DEPTH_TEST); //to make sure the fragment shader takes into account that some geometry has to be drawn in front of another
	ourShader.use(); //tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	ourShader.setInt("material.diffuse", 0);
	ourShader.setInt("material.specular", 1);

	//MARK: Object Setup
	GameOverMesh.loadOBJ("Powerup.obj");
	GameOverTexture.loadTexture("gameOverUV.png", true);

	YouWinMesh.loadOBJ("Powerup.obj");
	YouWinTexture.loadTexture("youWinUV.png", true);

	jmpPowerUpMesh.loadOBJ("Powerup.obj");
	JmpPowerUpTexture.loadTexture("jumpUV.png", true);

	atkPowerUpMesh.loadOBJ("Powerup.obj");
	AtkPowerUpTexture.loadTexture("attackUV.png", true);

	defPowerUpMesh.loadOBJ("Powerup.obj");
	DefPowerUpTexture.loadTexture("shieldUV.png", true);

	playerMesh.loadOBJ("blueCar.obj");
	playerTexture.loadTexture("blueCar.png", true, true);

	citySurfaceMesh.loadOBJ("cityLevel.obj");
	cityTexture.loadTexture("asphalt.jpg", true);

	grassSurfaceMesh.loadOBJ("grassLevel.obj");
	grassTexture.loadTexture("grass.jpg", true);

	desertSurfaceMesh.loadOBJ("sandLevel.obj");
	desertTexture.loadTexture("desert_texture.jpg", true);

	cubeMesh.loadVertexData(Utils::cubeVertexData, Utils::cubeArrayLen);
	objectMeshes.push_back(cubeMesh);
	cubeTexture.loadTexture("container_texture.jpg", true);
	objectTextures.push_back(cubeTexture);

	//MARK: Camera Setup
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)1200 / (float)800, 0.1f, 500.0f); //how to show perspective (fov, aspect ratio)
	ourShader.setMat4("projection", projection); //pass the projection matrix to the fragment shader
}

///render a single frame of the game
void Renderer::renderGameFrame(physx::PxMat44 pxPlayerTrans, //TODO: what are different texture units for?
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
	float interY
	
	){ //render a single frame of the game

	applyShaderValues(ourShader, cameraPos, view);

	//PLAYER
	glm::mat4 model = glm::mat4(1.0f); //identity matrix
	renderObject(ourShader, &playerMesh, &playerTexture, glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f), 180.f, vehicleScale, pxPlayerTrans);

	//OPPONENTS
	for (int i = 0; i < pxOpponentsTrans.size(); i++) 
		renderObject(ourShader, &playerMesh, &playerTexture, glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f), 180.f, vehicleScale, pxOpponentsTrans[i]);

	//GROUND
	renderObject(ourShader, &citySurfaceMesh, &cityTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, levelScale);
	renderObject(ourShader, &grassSurfaceMesh, &grassTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, levelScale);
	if (status == 0) { renderObject(ourShader, &desertSurfaceMesh, &desertTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, levelScale); }

	//OBJECTS
	for (int i = 0; i < pxObjectsTrans.size(); i++)
		renderObject(ourShader, &objectMeshes[0], &objectTextures[0], worldOrigin, defaultRotation, defaultRotAmountDeg, defaultScale, pxObjectsTrans[i]);

	//POWERUPS
	if(jump) renderObject(ourShader, &jmpPowerUpMesh, &JmpPowerUpTexture, glm::vec3(20.0f, 1.0f, 10.0f), defaultRotation, defaultRotAmountDeg, powerUpScale);
	if(attack) renderObject(ourShader, &atkPowerUpMesh, &AtkPowerUpTexture, glm::vec3(20.0f, 1.0f, 10.0f), defaultRotation, defaultRotAmountDeg, powerUpScale);
	if(defense) renderObject(ourShader, &defPowerUpMesh, &DefPowerUpTexture, glm::vec3(20.0f, 1.0f, 10.0f), defaultRotation, defaultRotAmountDeg, powerUpScale);


	//Game Over
	if(status == 1) renderObject(ourShader, &GameOverMesh, &GameOverTexture, glm::vec3(-30.0f, 1.0f, 10.0f), defaultRotation, defaultRotAmountDeg, powerUpScale);

	//You Win
	if(status == 2) renderObject(ourShader, &YouWinMesh, &YouWinTexture, glm::vec3(-30.0f, 1.0f, 10.0f), defaultRotation, defaultRotAmountDeg, powerUpScale);


	//AI test
	renderObject(ourShader, &objectMeshes[0], &objectTextures[0], glm::vec3(interX, 1.f, interY), defaultRotation, defaultRotAmountDeg, defaultScale);
}








void Renderer::applyShaderValues(Shader ourShader, glm::vec3 cameraPos, glm::mat4 view) {
	ourShader.use();
	ourShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
	ourShader.setVec3("viewPos", cameraPos);
	ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	ourShader.setVec3("light.diffuse", 0.6f, 0.6f, 0.6f);
	ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	ourShader.setFloat("material.shininess", 256.0f);
	ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader
	ourShader.setVec3("textColor", 0.f, 0.f, 0.f);
}

void Renderer::renderObject(Shader ourShader, Mesh* meshToRender, Texture2D* textureToApply, glm::vec3 translation, glm::vec3 rotationAxis,
	float rotationAmountDeg, glm::vec3 scale, physx::PxMat44 pxTransMat) { //render a single object for a single frame, passing in a px transformation matrix automatically overrides all other transformations

	textureToApply->bind(0);
	glm::mat4 model = glm::mat4(1.0f); //identity matrix
	model = Utils::getGlmMatFromPxMat(pxTransMat);
	model = glm::translate(model, translation);
	model = glm::rotate(model, glm::radians(rotationAmountDeg), rotationAxis); //fix model orientation
	model = glm::scale(model, scale);
	ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)	
	meshToRender->draw();
}

std::vector<Mesh*> Renderer::getGroundMeshes(int index) { //returns pointers to all ground meshes, cannot be called before setup
	if (index == 1) {
		std::vector<Mesh*> meshes;
		meshes.push_back(&citySurfaceMesh);
		meshes.push_back(&grassSurfaceMesh);
		meshes.push_back(&desertSurfaceMesh);
		return meshes;
	}
	if (index == 2) {
		std::vector<Mesh*> meshes;
		meshes.push_back(&citySurfaceMesh);
		meshes.push_back(&grassSurfaceMesh);
		return meshes;
	}
}