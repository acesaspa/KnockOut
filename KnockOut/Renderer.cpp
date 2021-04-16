#include "Renderer.h"
#include <iostream>
#include <ctype.h>
#include "Utils.h"
#include <map>
#include <stb_image.h>
#include <list>
#include "PowerUp.h"
#include "Skybox.h"



//VARS
glm::vec3 vehicleScale = glm::vec3(0.5f, 0.6f, 0.5f);
glm::vec3 powerUpScale = glm::vec3(0.5f, 0.5f, 0.5f);
glm::vec3 levelScale = glm::vec3(1.f, 1.f, 1.f);
glm::vec3 defaultScale = glm::vec3(1.f, 1.0f, 1.f);
glm::vec3 defaultRotation = glm::vec3(0.f, 1.0f, 0.f);
glm::vec3 worldOrigin = glm::vec3(0.f, 0.0f, 0.f);
float defaultRotAmountDeg = 0.f;

float screenRotDeg = 180.0f;
glm::vec3 screenRotation = glm::vec3(1.f, -.10f, 0.f);
glm::vec3 screenScale = glm::vec3(3.f, 3.f, 3.f);

float UIRotDeg = 90.f;
glm::vec3 UITranslation = glm::vec3(1.42f, -.11f, -1.9f);
glm::vec3 UIRot = glm::vec3(0.f, 0.f, 1.f);
glm::vec3 UIScale = glm::vec3(1.f, 1.f, 1.f);

// 0 = no boost
// 1 = jump boost
// 2 = attack boost
// 3 = defense boost
int uiBoost = 0;

unsigned long frameCounter = 0;
bool flashLevel = false;
int noCarsRemoved = 0;
Skybox skybox;
const unsigned int SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;
unsigned int depthMapFBO, depthMap;
float near_plane = 50.f, far_plane = 300.f;
glm::vec3 lightPos(0.f, 100.0f, 10.f);
glm::mat4 lightSpaceMatrix;

//text
std::map<GLchar, Character> Characters;
unsigned int textVAO, textVBO;










void Renderer::setUpRendering(glm::vec3 cameraPos, Shader ourShader, Shader textShader, Shader skyboxShader, Shader depthShader) { //call once before entering the game loop
	glEnable(GL_DEPTH_TEST); //to make sure the fragment shader takes into account that some geometry has to be drawn in front of another
	ourShader.use(); //tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	ourShader.setInt("material.diffuse", 0);
	ourShader.setInt("material.specular", 1);

	//Screens
	MainMenuScreen.loadOBJ("menu.obj");
	MainMenuTexture.loadTexture("Main.jpg", true);
	GameOverMesh.loadOBJ("menu.obj");
	GameOverTexture.loadTexture("Gameover.jpg", true);
	YouWinMesh.loadOBJ("menu.obj");
	YouWinTexture.loadTexture("Youwin.jpg", true);
	treeMesh.loadOBJ("menu.obj");
	treeTexture.loadTexture("Pause.jpg", true);

	//UI
	NoBoostUI.loadOBJ("boostUI.obj");
	NoBoostTxt.loadTexture("noBoostUI.jpg", true);
	JumpUI.loadOBJ("boostUI.obj");
	JumpTxt.loadTexture("jumpUI.jpg", true);
	AttackUI.loadOBJ("boostUI.obj");
	AttackTxt.loadTexture("AttackUI.jpg", true);
	DefendUI.loadOBJ("boostUI.obj");
	DefendTxt.loadTexture("defendUI.jpg", true);

	//Powerups
	jmpPowerUpMesh.loadOBJ("Powerup.obj");
	JmpPowerUpTexture.loadTexture("jumpUV.png", true);
	atkPowerUpMesh.loadOBJ("Powerup.obj");
	AtkPowerUpTexture.loadTexture("attackUV.png", true);
	defPowerUpMesh.loadOBJ("Powerup.obj");
	DefPowerUpTexture.loadTexture("shieldUV.png", true);

	//Vehicles
	playerMesh.loadOBJ("blueCar.obj");
	blueVehicleTexture.loadTexture("blueCar.png", true, true);
	redVehicleTexture.loadTexture("redCar.png", true, true);
	aiOpponentTextures.push_back(redVehicleTexture);
	purpleVehicleTexture.loadTexture("purpleCar.png", true, true);
	aiOpponentTextures.push_back(purpleVehicleTexture);
	greenVehicleTexture.loadTexture("greenCar.png", true, true);
	aiOpponentTextures.push_back(greenVehicleTexture);

	//Level
	citySurfaceMesh.loadOBJ("cityLevel4.obj");
	cityTexture.loadTexture("cityLevel3.png", true);
	grassSurfaceMesh.loadOBJ("grassLevel4.obj");
	grassTexture.loadTexture("grasslevelUV.png", true);
	desertSurfaceMesh.loadOBJ("sandLevel4.obj");
	desertSurfaceMesh.setIsMostOuterLevel(true); //used to determine the bounding box of the entire level
	desertTexture.loadTexture("sandLevel3.png", true);

	//Objects
	cubeMesh.loadVertexData(Utils::cubeVertexData, Utils::cubeArrayLen);
	objectMeshes.push_back(cubeMesh);
	cubeTexture.loadTexture("container_texture.jpg", true);
	objectTextures.push_back(cubeTexture);

	//Other
	glm::mat4 projection = glm::perspective(glm::radians(50.0f), (float)Utils::windowWidth / (float)Utils::windowHeight, 0.1f, 500.0f); //how to show perspective (fov, aspect ratio)
	ourShader.setMat4("projection", projection); //pass the projection matrix to the fragment shader
	ourShader.setInt("shadowMap", 1);
	skybox = Skybox(skyboxShader);

	prepShadows(depthShader, projection);
}

