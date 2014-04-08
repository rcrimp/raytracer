/*
 * File:    mygl.c
 * Author:  Raymond Scurr. (2003).
 * Updated: Raymond Scurr. (2005).
 *
 *
 * Description:
 *  Basic openGL functions for displaying a window, and colouring pixels.
 */


/* ----- INCLUDES ---------------------------------------------------------- */

#include "mygl.h"


/* ----- PRIVATE PROTOTYPES ------------------------------------------------ */
void setPenColour(RGBColour newColour);
void setPenSize(GLdouble newsize);

void setWindow(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top);
void setViewport(GLint left, GLint right, GLint bottom, GLint top);


/* ----- Utility functions ------------------------------------------------- */

void clearScreen(void) {
  glClear(GL_COLOR_BUFFER_BIT);
}

void showScreen(void) {
  glFlush();
}

void setPenColour(RGBColour newColour) {
  glColor3f(newColour.red, newColour.green, newColour.blue);
}

void setPenSize(GLdouble newsize) {
  glPointSize(newsize);
}


/* ----- Basic drawing function -------------------------------------------- */
void drawPixel(GLdouble x, GLdouble y, RGBColour colour) {
  glBegin(GL_POINTS);
    glColor3d(colour.red, colour.green, colour.blue);
    glVertex2d(x, y);
  glEnd();
}


/* ----- Initialisers ------------------------------------------------------ */
void setWindow(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(left, right, bottom, top);
}

void setViewport(GLint left, GLint right, GLint bottom, GLint top) {
  glViewport(left, bottom, right - left, top - bottom);
}


/* wrapper to set up the display event listener */
void mygl_make_display_callback(void displayImage(void)) {
  glutDisplayFunc(displayImage);
}


/* wrapper for the glut main loop */
void mygl_mainLoop() {
  glutMainLoop();
}

/*
 *  mygl_init - sets up some things.
 */
void mygl_init(int* argc,  char** argv, int image_size) {

  //RGBColour black = {0.0f, 0.0f, 0.0f};

  /* make the window */
  glutInit(argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(image_size-1, image_size-1);
  glutInitWindowPosition(100, 150);
  glutCreateWindow("Reuben Crimp");

  /* set up some other things */
  glClearColor(1.0, 1.0, 1.0, 0.0);
  setPenColour(colour_black);
  setPenSize  (1.0);

  /* make the drawing area */
  setWindow  (0, image_size, 0, image_size);
  setViewport(0, image_size, 0, image_size);
}
