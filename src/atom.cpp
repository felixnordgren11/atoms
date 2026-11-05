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

float orbitDistance = 35.0;

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



/*
********* General Particle struct *********
*/
struct Particle {
    vec2 pos;
    int charge;
    float angle;
    int n = 1; // energy level of particle. for hydrogen, energy of a level is -13.6/n**2 eV

    Particle(vec2 pos, int charge, float angle = 0.0f) : pos(pos), charge(charge), angle(angle) {}

    void draw (int segments = 50) {

        // Draw outline for orbit
        glLineWidth(0.4f);
        glBegin(GL_LINE_LOOP);
        glColor3f(0.4f, 0.4f, 0.4f);
        if (charge==1) {for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * M_PI * i/segments;
            float x = orbitDistance * cos(angle);
            float y = orbitDistance * sin(angle);
            glVertex2f(x + pos.x, y + pos.y);
        }; glEnd();}



        float r;
        if      (charge == -1)   {r = 1.5f; glColor3f(0.0f, 0.7f, 1.0f);}
        else if (charge ==  1)   {r = 7.0f; glColor3f(1.0f, 0.0f, 0.0f);}
        else                     {r = 7.0f; glColor3f(0.5f, 0.5f, 0.5f);}
        
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(pos.x, pos.y);
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * M_PI * i/segments;
            float x = r * cos(angle);
            float y = r * sin(angle);
            glVertex2f(x + pos.x, y + pos.y);
        }; glEnd();
    }
    
    
    void update (vec2 center) {
        float r = n * orbitDistance;
        angle += 0.001;
        pos= vec2(cos(angle) * r + center.x,
                  sin(angle) * r + center.y);
    }

};

/* 
********* Wave structs *********
*/
struct WavePoint {vec2 localPos; vec2 dir;};
struct Wave {

    // Variables
    vec2 pos, dir;
    float energy, wavelength = 16.0f;
    // k - wave number = 2pi/wavelength, sigma is?, phase is phase, a is amplitude, angleR is the angle of the ray?
    float sigma=40.0, k=2*M_PI/wavelength, phase=0.0, a=10.0, angleR;
    vector<WavePoint> points;

    Wave(float e, vec2 pos, vec2 dir) : energy(e), pos(pos), dir(dir) {
        dir = normalize(dir);
        for (float x = -sigma; x <= sigma; x+=0.1f) {
            points.push_back({ pos + x*dir, dir*200.0f});
        }
        angleR = atan2(dir.y, dir.x);
    }

    void draw () {
        glBegin(GL_LINE_STRIP);
        for (WavePoint& p : points) {
            // Perp. vector for sine displacement
            vec2 perp(-p.dir.y, p.dir.x);
            perp = normalize(perp);

            // localpos is the x direction as a reference to where you are on the wave currently (for drawing stuff)
            float y_displacement = a*sin(k*length(p.localPos) - phase);
            vec2 drawPos = p.localPos + perp + y_displacement;
            glVertex2f(drawPos.x, drawPos.y);
        } glEnd;
    }
};


/*
********* Atom struct *********
*/
struct Atom {
    vec2 pos;
    int nElectrons;
    float r = orbitDistance, phase;

    vector<Particle> particles = {
        Particle(pos, 1)
    };

    /* this works for one electron, use angle instead when creating a dynamical amount of electrons
    Particle(pos, 1),
    Particle(vec2(pos.x - orbitDistance, pos.y), -1),
    */
    
    Atom (vec2 pos, int nElectrons, float phase = 0.0f) : pos(pos), nElectrons(nElectrons), phase(phase) {
        for (int i = 0; i < nElectrons; i++) {
            float angle = 2.0 * M_PI * i / nElectrons + phase;
            float cx = r*cos(angle) + particles[0].pos.x;
            float cy = r*sin(angle) + particles[0].pos.y;
            particles.emplace_back(Particle(vec2(cx, cy), -1, angle));
        };
    }

    static float excitationEnergy (int N1, int N2) {
        return -13.6f / pow(N2, 2) - -13.6f / pow(N1, 2);
    };
};



int main() {



    Engine engine;

    vector<Wave> waves {};
    vector<Atom> atoms {};

    for (int i=0; i < 13; i++) {
        atoms.push_back(Atom(vec2(0.0f, i*50-300), 3, 20.0f));

    }

    // Initialize waves
    for (int i=0; i < 20; i++) {
        waves.push_back(Wave(Atom::excitationEnergy(1,2), vec2(-250.0f, i*10-200), vec2(-1.0f, 0.0f)));
    }

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