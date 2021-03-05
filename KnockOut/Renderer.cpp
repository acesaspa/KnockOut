#include "Renderer.h"
#include <iostream>
#include <ctype.h>
#include "Utils.h"
#include <map>

glm::vec3 modelScale = glm::vec3(0.5f, 0.6f, 0.5f);

void Renderer::cookMeshes(physx::PxPhysics* gPhysics, physx::PxCooking* gCooking, physx::PxScene* gScene) { //call once physx ready, this will cook all necessary meshes
	cookMesh(gPhysics, gCooking, gScene, &citySurfaceMesh);
	cookMesh(gPhysics, gCooking, gScene, &grassSurfaceMesh);
	cookMesh(gPhysics, gCooking, gScene, &desertSurfaceMesh);
}


void Renderer::cookMesh(physx::PxPhysics* gPhysics, physx::PxCooking* gCooking, physx::PxScene* gScene, Mesh* meshToCook) { //cook a tri mesh and add it to the physics scene
	physx::PxMaterial* gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f); //create some material
	physx::PxTriangleMeshDesc meshDesc; //mesh cooking from a triangle mesh

	std::vector<physx::PxVec3> vertices = meshToCook->getActualVertices();
	std::vector<physx::PxU32> indices = meshToCook->getVertexIndices();

	meshDesc.points.count = vertices.size(); //total number of vertices
	meshDesc.points.stride = sizeof(physx::PxVec3);
	meshDesc.points.data = reinterpret_cast<const void*>(vertices.data());

	meshDesc.triangles.count = indices.size() / 3; //total number of triangles (each index = 1 vertex, so divide by 3 to get the num of triangles)
	meshDesc.triangles.stride = 3 * sizeof(physx::PxU32);
	meshDesc.triangles.data = reinterpret_cast<const void*>(indices.data());

	//PxCookingParams params = gCooking->getParams();
	////TODO: potentially do this
	//gCooking->setParams(params);

	//physx::PxFilterData myData = physx::PxFilterData();
	//myData.word0 = 14;
	//myData.word1 = 2;

	physx::PxFilterData groundPlaneSimFilterData(snippetvehicle::COLLISION_FLAG_GROUND, snippetvehicle::COLLISION_FLAG_GROUND_AGAINST, 0, 0);

	physx::PxTriangleMesh* triMesh = NULL;
	physx::PxU32 meshSize = 0;
	triMesh = gCooking->createTriangleMesh(meshDesc, gPhysics->getPhysicsInsertionCallback()); //insert the cooked mesh directly into PxPhysics
	physx::PxRigidStatic* meshBody = gPhysics->createRigidStatic(physx::PxTransform(physx::PxVec3(0, 0, 0))); //create a rigid body for the cooked mesh
	physx::PxShape* meshShape = gPhysics->createShape(physx::PxTriangleMeshGeometry(triMesh), *gMaterial); //create a shape from the cooked mesh
	
	physx::PxFilterData qryFilterData;
	snippetvehicle::setupDrivableSurface(qryFilterData);
	meshShape->setQueryFilterData(qryFilterData);
	meshShape->setSimulationFilterData(groundPlaneSimFilterData);
	
	meshShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	meshBody->attachShape(*meshShape); //attach the shape to the body
	gScene->addActor(*meshBody); //and add it to the scene
	triMesh->release(); //clean up
}



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
	playerTexture.loadTexture("greenCar.png", true, true);

	citySurfaceMesh.loadOBJ("cityLevelNoHoles.obj");
	cityTexture.loadTexture("asphalt.jpg", true);

	grassSurfaceMesh.loadOBJ("grassLevelNoHoles.obj");
	grassTexture.loadTexture("grass.jpg", true);

	desertSurfaceMesh.loadOBJ("sandLevelNoHoles.obj");
	desertTexture.loadTexture("desert_texture.jpg", true);

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
	glm::vec3 cameraPos,
	int status,
	bool jump,
	bool attack,
	bool defense){ //render a single frame of the game

	//SHADER
	ourShader.use();
	ourShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
	ourShader.setVec3("viewPos", cameraPos);
	ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	ourShader.setVec3("light.diffuse", 0.6f, 0.6f, 0.6f);
	ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	ourShader.setFloat("material.shininess", 256.0f);
	ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader
	ourShader.setVec3("textColor", 0.f,0.f,0.f);

	//TODO: what are different texture units for?

	//PLAYER (drawing a vehicle)
	playerTexture.bind(0);
	glm::mat4 model = glm::mat4(1.0f); //identity matrix
	model = Utils::getGlmMatFromPxMat(pxPlayerTrans);
	model = glm::translate(model, glm::vec3(0.f, -1.f, 0.f));
	model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.0f, 1.0f, 0.0f)); //fix model orientation
	model = glm::scale(model, modelScale);
	ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)	
	playerMesh.draw();

	//OPPONENTS
	for (int i = 0; i < pxOpponentsTrans.size(); i++) {
		playerTexture.bind(0);
		model = Utils::getGlmMatFromPxMat(pxOpponentsTrans[i]);
		model = glm::translate(model, glm::vec3(0.f, -1.f, 0.f));
		model = glm::scale(model, modelScale);
		model[3][1] = model[3][1];
		ourShader.setMat4("model", model);
		playerMesh.draw();
	}

	//GROUND (drawing a static object)
	cityTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
	ourShader.setMat4("model", model);
	citySurfaceMesh.draw();

	grassTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
	ourShader.setMat4("model", model);
	grassSurfaceMesh.draw();

	desertTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.f, 0.f, 0.f));
	ourShader.setMat4("model", model);
	desertSurfaceMesh.draw();


	//OBJECTS (draw a dynamic object)
	for (int i = 0; i < pxObjectsTrans.size(); i++) { //TODO: boxes are either under the plane or not loaded at all
		objectTextures[0].bind(0);
		model = glm::mat4(1.0f);
		model = Utils::getGlmMatFromPxMat(physx::PxMat44(pxObjectsTrans[i]));
		ourShader.setMat4("model", model);
		objectMeshes[0].draw();
	}

	//Game Over
	GameOverTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-30.0f, 1.0f, 10.0f));
	ourShader.setMat4("model", model);
	if (status == 1) {
		GameOverMesh.draw();
	}

	//You Win
	YouWinTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-30.0f, 1.0f, 10.0f));
	ourShader.setMat4("model", model);
	if (status == 2) {
		YouWinMesh.draw();
	}

	//POWERUPS
	JmpPowerUpTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(20.0f, 1.0f, 10.0f));
	ourShader.setMat4("model", model);
	if (jump) {
		jmpPowerUpMesh.draw();
	}

	AtkPowerUpTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(20.0f, 1.0f, 10.0f));
	ourShader.setMat4("model", model);
	if (attack) {
		atkPowerUpMesh.draw();
	}

	DefPowerUpTexture.bind(0);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(20.0f, 1.0f, 10.0f));
	ourShader.setMat4("model", model);
	if (defense) {
		defPowerUpMesh.draw();
	}
}