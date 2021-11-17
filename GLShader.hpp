#ifndef GLSHADER_H
#define GLSHADER_H

#include "glad/glad.h"
#include <string>

GLuint LoadShader(const char* vertexPath, const char* fragmentPath);
std::string readFile(const char *filePath);

#endif