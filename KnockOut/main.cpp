#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
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
#include <chrono>
#include <ctime> 

#define _USE_MATH_DEFINES
#include <math.h>

using namespace physx;
using namespace snippetvehicle;

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void keyPress(unsigned char key, const PxTransform& camera);

PxReal stackZ = 10.0f;
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
bool vehicleReversing = false;
bool vehicleAccelerating = false;
unsigned int CUBE_VBO, GROUND_VBO, CUBE_VAO, GROUND_VAO;
unsigned int vehicle_texture, cube_texture2, ground_texture;

bool reset = false;
bool jump = false;
bool attack = false;
bool defense = false;
int powerup = 3;

auto start = std::chrono::system_clock::now();

Renderer mainRenderer;
Camera mainCamera;
VehiclePhysx Physics = VehiclePhysx();

AIBehavior beh;

/* 
0 = PLAY
1 = GAME OVER / YOU WIN SCREEN
2 = MAIN MENU SCREEN
*/
int st = 0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1 && action == GLFW_PRESS && st == 0) {
		printf("PLAY\n");
		//set game status to PLAY
	}
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS && st == 1) {
		printf("GAME OVER\n");
		//set game status to MENU
		st = 3;

	}
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS && st == 2) {
		printf("YOU WIN\n");
		//set game status to MENU
		st = 3;
	}
	else if (key == GLFW_KEY_1 && action == GLFW_PRESS && st == 3) {
		printf("MENU\n");
		//set game status to PLAY
		st = 0;
	}
}


void addPowerUp() {

	// Some computation here
	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> elapsed_seconds = end - start;

	//std::cout << elapsed_seconds.count() << "\n";


	if (elapsed_seconds.count()>=10) { //current time - last time = elapsed point
		int powerChoice = rand() % 3 + 1;

		switch (powerChoice) {
		case(1):
			if (!attack && !defense) {
				jump = true;
			}
			break;
		case(2):
			if (!jump && !defense) {
				attack = true;
			}
			break;
		case(3):
			if (!jump && !attack) {
				defense = true;
			}
			break;
		}
	}
}

