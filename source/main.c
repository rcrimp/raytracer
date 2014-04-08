/*
 * File: main.c
 * Author: Reuben Crimp
 *
 * Description: Contains the main routine plus the ray-tracing function.
 */

/* ----- INCLUDES ---------------------------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "typedefs.h"
#include "vector_ops.h"
#include "colour_ops.h"
#include "fileio.h"
#include "mygl.h"
#include "ppm.h"

/* ---- Global Variables ----------------------------------------------------*/
/* Must match declarations in typedefs.h
 * Values are loaded in fileio.c */

int             MAX_RECURSE_DEPTH;

CameraDef       camera;
int             image_size;
RGBColour       background_colour;
RGBColour       ambient_light;

int             num_lights;
LightSourceDef  light_source[MAX_NUM_LIGHTS];

int             num_objs;
ObjectDef       object[MAX_NUM_OBJS];

/* ----- main prototypes --------------------------------------------------- */
RGBColour ray_trace(RayDef ray, int recurse_depth);
RGBColour texture_diffuse(RGBColour diffuse_colour, int texture, Vector surface_point);

void renderImage(void);
int main (int argc,  char** argv);

/* ----- Functions --------------------------------------------------------- */

/* Geoff's code that may be used for testing Primary rays;
 * it is not part of the final solution. The code appears after main().
 */
bool circle(Vector v);
RGBColour ray_test(RayDef ray);

/* the main ray tracing procedure */
RGBColour ray_trace(RayDef ray, int recurse_depth) {
   float r;
   RGBColour colour;

   Vector point;

   double t, t1, t2;

   double A, B, C, det;

   Vector SurfacePoint, SurfaceNormal,ToLight;
   
   point.x = 0.0f;
   point.y = 0.0f;
   point.z = -5.0f;
   point.w = 1.0f;
   /*point = vector_transform(object[0].transform, point);*/
   r = 1.0f;
   point = point;

   /*printf("%f %f %f %f\n", point.x, point.y, point.z, point.w);*/

   /*ray.start = vector_add(ray.start, point);*/
   /*ray.start = vector_add(ray.start, point);*/
   /*ray.start = vector_transform(object[0].transform, ray.start);*/
   
   
   A = vector_dot(ray.direction, ray.direction);/* v.v */
   B = 2 * vector_dot(ray.direction, vector_subtract(ray.start,point) );/* 2 u.v */
   C = vector_dot(vector_subtract(ray.start,point), vector_subtract(ray.start,point)) - r; /* u.u -r */
   det = (B*B) - (4*A*C);
   if (det > 0) {
      
      t1 = (-B + sqrt(det)) / 2*A;
      t2 = (-B - sqrt(det)) / 2*A;
      t = max(t1,t2);
      /*printf("%3.1f, %3.1f \n",t1,t2);
      /* 
      SurfacePoint = vector_add(ray.start, vector_scale(ray.direction, t));

      printf("%d\t%d\t%d\t%3.1f, %3.1f %3.1f, %3.1f \n", t1, t2, t, SurfacePoint.x, SurfacePoint.y, SurfacePoint.z, SurfacePoint.w);

      point.x = 0;point.y=0;point.z=-5;point.w=1;
      SurfaceNormal = vector_normalise(vector_subtract(SurfacePoint, point));

      ToLight = vector_normalise(vector_subtract(light_source[0].position,vector_transform(object[0].transform,SurfaceNormal)));
      
      double nl = vector_dot(SurfaceNormal, ToLight);
      colour.red = object[0].material.diffuse_colour.red * light_source[0].colour.red * nl;
      colour.green = object[0].material.diffuse_colour.green * light_source[0].colour.green * nl;
      colour.blue = object[0].material.diffuse_colour.blue * light_source[0].colour.blue * nl;
      */
      colour.blue = colour.green = colour.red = 0.5;   
   } else colour = background_colour;

   return colour;
}

/* A function to modify the diffuse colour if a material's property
   requests this. Note that when texture==0 diffuse_colour is returned
   unmodified. 

   Feel free to merge this code into your own ray_trace function: it's
   presented like this so I can give you the texturing code without
   having to try to position them in the ray_trace skeleton code. */
