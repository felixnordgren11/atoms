#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <ctime>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace glm;
using namespace std;



struct Engine {

    GLFWwindow* window;
    double aspect_ratio = 16.0/9.0;
    int WIDTH = 1200;
    int HEIGHT = WIDTH / aspect_ratio;
    

    Engine () {
        // --- Initiate GLFW ---
        if (!glfwInit()) {
            cerr << "Failed to initiate glfw    \n";
            exit(EXIT_FAILURE);
        }
        // Create window
        window = glfwCreateWindow(WIDTH, HEIGHT, "2D atom sim", nullptr, nullptr);
        if (!window) {
            cerr << "Failed to create window.   \n";
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);
    }
    
    void run() {
        glClear(GL_COLOR_BUFFER_BIT);
        // Load
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // Update center
        double halfWidth = WIDTH / 2.0f, halfHeight = HEIGHT / 2.0f;
        glOrtho(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.0, 1.0); // set the dimensions of the window to be 
        // Reload
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    };
};

float orbitDistance = 150.0;

struct Particle {
    vec2 pos;
    int charge;
    float angle;

    Particle(vec2 pos, int charge) : pos(pos), charge(charge) {}

    void draw (int segments = 50) {

        float r;
        if (charge == -1){ r = 6.0f; glColor3f(0.0f, 0.7f, 1.0f);}
        else if (charge == 1) {r = 30.0f; glColor3f(1.0f, 0.0f, 0.0f);}
        else {r = 30.0f; glColor3f(0.5f, 0.5f, 0.5f);}

        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(pos.x, pos.y);

        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * M_PI * i/segments;
            float x = r * cos(angle);
            float y = r * sin(angle);
            glVertex2f(x + pos.x, y + pos.y);
        }

        glEnd();
    }
    
    
    void update () {
        angle += 0.05;
        pos.x = cos(angle) * orbitDistance;
        pos.y = sin(angle) * orbitDistance;
    }

};



int main() {

    Engine engine;

    vector<Particle> particles = {
        Particle(vec2(0,0), 1),
        Particle(vec2(-orbitDistance, 0.0f), -1),
        Particle(vec2(orbitDistance, 0.0f), -1),
    };

    // Keep window open until you exit
    while (!glfwWindowShouldClose(engine.window)) {
        engine.run();

        for (Particle p : particles) {
            p.draw();
            if (p.charge == -1) p.update();
        }



        glfwSwapBuffers(engine.window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
};