//MARK: Main
int main(int argc, char** argv) {

	//MARK: Init Sounds
	OpenALEngine wavPlayer = OpenALEngine();
	SoundManager bgm = wavPlayer.createSoundPlayer(0);
	bgm.setVolume(0.00f);
	bgm.loopSound(true);

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
	glfwSetKeyCallback(window, key_callback);

	Shader ourShader("vertex_shader.vs", "fragment_shader.fs");
	Shader textShader("text.vs", "text.fs");
	Shader skyboxShader("skybox.vs", "skybox.fs");

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
	mainRenderer.setUpRendering(mainCamera.getCameraPos(), ourShader, textShader);
	mainRenderer.prepText(textShader);
	mainRenderer.prepSkybox(skyboxShader);
	Physics.initPhysics(mainRenderer.getGroundMeshes(1));

	//MARK: RENDER LOOP ---------------------------------------------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window)) {

		addPowerUp();

		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			Physics.setGameStatus(0);
			Physics.reset();
		}

		if (powerup == 1) {
			mainRenderer.setUIBoost(1);
		}
		else if (powerup == 2) {
			mainRenderer.setUIBoost(2);
		}
		else if (powerup == 3) {
			mainRenderer.setUIBoost(3);
		}
		else {
			mainRenderer.setUIBoost(0);
		}

		//MARK: GAME OVER CHECK
		if (Physics.getGameStatus() == 0) {
			Physics.checkGameOver();
		}
		if (Physics.getGameStatus() != 0) {
			if (!reset) {
				reset = true;
				Physics.reset();
			}
		}
		if (glm::length(Physics.getVehiclePos(1)-glm::vec3(-30.0f, 1.0f, 10.0f)) < 2.f && reset) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
		if (glm::length(Physics.getVehiclePos(1) - glm::vec3(20.0f, 1.0f, 10.0f)) < 2.f && jump) {
			std::cout << "jump\n";
			start = std::chrono::system_clock::now();
			jump = false;
			powerup = 1; 
		}
		if (glm::length(Physics.getVehiclePos(1) - glm::vec3(20.0f, 1.0f, 10.0f)) < 2.f && attack) {
			std::cout << "attack\n";
			start = std::chrono::system_clock::now();
			attack = false;
			powerup = 2;
		}
		if (glm::length(Physics.getVehiclePos(1) - glm::vec3(20.0f, 1.0f, 10.0f)) < 2.f && defense) {
			std::cout << "defense\n";
			start = std::chrono::system_clock::now();
			defense = false;
			powerup = 3;
		}
		if (!bgm.soundPlaying()) {bgm.playSound();}

		//MARK: Frame Start
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);
		if (Physics.getGameStatus() == 0) {
			Physics.stepPhysics();
		}
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
			if (st == 0){
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

		//TODO: skybox
		
		//TODO: shadows
		//TODO: objects
		//TODO: another car
		//TODO: different behavior for the car
		//TODO: green segment logic
		//TODO: power-up indicator


		beh.frameUpdate(Physics.getVehDat(), Physics.getOpponentPos(), Physics.getOpponentForVec(), Physics.getVehiclePos(1), Physics.getPlayerForVec(),Physics.getOpponent4W());

		
		//Game over
		if (Physics.getGameStatus() == 1) {
			//Camera will go to game over screen
			st = 1;
			Physics.setGameStatus(3);
		}
		//You win
		else if (Physics.getGameStatus() == 2) {
			//Camera will go to you win screen
			st = 2;
			Physics.setGameStatus(4);
		}
		else {
			mainRenderer.renderGameFrame(Physics.getVehicleTrans(1), Physics.getVehicleTrans(1), pxOpponents, Physics.getGroundPos(), pxObjects, ourShader, textShader, skyboxShader, view, mainCamera.getCameraPos(), Physics.getGameStatus(), jump, attack, defense);
		}


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
	//if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) mainCamera.getCameraPos() += cameraSpeed * mainCamera.cameraFront;
	//if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) mainCamera.getCameraPos() -= cameraSpeed * mainCamera.cameraFront;
	//if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) mainCamera.getCameraPos() -= glm::normalize(glm::cross(mainCamera.getCameraFront(), mainCamera.getCameraUp())) * cameraSpeed;
	//if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) mainCamera.getCameraPos() += glm::normalize(glm::cross(mainCamera.getCameraFront(), mainCamera.getCameraUp())) * cameraSpeed;

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
		//std::cout << "DOWN1\n";
		Physics.setGMimicKeyInputs(true);
		Physics.startBrakeMode();
	}
	if ((glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_REPEAT)) {
		//std::cout << "LEFT1\n";
		Physics.setGMimicKeyInputs(true);
		Physics.startTurnHardRightMode();
	}
	if ((glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_REPEAT)) {
		//std::cout << "RIGHT1\n";
		Physics.setGMimicKeyInputs(true);
		Physics.startTurnHardLeftMode();
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (powerup == 1) {
			std::cout << "UP FORCE\n";
			
			mainRenderer.setUIBoost(0);
			
			Physics.applyForce(PxVec3(0.f, 700000.f, 0.f), 1);
			powerup = 0;
		}
		if (powerup == 2) {
			std::cout << "FRONT FORCE\n";
			
			mainRenderer.setUIBoost(0);
			
			glm::mat4 rotation = glm::rotate(glm::mat4{ 1.f }, float(-M_PI/2.f), glm::vec3(0,1,0));
			PxVec3 pre = (Physics.getRotation() + PxVec3(0.f, 0.05f, 0.f));
			glm::vec4 rot = glm::vec4(pre.x, pre.y, pre.z, 0.f);
			glm::vec4 rotated = rotation * rot;
			Physics.applyForce(1000000.f*PxVec3(rotated.x,rotated.y,rotated.z), 1);
			powerup = 0;
		}
		if (powerup == 3) {
			std::cout << "SHIELD FORCE\n";

			mainRenderer.setUIBoost(0);
			
			glm::vec3 vehiclePos = Physics.getVehiclePos(1);
			glm::vec3 enemyPos = Physics.getVehiclePos(2);

			glm::vec3 direction = enemyPos - vehiclePos;

			if (glm::length(direction) < 10) {

				Physics.stopVehicle(2);
			}

			powerup = 0;
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

