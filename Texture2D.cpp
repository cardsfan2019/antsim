#include "Texture2D.hpp"
#include "stb_image.h"
#include <glad/gl.h>
#include <iostream>

Texture2D::Texture2D(const char *imagePath)
{
    unsigned char *data = stbi_load(imagePath, &width, &height, &nrChannels, 0);

    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (data)
    {
        glGenTextures(1, &ID);

        glBindTexture(GL_TEXTURE_2D, ID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

}

void Texture2D::bind() {
    glBindTexture(GL_TEXTURE_2D, ID);
    //std::cout << glGetError() << std::endl;
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &ID);
}