RGBColour texture_diffuse(RGBColour diffuse_colour, int texture, Vector surface_point){
   /* used in "random noise" texture */
   static int rseed; 

   /* Calculate texture coordinates ( ...even if they're not actually used (!) ) */
   double cu = asin(surface_point.x)/M_PI*180;
   double cv = asin(surface_point.y)/M_PI*180;
   double tmp = sqrt(surface_point.x*surface_point.x+surface_point.z*surface_point.z);
   double su = acos(surface_point.x/tmp)/M_PI*180.0;
   double sv = atan(surface_point.y/tmp)/M_PI*180.0;

   switch(texture){
   case 0: /* default: don't change diffuse colour at all */
      break; 
   case 1: /* checkerboard -- scale original colour */
      /* condition below is Boolean exclusive or */
      if(!((int)(sv+90) % 40 < 20) != !(((int)su+10) % 40 < 20))
         diffuse_colour = colour_scale(0.2,diffuse_colour);
      break;
   case 2: /* noise -- scale original colour */
      /* rseed function here gives not-really-very-random numbers,
         but its determinism could be useful if, say, you wanted
         to interpolate between successive "random" values... */
      rseed = (rseed * 1103515245 + 12345) & ((1U << 31) - 1);
      diffuse_colour = colour_scale(1.0-((rseed % 10)/30.0),diffuse_colour);
      break;
   case 3: /* smooth stripes -- scale original colour */
      diffuse_colour = colour_scale(0.8+0.2*sin(2*su/M_PI),diffuse_colour);
      break;
   case 4: /* rings -- overwrite r+b colour */
      diffuse_colour.red = (0.5+sin(2*cu/M_PI))/2;
      diffuse_colour.blue = (0.5+sin(2*cv/M_PI))/2;
   }	
   return diffuse_colour;
}

/*
 *  The main drawing routine.
 *
 *  Use 'alreadyDrawn' to disable redrawing it when part of the picture
 *  is obscured.
 */
void renderImage(void) {
   
   int row, col;
   double step_size;
   RayDef ray;
   RGBColour pixelColour;
   FILE  *picfile;

   /* avoid redrawing it if the window is obscured */
   static bool alreadyDrawn = false;
   if (alreadyDrawn) return;
   alreadyDrawn = true;
   /* */

   clearScreen();

   /* set up the ppm file for writing */
   picfile = fopen("out.ppm", "w");
   initPPM(image_size, image_size, picfile); 

   /* Calculate the step size */
   step_size = camera.view_size / image_size;/* pixel width */
   step_size = step_size + 0;

   /*printf("%d %3.1f %3.1f %3.1f \n", image_size, camera.view_size, step_size, image_size/camera.view_size);*/
   
   /* create the start point for the primary ray */
   ray.start.x = 0.0;
   ray.start.y = 0.0;
   ray.start.z = 0.0;
   ray.start.w = 1.0;

   /* create the direction of the primary ray */
   ray.direction.x = 0;
   ray.direction.y = 0;
   ray.direction.z = -camera.lens; /* fudge value */
   ray.direction.w = 0.0f;
  
   /* create the image pixel by pixel */
   for (row = 0; row < image_size; row++) {
      ray.direction.y = 3*(-image_size/camera.view_size/2 + row/camera.view_size);
      /*ray.direction.y = -camera.view_size/6.5 + row/camera.view_size;*/
      for (col = 0; col < image_size; col++) {
         ray.direction.x = 3*(-image_size/camera.view_size/2 + col/camera.view_size);
         /* ray.direction.x = -camera.view_size/6.5 + col/camera.view_size; */
         
         /* Find out the colour of this pixel */
         pixelColour = ray_trace(ray, 0); /* ray_trace(ray, MAX_RECURSE_DEPTH); */

         /* Note that GL draws from bottom, not the top! */
         drawPixel(col, image_size-row-1, pixelColour);
         writePPM(pixelColour, picfile);
      }
   }

   /* make sure all of the picture is displayed */
   showScreen();

   /* close the ppm file */
   fclose(picfile);

   /* finished */
   printf("\nDone\n");

}


/* ------------------------------------------------------------------------- */
/* Main */
int main (int argc,  char** argv) {


   
   /* read the scene file */
   fileio_readfile(argv[1]);
   fileio_printscene();

   /* set up openGL, and call the render */
   mygl_init(&argc, argv, image_size);
   mygl_make_display_callback(renderImage);
   mygl_mainLoop();

   return EXIT_SUCCESS;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* function that could be useful for testing the early part of the raytracer */
/* ------------------------------------------------------------------------- */
/* circle:
 *
 * will test the ray for intersections with a 'standard' circle
 */

bool circle(Vector v) {
   return (v.z < 0) && (1225 * ((v.x*v.x) + (v.y*v.y)) < 256 * (v.z * v.z));
}

/* ray_test:
 *  Maybe useful for testing the Primary ray, before any sphere intersections
 *  are written
 *
 * For testing purposes,
 *     replace  ray_trace(ray, 0);
 *     with     ray_test(ray);
 *     in the renderImage() double loop.
 *
 * When the camera is in the default position (no transformations) 
 * it should produce a circle
 *
 */
RGBColour ray_test(RayDef ray) {

   static RGBColour black = {0.0, 0.0, 0.0};
   static RGBColour white = {1.0, 1.0, 1.0};

   if (circle(ray.direction)) {
      return white;
   } else {
      return black;
   }
}
