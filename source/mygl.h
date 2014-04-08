/*
 * File: mygl.h
 *
 * Description: Header file for openGL functions.
 */


#ifndef _MYGL_H_
#define _MYGL_H_

/* ----- INCLUDES ---------------------------------------------------------- */

/* OpenGL includes */
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include "colour_ops.h"


/* ----- PROTOTYPES -------------------------------------------------------- */


void clearScreen(void);
void showScreen(void);

void drawPixel(GLdouble x, GLdouble y, RGBColour colour);

void mygl_init(int* argc,  char** argv, int image_size);
void mygl_make_display_callback(void displayImage(void));
void mygl_mainLoop();


#endif /*  _MYGL_H_  */
