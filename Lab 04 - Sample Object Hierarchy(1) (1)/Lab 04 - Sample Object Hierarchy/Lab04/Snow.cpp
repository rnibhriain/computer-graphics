#include "Snow.h"

#define RAINSIZE 5000
int winWidth = 500, winHeight = 500;
int counter = 0;
time_t t;
float rotationAngle = 0;

unsigned int VBO;

struct drop {
    float x = 1000;
    float y = 1000;
    float z = 1000;
    float inc = 0.01;
    float radius = 0.5;
    float scale = 1.0;
    float rotationAngle = 0;
    float rotationInc = 1;
};

drop rain [RAINSIZE];

void drawParticleShape(int i) {
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
    glColor4f(0, 0.0, 0.25, 0.05);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_POINTS);
      //  glVertex2d(rain[i].y, rain[i].y);
    glEnd();
    glBegin(GL_LINES);
        glVertex2d(rain[i].y, rain[i].y);
        glVertex2d(rain[i].y, rain[i].y+4);
        glColor4f(1, 1, 1, 1);
    glEnd();
}

void drawDrop(int i) {
    glColor3f(1.0, 1.0, 1.0);
    glLineWidth(2);
    drawParticleShape(i);
    rain[i].y -= rain[i].inc;
    if (rain[i].y < -500) {
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
    for (int i = 0; i < RAINSIZE; i++) {
        drawDrop(i);
    }
    
}


Snow::Snow() {
    srand((unsigned)time(&t));
    for (int i = 0; i < RAINSIZE; i++) {
        rain[i].x = rand() % winWidth;
        rain[i].y = rand() % winHeight;
        rain[i].inc = 1.1 + (float)(rand() % 100) / 1000.0;
        rain[i].radius = (float)(rand() % 8);
        rain[i].scale = (float)(rand() % 20000) / 1000.0;
        //rain[i].rotationAngle = (float)(rand() % 3000) / 1000.0;
        rain[i].rotationInc = (float)(rand() % 100) / 1000.0;
        if ((rand() % 100) > 50) {
            rain[i].rotationInc = -rain[i].rotationInc;
        }
    }
    

}