
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "..\KnockOut\Dependencies\imgui\imgui.h"
#include "..\KnockOut\Dependencies\GLFW\include\GLFW\glfw3.h"
#include "..\KnockOut\Dependencies\imgui\imgui_impl_glfw.h"
#include "..\KnockOut\Dependencies\imgui\imgui_impl_opengl3.h"
#include <iostream>
#include "physx\PxPhysicsAPI.h"
#include <ctype.h>

#include "../include/physx/PxPhysicsAPI.h"
#include "../include/physx/snippetcommon/SnippetPrint.h"
#include "../include/physx/snippetcommon/SnippetPVD.h"
#include "../include/physx/snippetutils/SnippetUtils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Shader.cpp"

using namespace physx;


//MARK: Function Prototypes
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);


//MARK: Var
PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;
PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;
PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;
PxCooking* gCooking = NULL;
PxMaterial* gMaterial = NULL;
PxPvd* gPvd = NULL;
PxReal stackZ = 10.0f;
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400, lastY = 300;
bool firstMouse = true;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 direction;




//MARK: PhysX Functions
void stepPhysics(bool /*interactive*/){
    gScene->simulate(1.0f / 60.0f);
    gScene->fetchResults(true);
}

void cleanupPhysics(bool /*interactive*/){
    PX_RELEASE(gScene);
    PX_RELEASE(gDispatcher);
    PX_RELEASE(gPhysics);
    PX_RELEASE(gCooking);
    if (gPvd)
    {
        PxPvdTransport* transport = gPvd->getTransport();
        gPvd->release();	gPvd = NULL;
        PX_RELEASE(transport);
    }
    PX_RELEASE(gFoundation);
}












//MARK: Main
int main(int argc, char** argv){

    //MARK: INIT PHYSX & PVD
    static const PxU32 frameCount = 100;
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));
    gScene = gPhysics->createScene(sceneDesc);
    PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
    if (pvdClient) {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }



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
    glEnable(GL_DEPTH_TEST); //to make sure the fragment shader takes into account that some geometry has to be drawn in front of another
    float vertices[] = { //vertices of our cube
       //positions          //texture coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    unsigned int indices[] = { //for how to form a square face from 2 triangles
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    glm::vec3 cubePosition = glm::vec3(0.0f, 0.0f, 0.0f);
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO); //Vertex Array Object holds the VBO and the EBO (i.e. all the data and extra info GPU will need)
                                //also holds the attribute configuration (Eg. telling the GPU we care about position & textures)
    glGenBuffers(1, &VBO); //Vertex Buffer Object contains the vertex data we send to the GPU/vertex shader (vertex_shader.vs)
    glGenBuffers(1, &EBO); //Element Buffer Object tells the GPU how we want to draw our stuff, Eg. for a cube 1 vertex
                           //is listed 4 times in the vertex array, EBO (using the index array) will tell the GPU
                           //to draw each vertex only once

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); //position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); //texture coord attribute
    glEnableVertexAttribArray(1);












    //TODO: only 1 triangle from the box currently shows in PVD ---------------------------------------------------------------------------

    //MARK: INIT PHYSX OBJECTS
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f); //create some material
    PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial); //create a physics plane
    gScene->addActor(*groundPlane); //add it to our physics scene

    PxTriangleMeshDesc meshDesc; //mesh cooking from a triangle mesh
    float verts[] = { 
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };
    unsigned int inds[] = { //for how to form a square face from 2 triangles
        0, 1, 2, // first triangle
        3, 4, 5,
        6, 7, 8,
        9, 10, 11,
        12, 13, 14,
        15, 16, 17,
        18, 19, 20,
        21, 22, 23,
        24, 25, 26,
        27, 28, 29,
        30, 31, 32,
        33, 34, 35
    };
    //std::cout << sizeof(verts) << "\n";
    //std::cout << sizeof(PxVec3) << "\n";

    meshDesc.points.count = 108; 
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = verts;

    meshDesc.triangles.count = 108/9;
    meshDesc.triangles.stride = 3 * sizeof(PxU32);
    meshDesc.triangles.data = inds;

    //PxCookingParams params = gCooking->getParams();
    ////TODO: potentially do this
    //gCooking->setParams(params);

    PxTriangleMesh* triMesh = NULL;
    PxU32 meshSize = 0;
    triMesh = gCooking->createTriangleMesh(meshDesc, gPhysics->getPhysicsInsertionCallback()); //insert the cooked mesh directly into PxPhysics
    PxRigidStatic* boxBody = gPhysics->createRigidStatic(PxTransform(PxVec3(0, 0, 0))); //create a rigid body for the cooked mesh
    PxShape* boxShape = gPhysics->createShape(PxTriangleMeshGeometry(triMesh), *gMaterial); //create a shape from the cooked mesh
    boxBody->attachShape(*boxShape); //attach the shape to the body
    gScene->addActor(*boxBody); //and add it to the scene
    triMesh->release(); //clean up

    //TODO: only 1 triangle from the box currently shows in PVD ---------------------------------------------------------------------------















    //MARK: TEXTURE PREP
    unsigned int texture1, texture2;
    glGenTextures(1, &texture1); //TEXTURE 1
    glBindTexture(GL_TEXTURE_2D, texture1);
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
    }else std::cout << "Failed to load texture" << std::endl;
    stbi_image_free(data); //free image mem

    glGenTextures(1, &texture2); //TEXTURE 2
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    data = stbi_load("face_texture.png", &width, &height, &nrChannels, 0);
    if (data){
        //note that the face_texture.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else std::cout << "Failed to load texture" << std::endl;
    stbi_image_free(data);

    ourShader.use(); //tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);



    //MARK: CAMERA SETUP
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)800, 0.1f, 100.0f); //how to show perspective (fov, aspect ratio)
    ourShader.setMat4("projection", projection); //pass the projection matrix to the fragment shader




    //MARK: RENDER LOOP ---------------------------------------------------------------------------------------------------------------
    while (!glfwWindowShouldClose(window)){

        //MARK: Frame Start
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);
        stepPhysics(false);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        //MARK: Render Scene
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0); //bind textures on corresponding texture units
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        ourShader.use(); //activate our shader program (containing our vertex_shader.vs & fragment_shader.fs)

        glm::mat4 view = glm::mat4(1.0f); //transformations - first initialize the identity matrix
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); //apply a special built in matrix specifically made for camera views call the "Look At" matrix
        ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader

        glBindVertexArray(VAO); //tell OpenGL to render whatever we have in our Vertex Array Object
        glm::mat4 model = glm::mat4(1.0f); //identity matrix
        model = glm::translate(model, cubePosition); //model matrix converts the local coordinates (cubePosition) to the global world coordinates
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f)); //rotate
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
    cleanupPhysics(false);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}




//MARK: Input Functions
void processInput(GLFWwindow* window){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    if (firstMouse){
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





