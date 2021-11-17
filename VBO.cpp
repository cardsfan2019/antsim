#include "VBO.hpp"
#include "glad/glad.h"
	
VBO::VBO () {
    glGenBuffers(1, &ID);
}

void VBO::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::init(const void* data, unsigned int size, unsigned int usage) {
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

VBO::~VBO() {
	glDeleteBuffers(1, &ID);
}
