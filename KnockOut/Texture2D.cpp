#include "Texture2D.h"
#include "stb_image.h"
#include <iostream>

Texture2D::Texture2D()
	: mTexture(0)
{
}

Texture2D::~Texture2D()
{

}

bool Texture2D::loadTexture(const string& filename, bool generateMipMaps, bool flipVertically)
{
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, components;
	stbi_set_flip_vertically_on_load(flipVertically);
	unsigned char* imageData = stbi_load(filename.c_str(), &width, &height, &components, STBI_rgb_alpha);
	if (imageData == NULL)
	{
		std::cerr << "Error loading texture '" << filename << "'" << std::endl;
		return false;
	}
	//change second param to save performance
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(imageData);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}
void Texture2D::bind(GLuint texUnit)
{
	glBindTexture(GL_TEXTURE_2D, mTexture);
}

void Texture2D::unbind(GLuint texUnit)
{
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
}