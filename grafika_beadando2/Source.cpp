#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <iostream>

// Ablak méretei
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Kontrollpont struktúra
struct Point {
    float x, y;
};

// Globális változók
std::vector<Point> controlPoints;
int draggedPointIndex = -1; // -1 jelenti, hogy épp nem mozgatunk pontot
const float POINT_RADIUS = 6.0f; // A pontok átmérõje (3 <= d <= 9)

// Faktoriális számítás a Binomiális együtthatóhoz
unsigned long long factorial(int n) {
    if (n <= 1) return 1;
    unsigned long long res = 1;
    for (int i = 2; i <= n; ++i) res *= i;
    return res;
}

// Binomiális együttható
double binomialCoefficient(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (k > n / 2) k = n - k; // Szimmetria kihasználása (n alatt k = n alatt n-k)

    double res = 1.0;
    for (int i = 1; i <= k; ++i) {
        res = res * (n - k + i) / i;
    }
    return res;
}

// Egér kattintás eseménykezelõje
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (action == GLFW_PRESS) {
        // Ellenõrizzük, hogy kattintottunk-e létezõ pontra
        int clickedIndex = -1;
        for (size_t i = 0; i < controlPoints.size(); ++i) {
            float dx = controlPoints[i].x - (float)xpos;
            float dy = controlPoints[i].y - (float)ypos;
            if (std::sqrt(dx * dx + dy * dy) <= POINT_RADIUS * 1.5f) { // Kis ráhagyás a könnyebb kattintásért
                clickedIndex = i;
                break;
            }
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (clickedIndex != -1) {
                // Meglévõ pont megfogása (Drag and Drop)
                draggedPointIndex = clickedIndex;
            }
            else {
                // Új pont létrehozása üres helyre kattintva
                controlPoints.push_back({ (float)xpos, (float)ypos });
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            // Meglévõ pont eltávolítása jobb gombbal
            if (clickedIndex != -1) {
                controlPoints.erase(controlPoints.begin() + clickedIndex);
            }
        }
    }
    else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
        // Pont elengedése
        draggedPointIndex = -1;
    }
}

// Egér mozgás eseménykezelõje (Drag and drop)
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (draggedPointIndex != -1) {
        controlPoints[draggedPointIndex].x = (float)xpos;
        controlPoints[draggedPointIndex].y = (float)ypos;
    }
}

// Képernyõ újra-rajzolása
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    int n = controlPoints.size() - 1;

    // 1. Kontrollpoligon kirajzolása (sárga)
    if (controlPoints.size() > 1) {
        glColor3f(1.0f, 1.0f, 0.0f); // sárga szín
        glBegin(GL_LINE_STRIP);
        for (const auto& p : controlPoints) {
            glVertex2f(p.x, p.y);
        }
        glEnd();
    }

    // 2. Bézier-görbe kirajzolása törött vonallal (cyan)
    if (controlPoints.size() >= 2) {
        glColor3f(0.0f, 1.0f, 1.0f); // cyan szín
        glBegin(GL_LINE_STRIP);
        int resolution = 100; // Törött vonal részletessége
        for (int i = 0; i <= resolution; ++i) {
            float t = (float)i / resolution;
            float px = 0.0f;
            float py = 0.0f;

            // Általános Bézier formula alkalmazása tetszõleges fokszámra
            for (int j = 0; j <= n; ++j) {
                float blend = binomialCoefficient(n, j) * std::pow(1 - t, n - j) * std::pow(t, j);
                px += blend * controlPoints[j].x;
                py += blend * controlPoints[j].y;
            }
            glVertex2f(px, py);
        }
        glEnd();
    }

    // 3. Kontrollpontok kirajzolása (magenta)
    if (!controlPoints.empty()) {
        glColor3f(1.0f, 0.0f, 1.0f); // magenta szín
        glPointSize(POINT_RADIUS * 2.0f); // Átmérõ beállítása
        glEnable(GL_POINT_SMOOTH); // Kerek pontok
        glBegin(GL_POINTS);
        for (const auto& p : controlPoints) {
            glVertex2f(p.x, p.y);
        }
        glEnd();
        glDisable(GL_POINT_SMOOTH);
    }
}

int main() {

    // --- Programleiras a terminalba (ekezetek nelkul) ---
    std::cout << "Iranyitas:" << std::endl;
    std::cout << "  * BAL egergomb (ures teruleten): Uj kontrollpont" << std::endl;
    std::cout << "  * BAL egergomb (ponton nyomva): Pont mozgatasa" << std::endl;
    std::cout << "  * JOBB egergomb (ponton kattintva): Pont torlese" << std::endl;
    // ---------------------------------------------------
    // Kezdeti 4 kontrollpont (harmadfokú görbéhez)
    controlPoints.push_back({ 100, 300 });
    controlPoints.push_back({ 300, 100 });
    controlPoints.push_back({ 500, 500 });
    controlPoints.push_back({ 700, 300 });

    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Bezier Gorbe Rajzolo", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    // Visszahívó függvények (Callback) beállítása az egérhez
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // Képernyõ vetítési beállításai (Ortho 2D) - Bal felsõ sarok a (0,0)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // fekete háttér

    // Fõ ciklus
    while (!glfwWindowShouldClose(window)) {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
