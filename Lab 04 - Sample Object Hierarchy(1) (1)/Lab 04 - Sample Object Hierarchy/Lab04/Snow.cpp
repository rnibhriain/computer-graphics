#include "Snow.h"

#define RAINSIZE 500
int winWidth = 500, winHeight = 500;
int counter = 0;
time_t t;
float rotationAngle = 0;

unsigned int VBO;

struct drop {
    float x = 500;
    float y = 500;
    float z = 1000;
    float inc = 0.01;
    float radius = 5;
    float scale = 1.0;
    float rotationAngle = 0;
    float rotationInc = 1;
};

drop rain[RAINSIZE];

void drawParticleShape(int i) {
    glBegin(GL_POINTS);
    glVertex2d(rain[i].x, rain[i].y);
    glEnd();
    glBegin(GL_LINES);
    glVertex2d(rain[i].x, rain[i].y);
    glVertex2d(rain[i].x, rain[i].y + rain[i].radius * 2);
    glEnd();
}

void drawDrop(int i) {
    glColor3f(255.0, 255.0, 255.0);
    glLineWidth(2);
    drawParticleShape(i);
    rain[i].y -= rain[i].inc;
    if (rain[i].y < 0) {
        rain[i].y = winHeight;
    }
}

float elapsedTime = 0, base_time = 0, fps = 0, frames;

void calcFPS() {
    elapsedTime = glutGet(GLUT_ELAPSED_TIME);
    if ((elapsedTime - base_time) > 1000.0) {
        fps = frames * 1000.0 / (elapsedTime - base_time);
        base_time = elapsedTime;
        frames = 0;
    }
    frames++;
}

void Snow::drawRain() {
    glMatrixMode(GL_PROJECTION);
    for (int i = 0; i < RAINSIZE; i++) {
        drawDrop(i);
    }
    calcFPS();
    glFlush();
}


Snow::Snow() {

    srand((unsigned)time(&t));
    for (int i = 0; i < RAINSIZE; i++) {
        rain[i].x = rand() % 100;
        rain[i].y = rand() % 100;
        rain[i].z = rand() % 100;
        rain[i].inc = 0.1 + (float)(rand() % 100) / 1000.0;
        rain[i].radius = (float)(rand() % 8);
        rain[i].scale = (float)(rand() % 20000) / 1000.0;
        rain[i].rotationAngle = (float)(rand() % 3000) / 1000.0;
        rain[i].rotationInc = (float)(rand() % 100) / 1000.0;
        if ((rand() % 100) > 50) {
            rain[i].rotationInc = -rain[i].rotationInc;
        }
    }
}