void Renderer::prepShadows(Shader depthShader, glm::mat4 projection) {
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	depthShader.setMat4("projection", projection);
}








///render a single frame of the game
void Renderer::renderGameFrame(physx::PxMat44 pxPlayerTrans,
	physx::PxMat44 pxUITrans,
	std::vector<physx::PxMat44> pxOpponentsTrans,
	glm::vec3 pxLevelPos,
	std::vector<physx::PxTransform> pxObjectsTrans,
	Shader mainShader,
	Shader textShader,
	Shader skyboxShader,
	Shader depthShader,
	glm::mat4 view,
	glm::vec3 cameraPos,
	int carsRemoved,
	std::vector<PowerUp> powerUps,
	int gameStatus
) { //render a single frame of the game

//PREP
	frameCounter++;
	noCarsRemoved = carsRemoved;


	//RENDERING
	setDepthShader(depthShader, lightPos);
	renderScene(depthShader, pxPlayerTrans, pxUITrans, pxOpponentsTrans, pxLevelPos, pxObjectsTrans, view, cameraPos, carsRemoved, powerUps, gameStatus);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Utils::windowWidth, Utils::windowHeight); //reset viewport
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setMainShader(mainShader, cameraPos, view, gameStatus);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	renderScene(mainShader, pxPlayerTrans, pxUITrans, pxOpponentsTrans, pxLevelPos, pxObjectsTrans, view, cameraPos, carsRemoved, powerUps, gameStatus);
	skybox.renderSkybox(skyboxShader, view); //skybox
}

