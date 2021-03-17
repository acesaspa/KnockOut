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

PxReal stackZ = 10.0f;
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
bool vehicleReversing = false;
bool vehicleAccelerating = false;
unsigned int CUBE_VBO, GROUND_VBO, CUBE_VAO, GROUND_VAO;
unsigned int vehicle_texture, cube_texture2, ground_texture;

bool reset = false;
//bool jump = false;
//bool attack = false;
//bool defense = false;
//int powerup = 3;
std::list<PowerUp*> powerups;


int numPow = 0;

auto start = std::chrono::system_clock::now();

Renderer mainRenderer;
Camera mainCamera;
VehiclePhysx Physics = VehiclePhysx();
Source source;


AIBehavior beh;

/*
class PowerUp {
public:
	glm::vec3 location;
	int Type;
	bool collected;
	int Player;

	PowerUp() {
		location.x = 0.f;
		location.y = 0.f;
		location.z = 0.f;

		Type = 0;

		collected = false;
		Player = 0;
	}

	PowerUp(glm::vec3 loc, int type) {
		location.x = loc.x;
		location.y = loc.y;
		location.z = loc.z;

		Type = type;

		collected = false;
		Player = 0;
	}

	glm::vec3 getLocation() {
		return location;
	}

	void setType(int input) {
		Type = input;
	}

	int getType() {
		return Type;
	}

	void setCollected(bool input) {
		collected = input;
	}

	bool isCollected() {
		return collected;
	}

	void setPlayer(int input) {
		Player = input;
	}

	int getPlayer() {
		return Player;
	}
};*/


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

			std::cout << x << " " << y << " " << z << "\n\n";

			std::vector<Mesh*> groundMeshes = mainRenderer.getGroundMeshes(1);

			float minDistance = 1000.f;
			glm::vec3 height = glm::vec3(0, 0, 0);

			for (int i = 0; i < groundMeshes.size(); i++) {
				Mesh* meshToCook = groundMeshes[i];

				std::vector<PxVec3> vertices = meshToCook->getActualVertices();
				std::vector<PxU32> indices = meshToCook->getVertexIndices();

				std::cout << "size " << vertices.size() << "\n";


				for (int j = 0; j < vertices.size(); j++) {
					glm::vec3 point = glm::vec3(vertices[j].x,vertices[j].y,vertices[j].z);
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

			std::cout << height.x << " " << height.y << " " << height.z << "\n";

			PowerUp *test3 = new PowerUp(height+glm::vec3(0.f,1.f,0.f),powerChoice);
			powerups.push_back(test3);
		}

	}
	
}


//MARK: Main
int main(int argc, char** argv) {

	//MARK: Init Sounds
	OpenALEngine wavPlayer = OpenALEngine();
	SoundManager bgm = wavPlayer.createSoundPlayer(0);
	bgm.setVolume(0.0f);
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

		//MARK: GAME OVER CHECK
		Physics.checkGameOver();
		if (Physics.getGameStatus() != 0) {

			if (!reset) {
				reset = true;
				Physics.reset();
				Physics.removeGround(mainRenderer.getGroundMeshes(2));
			}
		}
		if (glm::length(Physics.getVehiclePos(1)-glm::vec3(-30.0f, 1.0f, 10.0f)) < 2.f && reset) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		//PowerUp Pick Up

		//Loop through powerups
		for (std::list<PowerUp*>::const_iterator it = powerups.begin(); it != powerups.end(); it++){
			//std::cout << "test\n";
			
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

							break;
						}
					}
				}
			}
			
		}
		

		if (!bgm.soundPlaying()) {bgm.playSound();}

		//MARK: Frame Start
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);
		Physics.stepPhysics();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		mainCamera.updateCamera(Physics.getAngleAroundY(), Physics.getVehiclePos(1));
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


		//beh.frameUpdate(Physics.getVehDat(), Physics.getOpponentPos(), Physics.getOpponentForVec(), Physics.getVehiclePos(1), Physics.getPlayerForVec(),Physics.getOpponent4W());

		
		if (Physics.getGameStatus() == 1) {
			mainRenderer.renderText(textShader, "GAME OVER", 300.f, 400.0f, 2.0f, glm::vec3(190 / 255.f, 0.f, 0.f));
		}
		else if (Physics.getGameStatus() == 2) {
			mainRenderer.renderText(textShader, "YOU WIN", 390.f, 400.0f, 2.0f, glm::vec3(57 / 255.f, 1.f, 20 / 255.f));
		}
		else {
			mainRenderer.renderGameFrame(Physics.getVehicleTrans(1), pxOpponents, Physics.getGroundPos(), pxObjects, ourShader, textShader, skyboxShader, view, mainCamera.getCameraPos(), Physics.getGameStatus(), powerups);
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
				powerups.remove(*it);
				break;
			}
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

