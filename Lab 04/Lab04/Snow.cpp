#include "Snow.h"

#define SNOWSIZE 10000
int winWidth = 500, winHeight = 500;
int counter = 0;
time_t t;

unsigned int VBO;

struct drop {
    float x = 400;
    float y = 400;
    float inc = 0.01;
    float radius = 0.5;
};

drop snow [SNOWSIZE];

void drawParticleShape(int i) {
    glBegin(GL_LINES);
    glVertex2d((snow[i].x/500)-1, snow[i].y-2);
    glVertex2d((snow[i].x / 500) -1, snow[i].y-2 + (snow[i].radius * 2)/1000);
    glEnd();
}

void drawDrop(int i) {
    glLineWidth(4);
    drawParticleShape(i);
    snow[i].y -= snow[i].inc;
    if (snow[i].y < -2) {
        snow[i].y = winHeight;
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

void Snow::drawSnow() {
    for (int i = 0; i < SNOWSIZE; i++) {
        drawDrop(i);
    }
    
}


Snow::Snow() {
    srand((unsigned)time(&t));
    for (int i = 0; i < SNOWSIZE; i++) {
        snow[i].x = i;
        snow[i].x = rand() % winWidth;
        snow[i].y = rand() % winHeight;
        snow[i].inc = 0.0001 + (float)(rand() % 50) / 1000.0;
        snow[i].radius = (float)(rand() % 8);
    }
    

}