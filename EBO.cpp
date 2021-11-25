#include "EBO.hpp"  
#include <glad/gl.h>
	
EBO::EBO() {
	glGenBuffers(1, &ID);
}

void EBO::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void EBO::init(const void* data, unsigned int size, unsigned int usage) {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}

EBO::~EBO() {
	glDeleteBuffers(1, &ID);
}