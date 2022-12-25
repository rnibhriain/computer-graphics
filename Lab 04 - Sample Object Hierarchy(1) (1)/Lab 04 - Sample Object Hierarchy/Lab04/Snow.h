#include <stdio.h>  // this library is for standard input and output
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GL/freeglut.h>

class Snow {
public:
	Snow();
	void drawRain();
private:
	int winWidth = 1000, winHeight = 1000;
	int counter = 0;
	time_t t;
	float rotationAngle = 0;
	void drawParticleShape(int i);
	float elapsedTime = 0, base_time = 0, fps = 0, frames;
};