#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "..\KnockOut\Dependencies\imgui\imgui.h"
#include "..\KnockOut\Dependencies\GLFW\include\GLFW\glfw3.h"
#include "..\KnockOut\Dependencies\imgui\imgui_impl_glfw.h"
#include "..\KnockOut\Dependencies\imgui\imgui_impl_opengl3.h"
#include <iostream>
#include "physx\PxPhysicsAPI.h"

extern int snippetMain(int, const char* const*);
void initializePhysics();
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main(int argc, char** argv)
{
    // Uncomment to run snippet code
    //return snippetMain(argc, argv);

    initializePhysics();

    const char* glsl_version = "#version 130";

    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(1280, 720, "Knock Out", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup Keyboard Input
    glfwSetKeyCallback(window, KeyCallback);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render here
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
        glVertex2f(-0.5f, -0.5f);
        glVertex2f(0.0f, 0.5f);
        glVertex2f(0.5f, -0.5f);
        glEnd();

        // Show a simple window
        {

            // Create a window
            ImGui::Begin("Debug Menu");

            // Display FPS
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return 0;
}

void initializePhysics() 
{
    physx::PxPhysics* gPhysics = NULL;
    std::cout << "PhysX initialized" << std::endl;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //press 1 to display part 1 - Squares and Diamonds
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        std::cout << "1" << std::endl;
    }
}

