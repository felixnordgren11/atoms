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

Engine engine;


/*
********* General Particle struct *********
*/
struct Particle {
    std::random_device rd;
    std::mt19937 gen(rd());
    vec2 pos;
    int charge;
    float angle;
    int n = 1; // energy level of particle. for hydrogen, energy of a level is -13.6/n**2 eV
    float excitationTimer = 0.0f;

    Particle(vec2 pos, int charge, float angle = 0.0f) : pos(pos), charge(charge), angle(angle) {}

    void draw (vec2 atomCenter, int segments = 50) {

        // Draw outline for orbit
        
        
        if (charge == -1) {
            float r = orbitDistance * n;
            glLineWidth(0.4f);
            glBegin(GL_LINE_LOOP);
            glColor3f(0.4f, 0.4f, 0.4f);
            for (int i = 0; i <= segments; i++) {
                float angle = 2.0f * M_PI * i/segments;
                float x = cos(angle) * r;
                float y = sin(angle) * r;
                glVertex2f(x + atomCenter.x, y + atomCenter.y);
            }; glEnd();}



        float r;
        if      (charge == -1)   {r = 2.5f; glColor3f(0.0f, 0.7f, 1.0f);}
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
        float r_u = n * orbitDistance;
        angle += 0.01;
        pos= vec2(cos(angle) * r_u + center.x,
                  sin(angle) * r_u + center.y);

        if (excitationTimer == 0.0f && n > 1) {
            n--;
            // --- Create a new photon when excitation goes back down ---
            excitationTimer +=0.003;
            double randAngle = std::uniform_real_distribution<> (0.0f, M_PI);
            float waveDirX = cos(randAngle);
            float waveDirY = sin(randAngle);
            float eDiff = Atom::excitationEnergy(n, n+1);
            waves.push_back(eDiff, pos, vec2(waveDirX, waveDirY));
            
        }
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
            vec2 drawPos = p.localPos + perp * y_displacement;
            glVertex2f(drawPos.x, drawPos.y);
        } glEnd();
    }

    void update (float dt) {
        phase += 30.0f*dt;

        for (WavePoint& p : points) {
            // move along velocity
            p.localPos += p.dir * dt;   // Move wave point forward in the wave direction by dt
        }
    }
};


/*
********* Atom struct *********
*/
struct Atom {
    vec2 pos;
    int nElectrons;
    float r = orbitDistance, phase;

    // Add one proton for a hydrogen
    vector<Particle> particles = {
        Particle(pos, 1)
    };
    
    Atom (vec2 pos, int nElectrons, float phase) : pos(pos), nElectrons(nElectrons), phase(phase) {
        for (int i = 0; i < nElectrons; i++) {
            float angle = (2.0 * M_PI * i / nElectrons) + phase;
            float cx = r*cos(angle) + particles[0].pos.x;
            float cy = r*sin(angle) + particles[0].pos.y;
            particles.emplace_back(Particle(vec2(cx, cy), -1, angle));
        };
    }

    static float excitationEnergy (int N1, int N2) {
        /*
        The energy needed to excite an electron from energy state N1 to N2 in a hydrogen atom
        */
        return -13.6f / pow(N2, 2) - -13.6f / pow(N1, 2);
    };
};



int main() {

    vector<Wave> waves {};
    vector<Atom> atoms {};


    for (int i=0; i < 13; i++) {
        atoms.push_back(Atom(vec2(0.0f, i*50-300), 1, radians(i*20.0f)));
    }
    float eH = engine.HEIGHT, eW = engine.WIDTH;
    // Initialize waves
    int nWaves = 20;
    for (int i=0; i < nWaves; i++) {
        waves.push_back(Wave(Atom::excitationEnergy(1,2), vec2(eW / 2.0f, i*(eH/nWaves)-eH/2.0f), vec2(-1.0f, 0.0f)));
    }

    // Keep window open until you exit
    while (!glfwWindowShouldClose(engine.window)) {
        engine.run();

        for (Atom& a : atoms) {
            for (Particle& p : a.particles) { // the "&" after the object means that we are referencing the actual object instead of a copy which means that, if we don't have the & we dont update the original object, jsut the copy of the object. 
                p.draw(a.pos);

                if (p.charge == -1) {

                    if (p.excitationTimer > 0.0f) {
                        p.excitationTimer -= -0.001f
                    }
                    p.update(a.pos);

                    for (Wave& w : waves) {
                        for (WavePoint& wp : w.points) {
                            float dist = length(p.pos - wp.localPos);
                            float energyforExcitation = Atom::excitationEnergy(p.n, p.n+1);
                            if (dist < 20.0f && w.energy == energyforExcitation) {
                                w.energy=0.0f; // absorb photon
                                p.n += 1;      // excite electron
                                p.excitationTimer += 0.003f;
                                break; // so one photon doesn't trigger several energy increases
                            } 
                        }
                    }
                }
            }
        }

        // Draw waves
        for (Wave& w : waves) {
            if (w.energy == 0.0f) continue;
            w.draw();
            w.update(0.01f);
        }

        glfwSwapBuffers(engine.window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
};