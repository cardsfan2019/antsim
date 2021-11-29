#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "GLShader.hpp"
#include "Camera.hpp"
#include "Texture2D.hpp"
#include "VAO.hpp"
#include "VBO.hpp"
#include "EBO.hpp"
#include "Compute.hpp"
#include "time.h"

#include <iostream>
#include <string>

#define TWO_PI 6.283

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// settings
const unsigned int SCR_WIDTH = 2560;
const unsigned int SCR_HEIGHT = 1440;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

bool debug = false;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float zoomSpeed = 0.1f;

struct Agent {
	float posX;
    float posY;
	float angle;
};

unsigned int hash(unsigned int state) {
	state ^= 2747636419u;
	state *= 2654435769u;
	state ^= state >> 16;
	state *= 2654435769u;
	state ^= state >> 16;
	state *= 2654435769u;
	return state;
}

int main()
{
    srand(time(NULL));
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Ant Simulation", glfwGetPrimaryMonitor(), NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback); 
    glfwSetCursorPosCallback(window, mouse_callback);  

    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

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

    Compute moveShader("move_shader.compute");
    Compute diffuseShader("diffuse_shader.compute");

    const int tex_w = SCR_WIDTH, tex_h = SCR_HEIGHT;

    Agent* agents = new Agent[1000000];

    for (int i = 0; i < 1000000; i++) {
        Agent a;

        float x = 0.0f;
        float y = 0.0f;

        while(true) {
            x = tex_w * rand() / (float)RAND_MAX;
            y = tex_h * rand() / (float)RAND_MAX;

            if ((x-tex_w/2)*(x-tex_w/2) + (y-tex_h/2)*(y-tex_h/2) < 600*600)
                break;
        }

        a.posX = x;
        a.posY = y;

        a.angle = atan2f(tex_h/2-y, tex_w/2-x);

        agents[i] = a;
    }

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Agent) * 1000000, agents, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);

    GLuint tex_output;
    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    float vertices[] = {
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  // top right
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom left
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f   // top left 
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
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

    shader.use();
    shader.setInt("texture1", 0);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;        
        
        glfwPollEvents();
        //glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //dispatch compute shader for updating the positions of the ants
        moveShader.use();
        glDispatchCompute((GLuint)1000000, 1, 1);

        //dispatch the compute shader for diffusing the color of the window (subtracting some from brightness);
        diffuseShader.use();
        glDispatchCompute((GLuint)tex_w, tex_h, 1);

        //wait for compute shaders to finish
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        //draw quad with texture from compute
        shader.use();
        glClear(GL_COLOR_BUFFER_BIT);
        quad_vao.bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_output);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glfwSwapBuffers(window);
    }
    delete [] agents;
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
    // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    lastX = xpos;
    lastY = ypos; 
}