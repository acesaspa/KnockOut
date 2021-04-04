#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Windows.h>
#include <..\KnockOut\Dependencies\imgui\imgui.h>
#include <..\KnockOut\Dependencies\GLFW\include\GLFW\glfw3.h>
#include <..\KnockOut\Dependencies\imgui\imgui_impl_glfw.h>
#include <..\KnockOut\Dependencies\imgui\imgui_impl_opengl3.h>
#include <iostream>
#include <ctype.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <../include/physx/PxPhysicsAPI.h>
#include <../include/physx/snippetcommon/SnippetPrint.h>
#include <../include/physx/snippetcommon/SnippetPVD.h>
#include <../include/physx/snippetutils/SnippetUtils.h>
#include <../include/physx/vehicle/PxVehicleUtil.h>
#include <../include/physx/snippetvehiclecommon/SnippetVehicleSceneQuery.h>
#include <../include/physx/snippetvehiclecommon/SnippetVehicleFilterShader.h>
#include <../include/physx/snippetvehiclecommon/SnippetVehicleTireFriction.h>
#include <../include/physx/snippetvehiclecommon/SnippetVehicleCreate.h>
#include <../include/physx/snippetcommon/SnippetPrint.h>
#include <../include/physx/snippetcommon/SnippetPVD.h>
#include <../include/physx/snippetutils/SnippetUtils.h>

#include "OpenALEngine.h"
#include "SoundManager.h"
#include "Shader.cpp"
#include "Mesh.h"
#include "Renderer.h"
#include "VehiclePhysx.h"
#include "Utils.h"
#include "Camera.h"
#include "AIBehavior.h"

#include <stdlib.h>
#include <cstdlib> 
#include <chrono>
#include <ctime> 
#include <list>

#include "PowerUp.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Source.h"

using namespace physx;
using namespace snippetvehicle;

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void keyPress(unsigned char key, const PxTransform& camera);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void addPowerUp();
void removeSegment();

PxReal stackZ = 10.0f;
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
bool vehicleReversing = false;
bool vehicleAccelerating = false;
unsigned int CUBE_VBO, GROUND_VBO, CUBE_VAO, GROUND_VAO;
unsigned int vehicle_texture, cube_texture2, ground_texture;
bool reset = false;
std::list<PowerUp*> powerups;
bool removingSegment = false;
int numPow = 0;

auto start = std::chrono::system_clock::now();
auto segmentRemovalStart = std::chrono::system_clock::now();

auto poweruptimestart = std::chrono::system_clock::now();

Renderer mainRenderer;
Camera mainCamera;
VehiclePhysx Physics = VehiclePhysx();
Source source;

AIBehavior ai1 = AIBehavior(1);
AIBehavior ai2 = AIBehavior(1);
AIBehavior ai3 = AIBehavior(1);


/*
0 = PLAY
1 = GAME OVER / YOU WIN SCREEN
2 = MAIN MENU SCREEN
*/
int st = 0;

OpenALEngine wavPlayer = OpenALEngine();
float baseVolume = 1.0f;
SoundManager activate = wavPlayer.createSoundPlayer(7);
SoundManager soundSelector = wavPlayer.createSoundPlayer(2);

