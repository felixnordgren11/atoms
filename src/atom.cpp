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

float orbitDistance = 100.0;

struct Particle {
    vec2 pos;
    int charge;
    float angle;

    Particle(vec2 pos, int charge, float angle = 0.0f) : pos(pos), charge(charge), angle(angle) {}

    void draw (int segments = 50) {

        float r;
        if (charge == -1){ r = 1.5f; glColor3f(0.0f, 0.7f, 1.0f);}
        else if (charge == 1) {r = 7.0f; glColor3f(1.0f, 0.0f, 0.0f);}
        else {r = 7.0f; glColor3f(0.5f, 0.5f, 0.5f);}

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
    
    
    void update (vec2 center) {
        angle += 0.00005;
        pos= vec2(cos(angle) * orbitDistance + center.x,
                  sin(angle) * orbitDistance + center.y);
    }

};

struct Atom {
    vec2 pos;
    int nElectrons;
    float r = orbitDistance;

    vector<Particle> particles = {
        Particle(pos, 1)
    };

    /* this works for one electron, use angle instead when creating a dynamical amount of electrons
    Particle(pos, 1),
    Particle(vec2(pos.x - orbitDistance, pos.y), -1),
    */
    
    Atom (vec2 pos, int nElectrons) : pos(pos), nElectrons(nElectrons) {
        for (int i = 0; i < nElectrons; i++) {
            float angle = 2.0 * M_PI * i / nElectrons;
            float cx = r*cos(angle) + particles[0].pos.x;
            float cy = r*sin(angle) + particles[0].pos.y;
            particles.emplace_back(Particle(vec2(cx, cy), -1, angle));
        };
    }
};

int main() {

    Engine engine;

    /*vector<Particle> particles = {
        Particle(vec2(0,0), 1),
        Particle(vec2(-orbitDistance, 0.0f), -1),
        Particle(vec2(orbitDistance, 0.0f), -1),
    };*/
    vector<Atom> atoms {
        Atom(vec2( 100.0f,  100.0f), 4),
        Atom(vec2(-100.0f, -100.0f), 3),
        Atom(vec2( 100.0f, -100.0f), 7),
        Atom(vec2(-100.0f,  100.0f), 11),
    };

    // Keep window open until you exit
    while (!glfwWindowShouldClose(engine.window)) {
        engine.run();

        for (Atom& a : atoms) {
            for (Particle& p : a.particles) { // the "&" after the object means that we are referencing the actual object instead of a copy which means that, if we don't have the & we dont update the original object, jsut the copy of the object. 
                p.draw();
                if (p.charge == -1) {
                    p.update(a.particles[0].pos);
                }
            }
        }



        glfwSwapBuffers(engine.window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
};