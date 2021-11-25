#include "VAO.hpp"  
#include <glad/gl.h>
#include <iostream>
	
VAO::VAO() {
    glGenVertexArrays(1, &ID);
    //std::cout << glGetError() << std::endl;
}

void VAO::bind() {
    glBindVertexArray(ID);
    // std::cout << glGetError() << std::endl;
}

void VAO::unbind() {
    glBindVertexArray(0);
}

void VAO::set(unsigned int index, unsigned int size, unsigned int type, unsigned int stride, const void *pointer) {
    glEnableVertexAttribArray(index);
    //std::cout << glGetError() << std::endl;
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, pointer);
    //std::cout << glGetError() << std::endl;
}
	
VAO::~VAO() {
	glDeleteVertexArrays(1, &ID);
}