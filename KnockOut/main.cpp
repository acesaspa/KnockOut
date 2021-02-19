#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "..\KnockOut\Dependencies\imgui\imgui.h"
#include "..\KnockOut\Dependencies\GLFW\include\GLFW\glfw3.h"
#include "..\KnockOut\Dependencies\imgui\imgui_impl_glfw.h"
#include "..\KnockOut\Dependencies\imgui\imgui_impl_opengl3.h"
#include <iostream>
#include <ctype.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../include/physx/PxPhysicsAPI.h"
#include "../include/physx/snippetcommon/SnippetPrint.h"
#include "../include/physx/snippetcommon/SnippetPVD.h"
#include "../include/physx/snippetutils/SnippetUtils.h"
#include "../include/physx/vehicle/PxVehicleUtil.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleSceneQuery.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleFilterShader.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleTireFriction.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleCreate.h"
#include "../include/physx/snippetcommon/SnippetPrint.h"
#include "../include/physx/snippetcommon/SnippetPVD.h"
#include "../include/physx/snippetutils/SnippetUtils.h"
#include "OpenALEngine.h"
#include "SoundManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Shader.cpp"
#include "Mesh.h"
#include "Texture2D.h"
#include "VehiclePhysx.h"

using namespace physx;
using namespace snippetvehicle;

//MARK: Function Prototypes
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

PxReal stackZ = 10.0f;
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
float yaw = 90.0f;
float pitch = 0.0f;
float lastX = 400, lastY = 300;
float cameraDistance = 5.0f;
float angleAroundTarget = 0.0f;
bool firstMouse = true;
bool mouseVisible = false;
bool vehicleReversing = false;
bool vehicleAccelerating = false;
unsigned int CUBE_VBO, GROUND_VBO, CUBE_VAO, GROUND_VAO;
unsigned int vehicle_texture, cube_texture2, ground_texture;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 15.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::vec3();
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 direction;

void keyPress(unsigned char key, const PxTransform& camera)
{
	PX_UNUSED(camera);
	PX_UNUSED(key);
}