//MARK: Main
int main(int argc, char** argv) {

	//MARK: Init Sounds
	SoundManager bgm = wavPlayer.createSoundPlayer(0);
	bgm.setVolume(baseVolume * 0.2);
	bgm.loopSound(true);

	SoundManager crash = wavPlayer.createSoundPlayer(1);
	crash.setVolume(baseVolume * 0.8);
	crash.loopSound(false);

	SoundManager select = wavPlayer.createSoundPlayer(2);
	select.setVolume(baseVolume * 0.8);
	select.loopSound(false);

	SoundManager victory = wavPlayer.createSoundPlayer(3);
	victory.setVolume(baseVolume * 0.3);
	victory.loopSound(false);

	SoundManager gameover = wavPlayer.createSoundPlayer(4);
	gameover.setVolume(baseVolume * 0.3);
	gameover.loopSound(false);

	SoundManager pickup = wavPlayer.createSoundPlayer(5);
	pickup.setVolume(baseVolume * 0.5);
	pickup.loopSound(false);

	SoundManager invalid = wavPlayer.createSoundPlayer(6);
	invalid.setVolume(baseVolume * 0.5);
	invalid.loopSound(false);

	SoundManager activate = wavPlayer.createSoundPlayer(7);
	activate.setVolume(baseVolume * 0.5);
	activate.loopSound(false);

	SoundManager reving = wavPlayer.createSoundPlayer(8);
	reving.setVolume(baseVolume * 0.8);
	reving.loopSound(true);

	SoundManager engine = wavPlayer.createSoundPlayer(9);
	engine.setVolume(baseVolume * 0.3);
	engine.loopSound(true);


	//MARK: Init Glfw
	const char* glsl_version = "#version 130";
	GLFWwindow* window;
	if (!glfwInit()) return -1;
	window = glfwCreateWindow(1200, 800, "Knock Out", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) std::cout << "Error!" << std::endl;
	std::cout << glGetString(GL_VERSION) << std::endl;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	glEnable(GL_DEPTH_TEST);
	Shader mainShader("main_vertex_shader.vs", "main_fragment_shader.fs");
	Shader textShader("text.vs", "text.fs");
	Shader skyboxShader("skybox.vs", "skybox.fs");
	Shader depthShader("depth_vertex_shader.vs", "depth_fragment_shader.fs");

	//MARK: Init Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	//TODO
	mainRenderer.setUpRendering(mainCamera.getCameraPos(), mainShader, textShader, skyboxShader, depthShader);
	Physics.initPhysics(mainRenderer.getGroundMeshes(0));	
	ai1.updateLevelBB(mainRenderer.getLevelBB());
	ai2.updateLevelBB(mainRenderer.getLevelBB());
	ai3.updateLevelBB(mainRenderer.getLevelBB());

	glfwSetKeyCallback(window, key_callback);
	Physics.setGameStatus(0);



	//MARK: RENDER LOOP ---------------------------------------------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window)) {
		addPowerUp();

		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			Physics.setGameStatus(0);
			Physics.reset();
		}

		bool hasPower = false;
		for (std::list<PowerUp*>::const_iterator it = powerups.begin(); it != powerups.end(); it++) {
			if ((*it)->isCollected) {
				if ((*it)->Player == 1) {
					hasPower = true;
					mainRenderer.setUIBoost((*it)->Type);
				}
			}
		}
		if (!hasPower) {
			mainRenderer.setUIBoost(0);
		}



		//MARK: Game Status & Segments
		if (Physics.getGameStatus() == 0) {
			Physics.checkGameOver();
			Physics.updateNumCars();
			if (Physics.getChanged()) {
				segmentRemovalStart = std::chrono::system_clock::now();
				removingSegment = true;
				mainRenderer.flashSegment(true);
				Physics.setChanged(false);
				ai1.updateLevelBB(mainRenderer.getLevelBB());
				ai2.updateLevelBB(mainRenderer.getLevelBB());
				ai3.updateLevelBB(mainRenderer.getLevelBB());
				ai1.startEvac();
				ai2.startEvac();
				ai3.startEvac();
			}
		}
		if (removingSegment) removeSegment();



		//MARK: POWER-UP PICK UP
		for (std::list<PowerUp*>::const_iterator it = powerups.begin(); it != powerups.end(); it++){ //Loop through powerups
			
			//Loop through the cars
			for (int i = 1; i < 3; i++) {

				bool hasPowerUp = false;
				for (std::list<PowerUp*>::const_iterator it2 = powerups.begin(); it2 != powerups.end(); it2++) {
					if ((*it2)->Player == i) {
						hasPowerUp = true;
					}
				}

				//Only pick up a powerup if you don't already have one
				if (!hasPowerUp) {
					//If the car is at the position of a powerup
					if (glm::length(Physics.getVehiclePos(i) - (*it)->Location) < 2.f) {

						//if the powerup isn't collected
						if (!(*it)->isCollected) {

							//pick up the powerup
							(*it)->isCollected = true;
							(*it)->Player = i;
							//sound here
							poweruptimestart = std::chrono::system_clock::now();
							break;
						}
					}
				}
			}
		}


		//MARK: Frame Start
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		auto poweruptimeend = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = poweruptimeend - poweruptimestart;
		if (elapsed_seconds.count() < 0.65)
		{
			if (!pickup.soundPlaying()) { pickup.playSound(); }
		}
		else
		{
			pickup.stopSound();
		}

		if (!bgm.soundPlaying()) { bgm.playSound(); }
		if (!engine.soundPlaying()) { engine.playSound(); }

		if ((glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_UP) == GLFW_REPEAT))
		{
			if (!reving.soundPlaying()) { reving.playSound(); }
		}
		else if ((glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_REPEAT))
		{
			if (!reving.soundPlaying()) { reving.playSound(); }
		}
		else
		{
			reving.stopSound();
		}

		if (Physics.getGameStatus() == 3)
		{
			bgm.stopSound();
			engine.stopSound();
			reving.stopSound();
			if (!gameover.soundPlaying()) { gameover.playSound(); }
		}

		if (Physics.getGameStatus() == 4)
		{
			bgm.stopSound();
			engine.stopSound();
			reving.stopSound();
			if (!victory.soundPlaying()) { victory.playSound(); }
		}

		processInput(window);
		Physics.stepPhysics();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (Physics.getGameStatus() == 0) {
			mainCamera.updateCamera(Physics.getAngleAroundY(), Physics.getVehiclePos(1));
		}
		else if (Physics.getGameStatus() == 3) {

			//give camera the position of the game over screen
			mainCamera.updateCamera(0.f, glm::vec3(-26.0f, 6.0f + 1110.f, 10.0f));

			//Press 1 to go to main menu
			if (st == 3) {
				Physics.setGameStatus(-1);
			}
			//Press 2 to exit
			else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
				glfwSetWindowShouldClose(window, true);
			}
		}
		else if (Physics.getGameStatus() == 4) {
			//give camera the position of the you win screen
			mainCamera.updateCamera(0.f, glm::vec3(-26.0f, 6.0f + 1120.f, 10.0f));

			//Press 1 to go to main menu
			if (st == 3) {
				Physics.setGameStatus(-1);
			}
			//Press 2 to exit
			else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
				glfwSetWindowShouldClose(window, true);
			}
		}
		else if (Physics.getGameStatus() == -1) {
			//give camera the position of the main menu screen
			mainCamera.updateCamera(0.f, glm::vec3(-26.0f, 6.0f + 1130.f, 10.0f));
			//Press 1 to play
			if (st == 0) {
				reset = true;
				Physics.reset();
				printf("reset\n");
				Physics.setGameStatus(0);
			}
			//Press 2 to exit
			else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
				glfwSetWindowShouldClose(window, true);
			}
			st = 3;
		}
		glm::mat4 view = mainCamera.getViewMatrix();



		//MARK: Render Scene
		glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		std::vector<PxTransform> pxObjects; //ideally this "arrayization" should be done in PhysX
		pxObjects.push_back(Physics.getBoxTrans(1));
		pxObjects.push_back(Physics.getBoxTrans(2));
		pxObjects.push_back(Physics.getBoxTrans(3));
		std::vector<PxMat44> pxOpponents;
		pxOpponents.push_back(Physics.getVehicleTrans(2));
		pxOpponents.push_back(Physics.getVehicleTrans(3));
		pxOpponents.push_back(Physics.getVehicleTrans(4));


		//ai1.frameUpdate(Physics.getVehDat(1), Physics.getOpponentPos(1), Physics.getOpponentForVec(1), Physics.getVehiclePos(1), Physics.getPlayerForVec(), Physics.getVehicle4W(1),
		//	Physics.getVehicle4W(0));
		//ai2.frameUpdate(Physics.getVehDat(2), Physics.getOpponentPos(2), Physics.getOpponentForVec(2), Physics.getVehiclePos(2), Physics.getPlayerForVec(), Physics.getVehicle4W(2),
		//	Physics.getVehicle4W(0));
		//ai3.frameUpdate(Physics.getVehDat(3), Physics.getOpponentPos(3), Physics.getOpponentForVec(3), Physics.getVehiclePos(3), Physics.getPlayerForVec(), Physics.getVehicle4W(3),
		//	Physics.getVehicle4W(0));


		if (Physics.getGameStatus() == 1) {
			//Camera will go to game over screen
			st = 1;
			Physics.setGameStatus(3);
		}
		//You win
		else if (Physics.getGameStatus() == 2) {
			//std::cout << "you win\n";
			//Camera will go to you win screen
			st = 2;
			Physics.setGameStatus(4);
		}
		else {
			mainRenderer.renderGameFrame(Physics.getVehicleTrans(1), Physics.getVehicleTrans(1), pxOpponents, Physics.getGroundPos(), pxObjects, mainShader, textShader, skyboxShader, depthShader, view, mainCamera.getCameraPos(), Physics.getNumCars(), powerups, Physics.getGameStatus());
		}
		/*
		if (Physics.getGameStatus() == 1) {
			mainRenderer.renderText(textShader, "GAME OVER", 300.f, 400.0f, 2.0f, glm::vec3(190 / 255.f, 0.f, 0.f));
		}
		else if (Physics.getGameStatus() == 2) {
			mainRenderer.renderText(textShader, "YOU WIN", 390.f, 400.0f, 2.0f, glm::vec3(57 / 255.f, 1.f, 20 / 255.f));
		}
		else {
			mainRenderer.renderGameFrame(Physics.getVehicleTrans(1), pxOpponents, Physics.getGroundPos(), pxObjects, ourShader, textShader, skyboxShader, view, mainCamera.getCameraPos(), Physics.getGameStatus(), powerups);
		}
		*/

		//MARK: Render Imgui
		{
			ImGui::Begin("Debug Menu");

			ImGui::Text("Vehicle Position");
			ImGui::Text("x: %.1f    y: %.1f    z: %.1f", Physics.getVehiclePos(1).x, Physics.getVehiclePos(1).y, Physics.getVehiclePos(1).z);
			ImGui::Text("-----------------------------------------------");

			ImGui::Text("");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



		//MARK: Frame End
		glfwSwapInterval(1);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//---------------------------------------------------------------------------------------------------------------------------------

	//MARK: Clean up & Terminate
	Physics.cleanupPhysics();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}