void Renderer::renderScene(Shader& shader,
	physx::PxMat44 pxPlayerTrans,
	physx::PxMat44 pxUITrans,
	std::vector<physx::PxMat44> pxOpponentsTrans,
	glm::vec3 pxLevelPos,
	std::vector<physx::PxTransform> pxObjectsTrans,
	glm::mat4 view,
	glm::vec3 cameraPos,
	int carsRemoved,
	std::vector<PowerUp> powerUps,
	int gameStatus) {

	glm::mat4 model = glm::mat4(1.0f); //identity matrix
	renderObject(shader, &playerMesh, &blueVehicleTexture, glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f), 180.f, vehicleScale, pxPlayerTrans);

	switch (uiBoost) { //player power-up UI
	case(0): renderObject(shader, &NoBoostUI, &NoBoostTxt, UITranslation, UIRot, UIRotDeg, UIScale, pxUITrans); break;
	case(1): renderObject(shader, &JumpUI, &JumpTxt, UITranslation, UIRot, UIRotDeg, UIScale, pxUITrans); break;
	case(2): renderObject(shader, &AttackUI, &AttackTxt, UITranslation, UIRot, UIRotDeg, UIScale, pxUITrans); break;
	case(3): renderObject(shader, &DefendUI, &DefendTxt, UITranslation, UIRot, UIRotDeg, UIScale, pxUITrans); break;
	}

	for (int i = 0; i < pxOpponentsTrans.size(); i++) {
		renderObject(shader, &playerMesh, &aiOpponentTextures[i], glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f), 180.f, vehicleScale, pxOpponentsTrans[i]); //opponents
	}
	for (int i = 0; i < pxObjectsTrans.size(); i++) renderObject(shader, &objectMeshes[0], &objectTextures[0], worldOrigin, defaultRotation, defaultRotAmountDeg, defaultScale, pxObjectsTrans[i]); //objects
	for (int i = 0; i < testLocs.size(); i++) renderObject(shader, &objectMeshes[0], &objectTextures[0], testLocs[i], defaultRotation, defaultRotAmountDeg, defaultScale); //testing

	switch (carsRemoved) { //level segments
	case 0:
		renderObject(shader, &desertSurfaceMesh, &desertTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, levelScale);
		renderObject(shader, &grassSurfaceMesh, &grassTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, levelScale);
		renderObject(shader, &citySurfaceMesh, &cityTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, levelScale);
		break;
	case 1:
		if (flashLevel) {
			int remainder = frameCounter % 60;
			if (remainder >= 0 && remainder < 30) renderObject(shader, &desertSurfaceMesh, &desertTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, levelScale);
		}
		renderObject(shader, &grassSurfaceMesh, &grassTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, levelScale);
		renderObject(shader, &citySurfaceMesh, &cityTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, levelScale);
		break;
	case 2:
		if (flashLevel) {
			int remainder = frameCounter % 60;
			if (remainder >= 0 && remainder < 30) renderObject(shader, &grassSurfaceMesh, &grassTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, levelScale);
		}
		renderObject(shader, &citySurfaceMesh, &cityTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, levelScale);
		break;
	}

	for (int i = 0; i < powerUps.size(); i++) {
		//std::cout << "rendering at " << powerUps->at(i).Location.x << " " << powerUps->at(i).Location.z << std::endl;
		switch (powerUps[i].Type) {
		case(1): renderObject(shader, &jmpPowerUpMesh, &JmpPowerUpTexture, powerUps[i].Location, defaultRotation, defaultRotAmountDeg, powerUpScale); break;
		case(2): renderObject(shader, &atkPowerUpMesh, &AtkPowerUpTexture, powerUps[i].Location, defaultRotation, defaultRotAmountDeg, powerUpScale); break;
		case(3): renderObject(shader, &defPowerUpMesh, &DefPowerUpTexture, powerUps[i].Location, defaultRotation, defaultRotAmountDeg, powerUpScale); break;
		}
	}

	renderObject(shader, &GameOverMesh, &GameOverTexture, glm::vec3(-30.0f, 10.0f + 1110.f, 10.0f), screenRotation, screenRotDeg, screenScale); //menu screens
	renderObject(shader, &YouWinMesh, &YouWinTexture, glm::vec3(-30.0f, 10.0f + 1120.f, 10.0f), screenRotation, screenRotDeg, screenScale);
	renderObject(shader, &MainMenuScreen, &MainMenuTexture, glm::vec3(-30.0f, 10.0f + 1130.f, 10.0f), screenRotation, screenRotDeg, screenScale);
	renderObject(shader, &treeMesh, &treeTexture, glm::vec3(-30.0f, 10.0f + 1140.f, 10.0f), screenRotation, screenRotDeg, screenScale);
}






















