#include "Renderer.h"

#include <iostream>
#include <ctype.h>

#include "Utils.h"


glm::vec3 modelScale = glm::vec3(0.5f, 0.5f, 0.5f); //blue car




void Renderer::setUpRendering(glm::vec3 cameraPos, Shader ourShader) {
	//MARK: Other Setup
	glEnable(GL_DEPTH_TEST); //to make sure the fragment shader takes into account that some geometry has to be drawn in front of another

	//MARK: Object Setup
	Mesh cubeMesh;
	Texture2D cubeTexture;
	cubeTexture.loadTexture("container_texture.jpg", true);
	objectTextures.push_back(cubeTexture);

	cubeMesh.loadVertexData(Utils::cubeVertexData, Utils::cubeArrayLen);
	objectMeshes.push_back(cubeMesh);

	levelMesh.loadVertexData(Utils::planeVertexData, Utils::planeArrayLen);
	levelTexture.loadTexture("grass.jpg", true);

	playerMesh.loadOBJ("blueCar.obj");
	playerTexture.loadTexture("blueCar_diffuse.jpg", true);


	//MARK: Shader Setup
	ourShader.use(); //tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	ourShader.setInt("material.diffuse", 0);
	ourShader.setInt("material.specular", 1);
	ourShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
	ourShader.setVec3("viewPos", cameraPos);
	ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	ourShader.setVec3("light.diffuse", 0.6f, 0.6f, 0.6f);
	ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	ourShader.setFloat("material.shininess", 256.0f);


	//MARK: Camera Setup
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)800, 0.1f, 500.0f); //how to show perspective (fov, aspect ratio)
	ourShader.setMat4("projection", projection); //pass the projection matrix to the fragment shader

	//TODO: I might need to call shader .use() again
}




glm::vec3 Renderer::getRigidDynamicPos(physx::PxRigidDynamic* pxRigid) {
	physx::PxTransform transMat = pxRigid->getGlobalPose();
	return glm::vec3(transMat.p[0], transMat.p[1], transMat.p[2]);
}

glm::vec3 Renderer::getRigidStaticPos(physx::PxRigidStatic* pxRigid) {
	physx::PxTransform transMat = pxRigid->getGlobalPose();
	return glm::vec3(transMat.p[0], transMat.p[1], transMat.p[2]);
}








void Renderer::renderGameFrame(physx::PxVehicleDrive4W* pxPlayer, std::vector<physx::PxVehicleDrive4W*> pxOpponents, /*std::vector<physx::PxRigidDynamic*> pxObjects*/ physx::PxRigidDynamic* box, physx::PxRigidStatic* pxLevel, Shader ourShader, glm::mat4 view) {
	//TODO: texture unit binding could be causing issues, look into details

	//FRAME SETUP
	glm::mat4 model = glm::mat4(1.0f); //identity matrix
	ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader

	//GROUND
	levelTexture.unbind(0); //TODO: what are different texture units for?
	levelTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, getRigidStaticPos(pxLevel));
	ourShader.setMat4("model", model);
	levelMesh.draw();

	//PLAYER
	playerTexture.unbind(0);
	playerTexture.bind(0);
	model = Utils::getGlmMatFromPxMat(physx::PxMat44(pxPlayer->getRigidDynamicActor()->getGlobalPose()));
	model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.0f, 1.0f, 0.0f)); //fix model orientation
	model = glm::scale(model, modelScale);
	model[3][1] = model[3][1]; //adjust height
	ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)	
	playerMesh.draw();

	//OPPONENTS
	for (int i = 0; i < pxOpponents.size(); i++) {
		//playerTexture.bind(0);
		model = Utils::getGlmMatFromPxMat(pxOpponents[i]->getRigidDynamicActor()->getGlobalPose());
		model = glm::scale(model, modelScale);
		model[3][1] = model[3][1];
		ourShader.setMat4("model", model);
		playerMesh.draw();
	}

	//OBJECTS
	//for (int i = 0; i < pxObjects.size(); i++) { //TODO: boxes are either under the plane or not loaded at all
		objectTextures[0].unbind(0);
		objectTextures[0].bind(0);
		model = glm::mat4(1.0f);
		model = Utils::getGlmMatFromPxMat(physx::PxMat44(box->getGlobalPose()));
		ourShader.setMat4("model", model);
		objectMeshes[0].draw();
	//}
}