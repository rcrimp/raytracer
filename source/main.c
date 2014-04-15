/*
 * File: main.c
 * Author: Nabeelah Saib
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
 * Values are loaded in fileio.c
 */

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
 
   RGBColour colour = background_colour;

   int i;
   for(i = 0; i < num_objs; i++){
      
      Vector objectPoint;
      objectPoint.x = 0.0f;
      objectPoint.y = 0.0f;
      objectPoint.z = -5.0f;
      objectPoint.w = 1.0f;

      ray.direction = vector_normalise(ray.direction);
      ray.start = vector_subtract(ray.start, objectPoint);
      
      double A = vector_dot(ray.direction, ray.direction);
      double B = 2.0*(vector_dot(ray.start, ray.direction));
      double C = vector_dot(ray.start, ray.start) - 1;

      double D = (B*B) - (4*A*C);
      double t = 0.0;
      double t1, t2;

      /*checks if the ray collides with the sphere by calculating the t
        values*/
      if(D >= 0){ 
         if(B > 0){
            t1 = ((-B - sqrt(D)))/(2.0 *A);
         }else{
            t1 = ((-B + sqrt(D)))/(2.0 *A);
         }
         t2 = (C/(A * t1));
      
         t = min(t1, t2);

         /*sets up variables for light equation*/
         
         Vector vt;
         vt = vector_scale(ray.direction, t);
 
         Vector hitPoint = vector_add(ray.start, vt);
         hitPoint = vector_normalise(hitPoint);
         Vector normal = hitPoint;
         /*normal = vector_norm(normal);*/

         Vector l = vector_subtract(light_source[i].position, hitPoint);
         l = vector_normalise(l);
         double nl = max(0, vector_dot(normal, l));
        
         Vector e = vector_subtract(ray.start, hitPoint);
         e = vector_normalise(e);
         
         /*to calculate r in the equation we use r = 2*(l.n) * n-l;*/
         //double scaler = 2.0 *  nl;
         //Vector nlMinus =  vector_subtract(normal, l);
         //Vector r = vector_scale(nlMinus, scaler);
         //r = vector_normalise(r);
         //double re = vector_dot(r, e);
         Vector r = vector_subtract(vector_scale(normal, 2*nl), l);
         double re = vector_dot(r, e);
         
         re = max(0, re);
         re = pow(re, object[i].material.phong);

         /* vector_display(e);
            printf("\n");*/
            
         /* calculates how the point is coloured (r, g, b)  using the light
            model equation I = Ia ka + Il [kd (n.l) + ks(r.e)^n], where Ia
            is the intensity of the ambient light, Il is the intensity
            of the light source and ka, kd, ks and n are all conststants*/
         colour.red = ambient_light.red
            * object[i].material.ambient_colour.red
            + light_source[0].colour.red
            * ((object[i].material.diffuse_colour.red *  nl)
               + (object[i].material.specular_colour.red * re));

         colour.green = (ambient_light.green
                         * object[i].material.ambient_colour.green)
            +  light_source[0].colour.green 
            * ((object[i].material.diffuse_colour.green *  nl)
               + (object[i].material.specular_colour.green * re));
         
         colour.blue = (ambient_light.blue
                        * object[i].material.ambient_colour.blue)
            +  light_source[0].colour.blue
            * ((object[i].material.diffuse_colour.blue *  nl)
               + (object[i].material.specular_colour.blue * re));

         /*moves the light source back to original position view*/
         /*colour.red = light_source[ind].position.z - t;
         colour.green = light_source[ind].position.z - t;
         colour.blue = light_source[ind].position.z - t;*/

      }
      
   }
  
   
        

   return (colour);
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
   double pixelSize;

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
   step_size = 0;
   pixelSize = camera.view_size/image_size;
  
   /* create the start point for the primary ray */
   ray.start.x = 0.0;
   ray.start.y = 0.0;
   ray.start.z = 0.0;
   ray.start.w = 1.0;

   /*equation of circle*/
   /*(x * x) + (y * y) = 1;*/

  

  
  
   /* create the direction of the primary ray */
   ray.direction.x = 0.0;
   ray.direction.y = 0.0;
   ray.direction.z = -camera.lens;
   ray.direction.w = 0.0;

   /* create the image pixel by pixel */
   for (row = 0; row < image_size; row++) {
      for (col = 0; col < image_size; col++) {

         /* Find out the colour of this pixel */
         ray.direction.x = -camera.view_size/2 + ((0.5 +col) * pixelSize);
         ray.direction.y= -camera.view_size/2 + ((0.5 + row) * pixelSize); 
         pixelColour = ray_trace(ray, 0);

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