void removeSegment() {
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - segmentRemovalStart;

	if (elapsed_seconds.count() >= 10) { //current time - last time = elapsed point
		powerups.clear();
		Physics.removeGround(mainRenderer.getGroundMeshes(Physics.getNumCars()));
		removingSegment = false;
		mainRenderer.flashSegment(false);
		ai1.setAttacking(); //after first segment removal all AIs will attack
		ai2.setAttacking();
		ai3.setAttacking();
	}
}


//MARK: Input Functions
void processInput(GLFWwindow* window) {

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
		if (mainCamera.getMouseVisible()) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			mainCamera.setMouseVisible(false);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			mainCamera.setMouseVisible(true);
		}
	}

	Physics.releaseAllControls();

	float cameraSpeed = 10 * deltaTime;

	if (Utils::getFreeCamMode()) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) mainCamera.getCameraPos() += cameraSpeed * mainCamera.getCameraFront();
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) mainCamera.getCameraPos() -= cameraSpeed * mainCamera.getCameraFront();
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) mainCamera.getCameraPos() -= glm::normalize(glm::cross(mainCamera.getCameraFront(), mainCamera.getCameraUp())) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) mainCamera.getCameraPos() += glm::normalize(glm::cross(mainCamera.getCameraFront(), mainCamera.getCameraUp())) * cameraSpeed;
	}
	else {
		if ((glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_UP) == GLFW_REPEAT)) {
			Physics.setGMimicKeyInputs(true);
			Physics.forceGearChange(PxVehicleGearsData::eFIRST);
			Physics.startAccelerateForwardsMode();
		}
		if ((glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_REPEAT)) {
			Physics.setGMimicKeyInputs(true);
			Physics.forceGearChange(PxVehicleGearsData::eREVERSE);
			Physics.startAccelerateReverseMode();
		}
		if ((glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_REPEAT)) {
			Physics.setGMimicKeyInputs(true);
			Physics.startBrakeMode();
		}
		if ((glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_REPEAT)) {
			Physics.setGMimicKeyInputs(true);
			Physics.startTurnHardRightMode();
		}
		if ((glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_REPEAT)) {
			Physics.setGMimicKeyInputs(true);
			Physics.startTurnHardLeftMode();
		}
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		activate.setVolume(baseVolume * 0.2);
		activate.loopSound(false);
		for (std::list<PowerUp*>::const_iterator it = powerups.begin(); it != powerups.end(); it++) {
			if ((*it)->isCollected) {
				switch ((*it)->Type) {
					case(1):
						Physics.applyForce(PxVec3(0.f, 700000.f, 0.f), 1);
						break;
					case(2): {
						glm::mat4 rotation = glm::rotate(glm::mat4{ 1.f }, float(-M_PI / 2.f), glm::vec3(0, 1, 0));
						PxVec3 pre = (Physics.getRotation() + PxVec3(0.f, 0.05f, 0.f));
						glm::vec4 rot = glm::vec4(pre.x, pre.y, pre.z, 0.f);
						glm::vec4 rotated = rotation * rot;
						Physics.applyForce(1000000.f * PxVec3(rotated.x, rotated.y, rotated.z), 1);
						}
						break;
					case(3): {
						glm::vec3 vehiclePos = Physics.getVehiclePos(1);
						glm::vec3 enemyPos = Physics.getVehiclePos(2);
						glm::vec3 direction = enemyPos - vehiclePos;
						if (glm::length(direction) < 10) {
							Physics.stopVehicle(2);
						}
					}
				}
				if (!activate.soundPlaying()) { activate.playSound(); }
				powerups.remove(*it);
				break;
			}
		}
		
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	soundSelector.setVolume(baseVolume * 0.5);
	soundSelector.loopSound(false);

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		if (st == 1) {
			printf("GAME OVER\n");
			//set game status to MENU
			st = 3;
			soundSelector.playSound();
		}
		else if (st == 2) {
			printf("YOU WIN\n");
			//set game status to MENU
			st = 3;
			soundSelector.playSound();
		}
		else if (st == 3) {
			printf("MENU\n");
			//set game status to PLAY
			st = 0;
			soundSelector.playSound();
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	mainCamera.mouseCallback(window, xpos, ypos);
}

