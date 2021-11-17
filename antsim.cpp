#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "GLShader.hpp"
#include "Camera.hpp"
#include "Model.hpp"
#include "Texture2D.hpp"
#include "VAO.hpp"
#include "VBO.hpp"
#include "EBO.hpp"

#include <iostream>
#include <string>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

bool debug = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Ant Simulation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    int work_grp_cnt[3];
    work_grp_cnt[0] = 0;
    work_grp_cnt[1] = 0;
    work_grp_cnt[2] = 0;

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

    printf("max global (total) work group counts x:%i y:%i z:%i\n",
    work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

{
    Shader shader("shader.vs", "shader.fs");
    shader.use();

    // Compile fragment shader
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

    // Read shaders

    std::string compShaderStr = readFile("shader.compute");
    const char *computeShaderSrc = compShaderStr.c_str();

    GLint result = GL_FALSE;
    int logLength;
    std::cout << "Compiling fragment shader." << std::endl;
    glShaderSource(computeShader, 1, &computeShaderSrc, NULL);
    glCompileShader(computeShader);

    // Check fragment shader

    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(computeShader, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> computeShaderError((logLength > 1) ? logLength : 1);
    glGetShaderInfoLog(computeShader, logLength, NULL, &computeShaderError[0]);
    std::cout << &computeShaderError[0] << std::endl;

    std::cout << "Linking program" << std::endl;
    GLuint computeShaderProgram = glCreateProgram();
    glAttachShader(computeShaderProgram, computeShader);
    glLinkProgram(computeShaderProgram);

    glGetProgramiv(computeShaderProgram, GL_LINK_STATUS, &result);
    glGetProgramiv(computeShaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> programError( (logLength > 1) ? logLength : 1 );
    glGetProgramInfoLog(computeShaderProgram, logLength, NULL, &programError[0]);
    std::cout << &programError[0] << std::endl;

    glDeleteShader(computeShader);
    const GLchar* shader_source = "shader.compute";

    //Texture2D texture("container.jpg");

    const int tex_w = 512, tex_h = 512;
    GLuint tex_output;
    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    // glUseProgram(computeShaderProgram);
    // glBindTexture(GL_TEXTURE_2D, tex_output);
    // float data[tex_h * tex_w * 4];
    // glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);

    // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);

    float vertices[] = {
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,  // top right
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    VAO quad_vao;
    VBO quad_vbo;
    EBO quad_ebo;

    quad_vao.bind();
    quad_vbo.bind();
    quad_ebo.bind();

    quad_vbo.init(vertices, sizeof(vertices), GL_STATIC_DRAW);
    quad_ebo.init(indices, sizeof(indices), GL_STATIC_DRAW);
    quad_vao.set(0, 3, GL_FLOAT, 5 * sizeof(float), (void*)0);
    quad_vao.set(1, 2, GL_FLOAT, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    //texture.bind();
    shader.use();
    shader.setInt("texture1", 0);

    //glActiveTexture(GL_TEXTURE0);
    //glBindImageTexture(0, texture.ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    //texture.bind();

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;        
        
        glfwPollEvents();
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // shader.use();
        // glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        // glm::mat4 view = camera.GetViewMatrix();
        // shader.setMat4("projection", projection);
        // shader.setMat4("view", view);

        glUseProgram(computeShaderProgram);
        //std::cout << glGetError() << std::endl;
        glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);
        //std::cout << glGetError() << std::endl;
        // make sure writing to image has finished before read
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        //std::cout << glGetError() << std::endl;
        shader.use();
        { // normal drawing pass
            glClear(GL_COLOR_BUFFER_BIT);
            quad_vao.bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex_output);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
        
        glfwSwapBuffers(window);
    }
}

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //     camera.ProcessKeyboard(FORWARD, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //     camera.ProcessKeyboard(BACKWARD, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //     camera.ProcessKeyboard(LEFT, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //     camera.ProcessKeyboard(RIGHT, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    //     camera.ProcessKeyboard(UP, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    //     camera.ProcessKeyboard(DOWN, deltaTime);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        debug = !debug;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
