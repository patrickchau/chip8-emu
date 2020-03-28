
#include <iostream>
#include "chip8.h"
#include <conio.h>
#include <stdio.h>
#include <Windows.h>
#include <GL\glew.h>
#include <GL\freeglut.h>

using namespace std;

// comes from the standard size of the chip8 display which had 64 x positions and 32 y positions
const int WINDOW_WIDTH = 64;
const int WINDOW_HEIGHT = 32;
const int modifier = 10;
int display_width = WINDOW_WIDTH * modifier;
int display_height = WINDOW_HEIGHT * modifier;

chip8 cpu;

bool setupCallbacks();
void changeViewPort(GLsizei w, GLsizei h);
void render();
void keydown(unsigned char key, int x, int y);
void keyup(unsigned char key, int x, int y);


// changes all necessary settings for GLUT window
bool setupCallbacks() {
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(display_width, display_height);
	glutCreateWindow("Chip 8 Emulator");

	glutReshapeFunc(changeViewPort);
	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutDisplayFunc(render);
	glutIdleFunc(render);
	
	// check for GLEW errors and if so, then exit
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW error");
		return false;
	}
	
	// runs GLUT
	glutMainLoop();
	return true;
}

// changes the size of the GLUT window
void changeViewPort(GLsizei w, GLsizei h)
{
	// standard setup for GLUT.
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,w-1,h-1,0);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, w, h);

	display_width = w;
	display_height = h;
}

// runs in the GLUT main loop, renders what we need
void render()
{
	cpu.emulateCycle();
	
	if (cpu.drawFlag) {
		glClear(GL_COLOR_BUFFER_BIT);

		for (int y = 0; y < 32; y++) {
			for (int x = 0; x < 64; x++) {
				if (cpu.gfx[(y * 64) + x] == 1){
					glColor3f(1.0f, 1.0f, 1.0f);
				}
				else {
					glColor3f(0.0f, 0.0f, 0.0f);
				}
				GLfloat x1 = x * modifier;
				GLfloat x2 = (x * modifier) + modifier;
				GLfloat y1 = (y * modifier);
				GLfloat y2 = (y * modifier) + modifier;
				glBegin(GL_POLYGON);
				glVertex2f(x1, y1);
				glVertex2f(x2, y1);
				glVertex2f(x2, y2);
				glVertex2f(x1, y2);
				glEnd();
			}
		}
		
		glutSwapBuffers();

		cpu.drawFlag = false;
	}

}


int main( int argc, char* argv[] ) {

	// sets up GLUT
	glutInit(&argc, argv);

	// initialize settings and clear memory
	cpu.initialize();

	// read in the file and store into memory
	if (!cpu.loadGame(argv[1])) {
		// if loading the rom fails, just return
		return 1;
	}
	
	// setup callbacks runs the main loop, and runs our CPU emulation every cycle
	if (!setupCallbacks()) {
		// if it fails, just return
		return 1;
	}
	
	return 0;
}

void keydown(unsigned char key, int x, int y) {
	// sets a key to 1 (pressed) when down
	int pressed = -1;
	switch (key) {
	case '1':
		pressed = 0;
		break;
	case '2':
		pressed = 1;
		break;
	case '3':
		pressed = 2;
		break;
	case '4':
		pressed = 3;
		break;
	case 'q':
		pressed = 4;
		break;
	case 'w':
		pressed = 5;
		break;
	case 'e':
		pressed = 6;
		break;
	case 'r':
		pressed = 7;
		break;
	case 'a':
		pressed = 8;
		break;
	case 's':
		pressed = 9;
		break;
	case 'd':
		pressed = 10;
		break;
	case 'f':
		pressed = 11;
		break;
	case 'z':
		pressed = 12;
		break;
	case 'x':
		pressed = 13;
		break;
	case 'c':
		pressed = 14;
		break;
	case 'v':
		pressed = 15;
		break;
	default:
		pressed = -1;
		break;
	}

	if (pressed != -1) {
		cpu.key[pressed] = 1;
		//printf("Key Down: %c", key);
	}
}

void keyup(unsigned char key, int x, int y) {
	// sets a key to 0 in the emulator when key is up.
	int pressed = -1;
	switch (key) {
	case '1':
		pressed = 0;
		break;
	case '2':
		pressed = 1;
		break;
	case '3':
		pressed = 2;
		break;
	case '4':
		pressed = 3;
		break;
	case 'q':
		pressed = 4;
		break;
	case 'w':
		pressed = 5;
		break;
	case 'e':
		pressed = 6;
		break;
	case 'r':
		pressed = 7;
		break;
	case 'a':
		pressed = 8;
		break;
	case 's':
		pressed = 9;
		break;
	case 'd':
		pressed = 10;
		break;
	case 'f':
		pressed = 11;
		break;
	case 'z':
		pressed = 12;
		break;
	case 'x':
		pressed = 13;
		break;
	case 'c':
		pressed = 14;
		break;
	case 'v':
		pressed = 15;
		break;
	default:
		pressed = -1;
		break;
	}

	if (pressed != -1) {
		cpu.key[pressed] = 0;
		//printf("Key Up: %c", key);
	}
}
