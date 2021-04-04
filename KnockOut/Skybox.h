#ifndef SKYBOX_H
#define SKYBOX_H

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.cpp"

class Skybox {
public:
	Skybox() = default;
	Skybox(Shader skyboxShader);
	void renderSkybox(Shader skyboxShader, glm::mat4 view);
private:
	void prepSkybox(Shader shader);
	unsigned int loadCubemap(std::vector<std::string> faces);
};

#endif