void prepCubeRendering(Shader* ourShader) {
	glEnable(GL_DEPTH_TEST); //to make sure the fragment shader takes into account that some geometry has to be drawn in front of another
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	glGenVertexArrays(1, &CUBE_VAO);
	glGenBuffers(1, &CUBE_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, CUBE_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(CUBE_VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//TEXTURES
	glGenTextures(1, &vehicle_texture); //TEXTURE 1
	glBindTexture(GL_TEXTURE_2D, vehicle_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //set the texture wrapping parameters (= how to behave when the texture not big enough to cover the whole area)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //set texture filtering parameters (= how to decide which texel (texture pixel) to show on the current
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //screen pixel, i.e. do we just take what's directly underneath?, or add up the neighboring colors?, etc.)
	int width, height, nrChannels; //load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); //tell stb_image.h to flip loaded texture's on the y-axis
	unsigned char* data = stbi_load("container_texture.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else std::cout << "Failed to load texture" << std::endl;
	stbi_image_free(data); //free image mem
}


void prepGroundRendering(Shader* ourShader) {
	glEnable(GL_DEPTH_TEST); //to make sure the fragment shader takes into account that some geometry has to be drawn in front of another
	float vertices[] = { //vertices of our plane
		 //positions              //normals           //texture coords
		-300.0f,  0.0f,  300.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
		 300.0f,  0.0f,  300.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
		 300.0f,  0.0f, -300.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,

		-300.0f,  0.0f, -300.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
		-300.0f,  0.0f,  300.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
		 300.0f,  0.0f, -300.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
	};
	glGenVertexArrays(1, &GROUND_VAO);
	glGenBuffers(1, &GROUND_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, GROUND_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(GROUND_VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//TEXTURE
	glGenTextures(1, &ground_texture);
	glBindTexture(GL_TEXTURE_2D, ground_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //set the texture wrapping parameters (= how to behave when the texture not big enough to cover the whole area)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //set texture filtering parameters (= how to decide which texel (texture pixel) to show on the current
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //screen pixel, i.e. do we just take what's directly underneath?, or add up the neighboring colors?, etc.)
	int width, height, nrChannels; //load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); //tell stb_image.h to flip loaded texture's on the y-axis
	unsigned char* data = stbi_load("grass.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else std::cout << "Failed to load texture" << std::endl;
	stbi_image_free(data); //free image mem
}

glm::mat4 getGlmMatrix(PxMat44 tempMat) { //convert a 4x4 px matrix to a 4x4 glm matrix
	glm::mat4 model = glm::mat4(1.0f);
	model[0].x = tempMat.column0[0];
	model[0].y = tempMat.column0[1];
	model[0].z = tempMat.column0[2];
	model[0].w = tempMat.column0[3];

	model[1].x = tempMat.column1[0];
	model[1].y = tempMat.column1[1];
	model[1].z = tempMat.column1[2];
	model[1].w = tempMat.column1[3];

	model[2].x = tempMat.column2[0];
	model[2].y = tempMat.column2[1];
	model[2].z = tempMat.column2[2];
	model[2].w = tempMat.column2[3];

	model[3].x = tempMat.column3[0];
	model[3].y = tempMat.column3[1];
	model[3].z = tempMat.column3[2];
	model[3].w = tempMat.column3[3];
	return model;
}

VehiclePhysx Physics = VehiclePhysx();

//MARK: Main
int main(int argc, char** argv) {

	//MARK: INIT Sounds
	OpenALEngine wavPlayer = OpenALEngine();
	SoundManager bgm = wavPlayer.createSoundPlayer(0);
	bgm.setVolume(0.3f);
	bgm.loopSound(true);

	//MARK: INIT GLFW
	const char* glsl_version = "#version 130";
	GLFWwindow* window;
	if (!glfwInit()) return -1;
	window = glfwCreateWindow(800, 800, "Knock Out", NULL, NULL);
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


	//Model Positions
	glm::vec3 modelPos = {
		glm::vec3(1.0f, 1.0f, 1.0f) //blue car
	};

	//Model scale
	glm::vec3 modelScale = {
		glm::vec3(0.5f, 0.5f, 0.5f) //blue car
	};

	// Load meshes and textures
	const int numModels = 1;
	Mesh mesh[numModels];
	Texture2D texture;



	//MARK: INIT IMGUI
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

	//MARK: SCENE RENDER PREP
	prepCubeRendering(&ourShader);
	prepGroundRendering(&ourShader);
	ourShader.use(); //tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	ourShader.setInt("material.diffuse", 0);
	ourShader.setInt("material.specular", 1);



	mesh[0].loadOBJ("blueCar.obj");
	texture.loadTexture("blueCar_diffuse.jpg", true);


	Physics.initPhysics();

	//MARK: CAMERA SETUP
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)800, 0.1f, 500.0f); //how to show perspective (fov, aspect ratio)
	ourShader.setMat4("projection", projection); //pass the projection matrix to the fragment shader

	//MARK: RENDER LOOP ---------------------------------------------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window)) {

		if (!bgm.soundPlaying()) {
			bgm.playSound();
		}

		//MARK: Frame Start
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);
		Physics.stepPhysics();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		PxMat44 pxTransMatrix = Physics.getVehicleTrans(1);

		PxMat44 pxTransMatrix2 = Physics.getVehicleTrans(2);

		float angleAroundY = Physics.getAngleAroundY();

		glm::vec3 cubePos = Physics.getVehiclePos();

		cameraPos.y = cubePos.y + 2.f; //calculatePos
		float xOffset = cameraDistance * cos(glm::radians(angleAroundY + angleAroundTarget));
		float zOffset = cameraDistance * sin(glm::radians(angleAroundY + angleAroundTarget));
		cameraPos.x = cubePos.x - xOffset;
		cameraPos.z = cubePos.z - zOffset;

		yaw = angleAroundY + angleAroundTarget;

		cameraFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); //update
		cameraFront.y = sin(glm::radians(pitch));
		cameraFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(cameraFront);
		cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
		cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

		glm::vec3 groundPos = Physics.getGroundPos();

		glm::vec3 boxPos = Physics.getBoxPos(1);

		glm::vec3 boxPos2 = Physics.getBoxPos(2);

		glm::vec3 boxPos3 = Physics.getBoxPos(3);

		glm::mat4 view = glm::mat4(1.0f); //transformations - first initialize the identity matrix
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); //apply a special built in matrix specifically made for camera views call the "Look At" matrix
		//ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader

		//MARK: Render Scene
		glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//DIRECTIONAL LIGHTING (sunglight)
		ourShader.use();
		ourShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
		ourShader.setVec3("viewPos", cameraPos);
		ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		ourShader.setVec3("light.diffuse", 0.6f, 0.6f, 0.6f);
		ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		ourShader.setFloat("material.shininess", 256.0f);

		//VEHICLE
		texture.bind(0);

		glm::mat4 model = glm::mat4(1.0f); //identity matrix
		model = getGlmMatrix(pxTransMatrix);
		//model = glm::translate(model, cubePos); //model matrix converts the local coordinates (cubePosition) to the global world coordinates
		model = glm::rotate(model, glm::radians(180.f), glm::vec3(0.0f, 1.0f, 0.0f)); //rotate
		model = glm::scale(model, modelScale);
		ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)
		model[3][1] = model[3][1] - 3.0f;
		ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader
		mesh[0].draw();
		//glDrawArrays(GL_TRIANGLES, 0, 36); //draw the triangle data, starting at 0 with 36 vertex data points

		//VEHICLE2

		texture.bind(0);

		glm::mat4 model2 = glm::mat4(1.0f); //identity matrix
		model2 = getGlmMatrix(pxTransMatrix2);
		//model = glm::translate(model, cubePos); //model matrix converts the local coordinates (cubePosition) to the global world coordinates
		//model = glm::rotate(model, glm::radians(0), glm::vec3(0.0f, 1.0f, 0.0f)); //rotate
		//model = glm::scale(model, glm::vec3(pxBounds.getDimensions().x, pxBounds.getDimensions().y, pxBounds.getDimensions().z));
		model2 = glm::scale(model2, modelScale);
		ourShader.setMat4("model", model2); //set the model matrix (which when applied converts the local position to global world coordinates...)
		model2[3][1] = model2[3][1] - 3.0f;
		ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader
		mesh[0].draw();
		//glDrawArrays(GL_TRIANGLES, 0, 36); //draw the triangle data, starting at 0 with 36 vertex data points


		//GROUND
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ground_texture);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, ground_texture);
		glBindVertexArray(GROUND_VAO);
		model = glm::mat4(1.0f);
		model = glm::translate(model, groundPos);
		//model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f));
		ourShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//BOX
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glBindVertexArray(CUBE_VAO); //tell OpenGL to render whatever we have in our Vertex Array Object
		model = glm::mat4(1.0f); //identity matrix
		model = getGlmMatrix(PxMat44(Physics.getBoxTrans(1)));
		ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)
		glDrawArrays(GL_TRIANGLES, 0, 36); //draw the triangle data, starting at 0 with 36 vertex data points

		glActiveTexture(GL_TEXTURE0); //bind textures on corresponding texture units
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glBindVertexArray(CUBE_VAO); //tell OpenGL to render whatever we have in our Vertex Array Object
		model = glm::mat4(1.0f); //identity matrix
		model = getGlmMatrix(PxMat44(Physics.getBoxTrans(2)));
		ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)
		glDrawArrays(GL_TRIANGLES, 0, 36); //draw the triangle data, starting at 0 with 36 vertex data points

		glActiveTexture(GL_TEXTURE0); //bind textures on corresponding texture units
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glBindVertexArray(CUBE_VAO); //tell OpenGL to render whatever we have in our Vertex Array Object
		model = glm::mat4(1.0f); //identity matrix
		model = getGlmMatrix(PxMat44(Physics.getBoxTrans(3)));
		ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)
		glDrawArrays(GL_TRIANGLES, 0, 36); //draw the triangle data, starting at 0 with 36 vertex data points

		//MARK: Render ImgUI
		{
			ImGui::Begin("Debug Menu");
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
		if (mouseVisible) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			mouseVisible = false;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			mouseVisible = true;
		}
	}

	Physics.releaseAllControls();

	float cameraSpeed = 10 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	if ((glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_UP) == GLFW_REPEAT)) {
		//std::cout << "UP1\n";
		Physics.setGMimicKeyInputs(true);
		Physics.forceGearChange(PxVehicleGearsData::eFIRST);
		Physics.startAccelerateForwardsMode();
	}
	if ((glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_REPEAT)) {
		//std::cout << "DOWN1\n";
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
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}