void Renderer::setDepthShader(Shader depthShader, glm::vec3 lightPos) {
	glm::mat4 lightProjection, lightView;
	lightProjection = glm::ortho(300.f, -300.f, 300.f, -300.f, near_plane, far_plane);
	lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	depthShader.use();
	depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::setMainShader(Shader mainShader, glm::vec3 cameraPos, glm::mat4 view, int gameStatus) {
	mainShader.use();
	mainShader.setVec3("lightPos", lightPos);
	mainShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
	mainShader.setVec3("viewPos", cameraPos);
	if (gameStatus != 0) mainShader.setVec3("light.ambient", 1.f, 1.f, 1.f);
	else mainShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	mainShader.setVec3("light.diffuse", 0.6f, 0.6f, 0.6f);
	mainShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	mainShader.setFloat("material.shininess", 256.0f);
	mainShader.setMat4("view", view); //set the camera view matrix in our fragment shader
	mainShader.setVec3("textColor", 0.f, 0.f, 0.f);
	mainShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
}

void Renderer::renderObject(Shader shader, Mesh* meshToRender, Texture2D* textureToApply, glm::vec3 translation, glm::vec3 rotationAxis,
	float rotationAmountDeg, glm::vec3 scale, physx::PxMat44 pxTransMat) { //render a single object for a single frame, passing in a px transformation matrix automatically overrides all other transformations
	textureToApply->bind(0);
	glm::mat4 model = glm::mat4(1.0f); //identity matrix
	model = Utils::getGlmMatFromPxMat(pxTransMat);
	model = glm::translate(model, translation);
	model = glm::rotate(model, glm::radians(rotationAmountDeg), rotationAxis); //fix model orientation
	model = glm::scale(model, scale);
	shader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)	
	meshToRender->draw();
}

std::vector<Mesh*> Renderer::getGroundMeshes(int index) { //returns pointers to all ground meshes, cannot be called before setup
	if (index == 0) {
		std::vector<Mesh*> meshes;
		meshes.push_back(&citySurfaceMesh);
		meshes.push_back(&grassSurfaceMesh);
		meshes.push_back(&desertSurfaceMesh);
		return meshes;
	}
	else if (index == 1) {
		std::vector<Mesh*> meshes;
		meshes.push_back(&citySurfaceMesh);
		meshes.push_back(&grassSurfaceMesh);
		return meshes;
	}
	else if (index == 2) {
		std::vector<Mesh*> meshes;
		meshes.push_back(&citySurfaceMesh);
		return meshes;
	}
}

std::vector<glm::vec3> Renderer::getLevelBB() { //TODO: make a bit more streamlined once the segment logic is in
	switch (noCarsRemoved) {
	case 0:
		return desertSurfaceMesh.getBoundingBoxVertices();
		break;
	case 1:
		return grassSurfaceMesh.getBoundingBoxVertices();
		break;
	case 2:
		return citySurfaceMesh.getBoundingBoxVertices();
		break;
	default:
		return desertSurfaceMesh.getBoundingBoxVertices();
		break;
	}
}

bool Renderer::isLastSegment() {
	if (noCarsRemoved == 2) return true; //we're down to the last map segment
	else return false;
}

void Renderer::flashSegment(bool keepFlashing) {
	flashLevel = keepFlashing;
}

int Renderer::getUIBoost() {
	return uiBoost;
}

void Renderer::setUIBoost(int ui) {
	uiBoost = ui;
}

void Renderer::prepText(Shader shader) {
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// compile and setup the shader
	// ----------------------------
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(Utils::windowWidth), 0.0f, static_cast<float>(Utils::windowHeight));
	shader.use();
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// FreeType
	// --------
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	}

	// find path to font
	std::string font_name = "arial.ttf";
	if (font_name.empty())
	{
		std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
	}

	// load font as face
	FT_Face face;
	if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	}
	else {
		// set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, 48);

		// disable byte-alignment restriction
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// load first 128 characters of ASCII set
		for (unsigned char c = 0; c < 128; c++)
		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			// generate texture
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			// set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// now store character for later use
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<unsigned int>(face->glyph->advance.x)
			};
			Characters.insert(std::pair<char, Character>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	// destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);


	// configure VAO/VBO for texture quads
	// -----------------------------------
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);
	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Renderer::renderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
	//glClearColor(0.f, 0.f, 0.f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// activate corresponding render state	
	shader.use();
	glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(textVAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