void keyPress(unsigned char key, const PxTransform& camera)
{
	PX_UNUSED(camera);
	PX_UNUSED(key);
}

void addPowerUp() {
	if (powerups.size() < 5) {
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;

		if (elapsed_seconds.count() >= 5) { //current time - last time = elapsed point
			start = std::chrono::system_clock::now();

			srand(time(NULL));

			int powerChoice = rand() % 3 + 1;
			float x = rand() % (100 + 100 + 1) - 100;
			float z = rand() % (100 + 100 + 1) - 100;
			float y = 5.f;

			std::vector<Mesh*> groundMeshes = mainRenderer.getGroundMeshes(Physics.getNumCars());

			float minDistance = 1000.f;
			glm::vec3 height = glm::vec3(0, 0, 0);

			for (int i = 0; i < groundMeshes.size(); i++) {
				Mesh* meshToCook = groundMeshes[i];

				std::vector<PxVec3> vertices = meshToCook->getActualVertices();
				std::vector<PxU32> indices = meshToCook->getVertexIndices();

				for (int j = 0; j < vertices.size(); j++) {
					glm::vec3 point = glm::vec3(vertices[j].x, vertices[j].y, vertices[j].z);
					glm::vec3 power = glm::vec3(x, y, z);

					glm::vec3 dif = point - power;
					float distance = glm::length(dif);

					if (distance < minDistance) {
						minDistance = distance;
						height.x = point.x;
						height.y = point.y;
						height.z = point.z;
					}
					//std::cout << vertices[j].x << " " << vertices[j].y << " " << vertices[j].z << "\n";
				}
			}
			//std::cout << height.x << " " << height.y << " " << height.z << "\n";
			PowerUp* test3 = new PowerUp(height + glm::vec3(0.f, 1.f, 0.f), powerChoice);
			powerups.push_back(test3);
		}
	}
}
