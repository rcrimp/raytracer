/*
 * File: typedefs.h
 * Author: Raymond
 *
 * Description: Contains the required typedefs.
 */

#ifndef _TYPEDEFS_H_
#define _TYPEDEFS_H_


/* ----- INCLUDES ---------------------------------------------------------- */

/* ANSI-standard includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* our includes */
#include "vector_ops.h"
#include "matrix_ops.h"
#include "colour_ops.h"


/* ----- CONSTANTS --------------------------------------------------------- */

/* booleans are not supported in this version of C, so make our own */
#define bool  int
#define true  1
#define false 0

#define MAX_NUM_LIGHTS     16
#define MAX_NUM_OBJS      64

/* ----- TYPE DECLARATIONS ------------------------------------------------- */

/* ray is defined by a point and a direction */
typedef struct RayDef {
   Vector start;
   Vector direction;
} RayDef;
	

/* information about our point light source */
typedef struct LightSourceDef {
   Vector position;
   RGBColour colour;
} LightSourceDef;


/* material properties for a surface */
typedef struct MaterialProperty {
   RGBColour ambient_colour;
   RGBColour diffuse_colour;
   RGBColour specular_colour;
   double phong;
   RGBColour mirror_colour;
   int texture;
} MaterialProperty;


/* See Fileio.c  to see how to access the values in the array of objects */
typedef struct ObjectDef {
   MaterialProperty material;
   Matrix transform;
} ObjectDef;



/*
 * We provide viewing geometry details here
 */
typedef struct CameraDef {
   double view_size;  /* defines the camera square view plane */
   double lens;       /* distance from camera to view plane */
   Matrix transform;  /* transformation from the 'standard' position */
} CameraDef;


/* ---- MACROS --------------------------------------------------------------*/

#define sqr(x)       ((x)*(x))

#define max(x,y)     (((x) > (y)) ? (x) : (y))

#define min(x,y)     (((x) < (y)) ? (x) : (y))



/* ---- GLOBAL VARIABLES ----------------------------------------------------*/

extern int             MAX_RECURSE_DEPTH;

extern CameraDef       camera;
extern int             image_size;
extern RGBColour       background_colour;
extern RGBColour       ambient_light;

extern int             num_lights;
extern LightSourceDef  light_source[MAX_NUM_LIGHTS];

extern int             num_objs;
extern ObjectDef       object[MAX_NUM_OBJS];



#endif  /* _TYPEDEFS_H_ */
