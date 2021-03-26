#include "Renderer.h"
#include <iostream>
#include <ctype.h>
#include "Utils.h"
#include <map>
#include <stb_image.h>
#include <list>
#include "PowerUp.h"

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

//text
std::map<GLchar, Character> Characters;
unsigned int textVAO, textVBO;

//byskox
unsigned int cubemapTexture;
unsigned int skyboxVAO, skyboxVBO;





unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

int Renderer::getUIBoost() {
	return uiBoost;
}

void Renderer::setUIBoost(int ui) {
	uiBoost = ui;
}

void Renderer::prepSkybox(Shader shader) {
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// skybox VAO
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	std::vector<std::string> faces
	{
		"right2.jpg",
		"left2.jpg",
		"top2.jpg",
		"bottom2.jpg",
		"front2.jpg",
		"back2.jpg"
	};
	cubemapTexture = loadCubemap(faces);

	shader.use();
	shader.setInt("skybox", 0);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)1200 / (float)800, 0.1f, 500.0f);
	shader.setMat4("projection", projection);
}






void Renderer::renderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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






void Renderer::prepText(Shader shader) {
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// compile and setup the shader
	// ----------------------------
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(1200), 0.0f, static_cast<float>(800));
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



void Renderer::setUpRendering(glm::vec3 cameraPos, Shader ourShader, Shader textShader) { //call once before entering the game loop
	glEnable(GL_DEPTH_TEST); //to make sure the fragment shader takes into account that some geometry has to be drawn in front of another
	ourShader.use(); //tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	ourShader.setInt("material.diffuse", 0);
	ourShader.setInt("material.specular", 1);

	//MARK: Object Setup
	//Screens
	MainMenuScreen.loadOBJ("menu.obj");
	MainMenuTexture.loadTexture("MainMenuScreen.jpg", true);
	GameOverMesh.loadOBJ("menu.obj");
	GameOverTexture.loadTexture("GameOverScreen.jpg", true);
	YouWinMesh.loadOBJ("menu.obj");
	YouWinTexture.loadTexture("YouWinScreen.jpg", true);

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

	//Player car
	playerMesh.loadOBJ("blueCar.obj");
	playerTexture.loadTexture("blueCar.png", true, true);

	citySurfaceMesh.loadOBJ("cityLevel2.obj");
	cityTexture.loadTexture("asphalt.jpg", true);

	grassSurfaceMesh.loadOBJ("test.obj");
	grassTexture.loadTexture("grass.jpg", true);

	desertSurfaceMesh.loadOBJ("sandLevel2.obj");
	desertSurfaceMesh.setIsMostOuterLevel(true); //used to determine the bounding box of the entire level
	desertTexture.loadTexture("desert_texture.jpg", true);

	//std::cout << "Desert verts: " << citySurfaceMesh.getActualVertices().size() << std::endl;
	//std::cout << "Desert inds: " <<  citySurfaceMesh.getVertexIndices().size() << std::endl;

	treeMesh.loadOBJ("normalTree.obj");
	treeTexture.loadTexture("normalTreeTexture.png", true);

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
	physx::PxMat44 pxUITrans,
	std::vector<physx::PxMat44> pxOpponentsTrans,
	glm::vec3 pxLevelPos,
	std::vector<physx::PxTransform> pxObjectsTrans,
	Shader ourShader,
	Shader textShader,
	Shader skyboxShader,
	glm::mat4 view,
	glm::vec3 cameraPos,
	int status,
	std::list<PowerUp*>& powerups
	){ //render a single frame of the game

	applyShaderValues(ourShader, cameraPos, view);

	//PLAYER
	glm::mat4 model = glm::mat4(1.0f); //identity matrix
	renderObject(ourShader, &playerMesh, &playerTexture, glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f), 180.f, vehicleScale, pxPlayerTrans);

	//no boost
	if (uiBoost == 0) {
		renderObject(ourShader, &NoBoostUI, &NoBoostTxt, UITranslation, UIRot, UIRotDeg, UIScale, pxUITrans);
	}
	//jump boost
	else if (uiBoost == 1) {
		renderObject(ourShader, &JumpUI, &JumpTxt, UITranslation, UIRot, UIRotDeg, UIScale, pxUITrans);
	}
	//attack boost
	else if (uiBoost == 2) {
		renderObject(ourShader, &AttackUI, &AttackTxt, UITranslation, UIRot, UIRotDeg, UIScale, pxUITrans);
	}
	//defense boost
	else if (uiBoost == 3) {
		renderObject(ourShader, &DefendUI, &DefendTxt, UITranslation, UIRot, UIRotDeg, UIScale, pxUITrans);
	}


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
	
	for (std::list<PowerUp*>::const_iterator it = powerups.begin(); it != powerups.end(); it++) {
		//std::cout << (*it)->getLocation().x << "\n";
		if ((*it)->isCollected==false) {
			switch ((*it)->Type) {
				case(1):
					renderObject(ourShader, &jmpPowerUpMesh, &JmpPowerUpTexture, (*it)->Location, defaultRotation, defaultRotAmountDeg, powerUpScale);
					break;
				case(2):
					renderObject(ourShader, &atkPowerUpMesh, &AtkPowerUpTexture, (*it)->Location, defaultRotation, defaultRotAmountDeg, powerUpScale);
					break;
				case(3):
					renderObject(ourShader, &defPowerUpMesh, &DefPowerUpTexture, (*it)->Location, defaultRotation, defaultRotAmountDeg, powerUpScale);
			}
		}
	}
	
	//if(jump) renderObject(ourShader, &jmpPowerUpMesh, &JmpPowerUpTexture, glm::vec3(20.0f, 1.0f, 10.0f), defaultRotation, defaultRotAmountDeg, powerUpScale);
	//if(attack) renderObject(ourShader, &atkPowerUpMesh, &AtkPowerUpTexture, glm::vec3(20.0f, 1.0f, 10.0f), defaultRotation, defaultRotAmountDeg, powerUpScale);
	//if(defense) renderObject(ourShader, &defPowerUpMesh, &DefPowerUpTexture, glm::vec3(20.0f, 1.0f, 10.0f), defaultRotation, defaultRotAmountDeg, powerUpScale);


	//Game Over
	renderObject(ourShader, &GameOverMesh, &GameOverTexture, glm::vec3(-30.0f, 10.0f + 1110.f, 10.0f), screenRotation, screenRotDeg, screenScale);

	//You Win
	renderObject(ourShader, &YouWinMesh, &YouWinTexture, glm::vec3(-30.0f, 10.0f + 1120.f, 10.0f), screenRotation, screenRotDeg, screenScale);

	//Main menu
	renderObject(ourShader, &MainMenuScreen, &MainMenuTexture, glm::vec3(-30.0f, 10.0f + 1130.f, 10.0f), screenRotation, screenRotDeg, screenScale);


	renderObject(ourShader, &treeMesh, &treeTexture, worldOrigin, defaultRotation, defaultRotAmountDeg, defaultScale);

	//Byskox
	renderSkyBox(skyboxShader, view);


	//TESTING
	//for(int i = 0; i < desertSurfaceMesh.getBoundingBoxVertices().size(); i++)
	//	renderObject(ourShader, &objectMeshes[0], &objectTextures[0], desertSurfaceMesh.getBoundingBoxVertices()[i], defaultRotation, defaultRotAmountDeg, defaultScale);
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

void Renderer::renderSkyBox(Shader skyboxShader, glm::mat4 view) {
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	skyboxShader.use();
	skyboxShader.setMat4("view", view);

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default
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




std::vector<glm::vec3> Renderer::getBB() { //TODO: make a bit more streamlined once the segment logic is in
	return desertSurfaceMesh.getBoundingBoxVertices();
}