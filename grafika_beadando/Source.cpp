#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cmath>

// Konstansok a feladat leírása alapján
const int WIN_W = 600;
const int WIN_H = 600;
const float RADIUS = 50.0f;
const float SPEED_LEN = 10.0f; // Extra 3: 10 pixel hosszúságú irányvektor

float circleX = 300.0f;
float circleY = 300.0f;
float lineY = 300.0f;
float dx = 4.0f; // Kezdeti vízszintes sebesség
float dy = 0.0f;
bool diagonalStarted = false;
bool keys[1024];

std::string readShaderFile(const char* filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return "";
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

GLuint loadShaders(const char* vPath, const char* fPath) {
    std::string vCode = readShaderFile(vPath);
    std::string fCode = readShaderFile(fPath);
    const char* vPtr = vCode.c_str();
    const char* fPtr = fCode.c_str();
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vPtr, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fPtr, NULL);
    glCompileShader(fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs); glAttachShader(prog, fs);
    glLinkProgram(prog);
    return prog;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS && !diagonalStarted) {
        diagonalStarted = true;
        float angle = 25.0f * 3.14159f / 180.0f;
        dx = cos(angle) * SPEED_LEN; // Pontosan 10 pixel/frame sebesség
        dy = sin(angle) * SPEED_LEN;
    }
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    GLFWwindow* window = glfwCreateWindow(WIN_W, WIN_H, "Beadando 1 - Grafika", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();
    glfwSetKeyCallback(window, keyCallback);

    float vertices[] = { -1,-1, 1,-1, -1,1, 1,-1, 1,1, -1,1 };
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint programID = loadShaders("vertexShader.glsl", "fragmentShader.glsl");
    glUseProgram(programID);

    while (!glfwWindowShouldClose(window)) {
        circleX += dx;
        circleY += dy;

        // Visszapattanás pontosan a széleken (r=50-nél)
        if (circleX <= RADIUS || circleX >= (WIN_W - RADIUS)) dx *= -1.0f;

        if (diagonalStarted) {
            if (circleY <= RADIUS || circleY >= (WIN_H - RADIUS)) dy *= -1.0f;
        }
        else {
            circleY = 300.0f; // Alapértelmezett vízszintes mozgás
        }

        if (keys[GLFW_KEY_UP]) lineY += 5.0f;
        if (keys[GLFW_KEY_DOWN]) lineY -= 5.0f;
        lineY = std::max(5.0f, std::min(595.0f, lineY));

        // Metszés vizsgálata (Extra 2)
        float closestX = std::max(200.0f, std::min(circleX, 400.0f));
        float distSq = pow(circleX - closestX, 2) + pow(circleY - lineY, 2);
        bool hit = (distSq <= pow(RADIUS, 2));

        glClear(GL_COLOR_BUFFER_BIT);
        glUniform2f(glGetUniformLocation(programID, "u_resolution"), (float)WIN_W, (float)WIN_H);
        glUniform2f(glGetUniformLocation(programID, "u_circleCenter"), circleX, circleY);
        glUniform1f(glGetUniformLocation(programID, "u_lineY"), lineY);

        // Színbeállítás a metszés alapján
        if (!hit) {
            glUniform3f(glGetUniformLocation(programID, "u_innerColor"), 1.0f, 0.0f, 0.0f); // Piros közép
            glUniform3f(glGetUniformLocation(programID, "u_outerColor"), 0.0f, 1.0f, 0.0f); // Zöld szél
        }
        else {
            glUniform3f(glGetUniformLocation(programID, "u_innerColor"), 0.0f, 1.0f, 0.0f); // Zöld közép
            glUniform3f(glGetUniformLocation(programID, "u_outerColor"), 1.0f, 0.0f, 0.0f); // Piros szél
        }

        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
} 