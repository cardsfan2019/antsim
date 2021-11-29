#include <glad/gl.h>
#include "GLShader.hpp"
#include <glm/glm.hpp>
#include "Compute.hpp"

Compute::Compute(const char* computePath) {
    this->ID = LoadCompute(computePath);
    
}

void Compute::use() {
    glUseProgram(ID);
    //std::cout << glGetError() << std::endl;
}

Compute::~Compute() {
    glDeleteProgram(this->ID);
}