#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h> // A kamera (gluLookAt) és gömb (gluSphere) miatt szükséges
#include <cmath>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

// Ablak méretei
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// --- Kamera és világítás paraméterek ---
float r = 8.0f;           // Kamera távolsága (8 és 10 között)
float cameraAngle = 0.0f; // z-tengely körüli forgás (radiánban)
float cameraZ = 0.0f;     // z-tengely menti magasság

float lightAngle = 0.0f;  // Fényforrás mozgásának szöge
bool lightEnabled = true; // Világítás állapota
GLuint sunTexture;        // Textúra azonosító

// Manuális kockarajzoló függvény (mivel a GLFW-ben nincs glutSolidCube)
void drawCube(float size) {
    float v = size / 2.0f;
    glBegin(GL_QUADS);
    // Z-Pozitív lap (Felső lap, mivel az UP vektor (0,0,1))
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-v, -v, v); glVertex3f(v, -v, v); glVertex3f(v, v, v); glVertex3f(-v, v, v);
    // Z-Negatív lap (Alsó lap)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-v, -v, -v); glVertex3f(-v, v, -v); glVertex3f(v, v, -v); glVertex3f(v, -v, -v);
    // X-Pozitív lap
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(v, -v, -v); glVertex3f(v, v, -v); glVertex3f(v, v, v); glVertex3f(v, -v, v);
    // X-Negatív lap
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-v, -v, -v); glVertex3f(-v, -v, v); glVertex3f(-v, v, v); glVertex3f(-v, v, -v);
    // Y-Pozitív lap
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-v, v, -v); glVertex3f(-v, v, v); glVertex3f(v, v, v); glVertex3f(v, v, -v);
    // Y-Negatív lap
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-v, -v, -v); glVertex3f(v, -v, -v); glVertex3f(v, -v, v); glVertex3f(-v, -v, v);
    glEnd();
}

// Egyszerű 2x2-es textúra generálása külső képfájl nélkül
void createSunTexture() {
    glGenTextures(1, &sunTexture);
    glBindTexture(GL_TEXTURE_2D, sunTexture);

    // Textúra beállításai (ismétlődés és szűrés)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Kép betöltése az stb_image segítségével
    int width, height, nrChannels;

    // Y-tengely megfordítása, mert az OpenGL fordítva kezeli a képek Y koordinátáját
    stbi_set_flip_vertically_on_load(true);

    // Kép betöltése (fontos, hogy a sun.jpg ott legyen a Source.cpp mellett)
    unsigned char* data = stbi_load("sun.jpg", &width, &height, &nrChannels, 0);

    if (data) {
        // Ha a képnek van alpha csatornája (pl. PNG), akkor GL_RGBA kell, JPG esetén GL_RGB
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "HIBA: Nem sikerult betolteni a sun.jpg texturat! Ellenorizd a mappat." << std::endl;
    }

    // A kép adatai már a videókártyán vannak, a memóriából törölhetjük
    stbi_image_free(data);
}

// Inicializálás
void init() {
    glEnable(GL_DEPTH_TEST); // Z-buffer bekapcsolása (Tipp a feladatban)

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    // Fényforrás színe (tetszőleges, fehértől eltérő diffúz: sárgás)
    GLfloat lightDiffuse[] = { 1.0f, 0.8f, 0.2f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Sötétszürke háttér

    createSunTexture();
}

// Billentyűzet eseménykezelője
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        float moveSpeed = 0.1f;
        switch (key) {
        case GLFW_KEY_LEFT:
            cameraAngle -= moveSpeed;
            break;
        case GLFW_KEY_RIGHT:
            cameraAngle += moveSpeed;
            break;
        case GLFW_KEY_UP:
            cameraZ += moveSpeed;
            break;
        case GLFW_KEY_DOWN:
            cameraZ -= moveSpeed;
            break;
        case GLFW_KEY_L:
            if (action == GLFW_PRESS) lightEnabled = !lightEnabled;
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
    }
}

// Képernyő újra-rajzolása
void display(GLFWwindow* window) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (height == 0) height = 1;

    glViewport(0, 0, width, height);

    // Vetítés beállítása: perspective 55 fok
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)width / (float)height;
    gluPerspective(55.0, aspect, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Kamera beállítása
    float camX = r * cos(cameraAngle);
    float camY = r * sin(cameraAngle);

    // gluLookAt(SzemX, SzemY, SzemZ, CélX, CélY, CélZ, FelX, FelY, FelZ)
    gluLookAt(camX, camY, cameraZ,
        0.0, 0.0, 0.0,
        0.0, 0.0, 1.0); // UP vektor (0,0,1)

    // Világítás ki/be kapcsolása
    if (lightEnabled) glEnable(GL_LIGHTING);
    else glDisable(GL_LIGHTING);

    // 2. Fényforrás pozicionálása és kirajzolása
    glPushMatrix();
    float lightRadius = 2.0f * r; // Sugár: 2*r
    float lX = lightRadius * cos(lightAngle);
    float lY = lightRadius * sin(lightAngle);

    GLfloat lightPos[] = { lX, lY, 0.0f, 1.0f }; // Z = 0
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // Nap gömbje
    glTranslatef(lX, lY, 0.0f);

    GLfloat sunEmission[] = { 1.0f, 0.8f, 0.2f, 1.0f };
    GLfloat noEmission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, sunEmission);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, sunTexture);

    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluSphere(quad, 0.25, 32, 32); // Átmérő = 0.5 (tehát sugár = 0.25)
    gluDeleteQuadric(quad);

    glDisable(GL_TEXTURE_2D);
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
    glPopMatrix();

    // 3. Kockák kirajzolása (3 db fehér egységkocka)
    GLfloat cubeMaterial[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cubeMaterial);

    // Középső kocka (origóban)
    glPushMatrix();
    drawCube(1.0f);
    glPopMatrix();

    // Felső kocka (+2 Z magasságban)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 2.0f);
    drawCube(1.0f);
    glPopMatrix();

    // Alsó kocka (-2 Z magasságban)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -2.0f);
    drawCube(1.0f);
    glPopMatrix();
}

int main() {
    // --- Programleiras a terminalba (ekezetek nelkul) ---
    std::cout << "Iranyitas:" << std::endl;
    std::cout << "  * BAL/JOBB nyilak: Kamera korbe mozgatasa (r=8 hengerfeluleten)" << std::endl;
    std::cout << "  * FEL/LE nyilak: Kamera fel-le mozgatasa a Z-tengely menten" << std::endl;
    std::cout << "  * 'L' billentyu: Vilagitas (es beepitett texturazott Nap) Ki/Be" << std::endl;
    std::cout << "  * ESC: Kilepes" << std::endl;
    // ---------------------------------------------------

    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Kockak es Kamera (GLFW ver.)", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    // Visszahívó függvény beállítása a billentyűzethez
    glfwSetKeyCallback(window, key_callback);

    // OpenGl inicializálás
    init();

    // Fő ciklus
    while (!glfwWindowShouldClose(window)) {

        // Fényforrás animálása minden iterációban
        lightAngle += 0.001f;
        if (lightAngle > 2 * M_PI) {
            lightAngle -= 2 * M_PI;
        }

        display(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}