/*
 * File: main.c
 * Author: Reuben Crimp
 *
 * Description: Contains the main routine plus the ray-tracing function.
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "typedefs.h"
#include "vector_ops.h"
#include "colour_ops.h"
#include "fileio.h"
#include "mygl.h"
#include "ppm.h"

/* ---- Global Variables ----------------------------------------------------*/
/* Must match declarations in typedefs.h
 * Values are loaded in fileio.c */
#define SUPER_SAMPLES 16 /* needs to be a square number */

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

/* A function to modify the diffuse colour if a material's property
   requests this. Note that when texture==0 diffuse_colour is returned
   unmodified. */
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


/* the main ray tracing procedure */
RGBColour ray_trace(RayDef ray, int recurse_depth) {
   int cur_obj, i;   
   RGBColour colour;
   Vector obj_translation;
   double A, B, C, det, t1, t2, t; //quadratic variables
   Vector SurfaceNormal, ToLight, ToCamera;
   Vector cur_ray_start;
   Vector cur_ray_dir;
   Vector cur_light_pos;
   
   /* setup */
   colour = background_colour;

   /* copy the ray, we don't want to modify the original */
   //cur_ray_start = ray.start;
   cur_ray_dir = vector_normalise(ray.direction);

   int ray_intersected = 0;
   double intersection[num_objs];
   for(i = 0; i < num_objs; i++){
      intersection[i] = DBL_MAX;
   }
   
   for(cur_obj = 0; cur_obj < num_objs; cur_obj++){ //for each object

      obj_translation = vector_new(0,0,0,1);
      obj_translation = vector_transform(object[cur_obj].transform, obj_translation);

      //cur_ray_start = vector_subtract(ray.start, obj_translation);
      cur_ray_start = vector_transform(object[cur_obj].transform, ray.start);
      
      A = vector_dot(cur_ray_dir, cur_ray_dir);/* v.v */
      B = 2 * vector_dot(cur_ray_dir, cur_ray_start );/* 2 * u.v */
      C = vector_dot(cur_ray_start, cur_ray_start) - 1; /* u.u -r */
      det = (B*B) - (4*A*C);
      if (det > 0) {
         if ( B > 0 )
            t1 = (-B - sqrt(det)) / 2*A;
         else
            t1 = (-B + sqrt(det)) / 2*A;
         
         t2 = C / (A*t1);

         intersection[cur_obj] = min(t1, t2);
         ray_intersected = 1;
      }
   }

   if(ray_intersected){
      cur_obj = 0;
      t = intersection[0];

      /* find closest object */
      for(i = 1; i < num_objs; i++){
         if (intersection[i] < t){
            t = intersection[i];
            cur_obj = i;
         }
      }

      /* get object translation */
      //obj_translation = vector_new(0,0,0,1);
      //obj_translation = vector_transform(object[cur_obj].transform, obj_translation);

      /* translate light and objects */
      //cur_ray_start = vector_subtract(ray.start, obj_translation);
      //cur_light_pos = vector_subtract(light_source[0].position, obj_translation);
      cur_ray_start = vector_transform(object[cur_obj].transform, ray.start);
      cur_light_pos = vector_transform(object[cur_obj].transform, light_source[0].position);
      
      /*  */
      SurfaceNormal = (vector_add(cur_ray_start, vector_scale(cur_ray_dir, t)));
      ToLight = vector_normalise(vector_subtract(cur_light_pos, SurfaceNormal));
      ToCamera = vector_normalise(vector_subtract(cur_ray_start, SurfaceNormal));
         
      double nl = vector_dot(SurfaceNormal, ToLight);
      Vector r = vector_normalise(vector_subtract(vector_scale(SurfaceNormal, 2*nl), ToLight));
      double rv =  vector_dot(r, ToCamera);

      /* range: 0-1 */
      nl = max(0, nl);
      rv = max(0, rv);
         
      rv = pow(rv, object[cur_obj].material.phong);

      RGBColour texc = texture_diffuse(object[cur_obj].material.diffuse_colour, object[cur_obj].material.texture, SurfaceNormal);

      //colour = object[cur_obj].material.diffuse_colour;
      
      /* calculate RGB */
      
        colour.red =
         object[cur_obj].material.ambient_colour.red * ambient_light.red +
         object[cur_obj].material.diffuse_colour.red * light_source[0].colour.red * texc.red * nl + 
         object[cur_obj].material.specular_colour.red * light_source[0].colour.red * rv;

      colour.green =
         object[cur_obj].material.ambient_colour.green * ambient_light.green +
         object[cur_obj].material.diffuse_colour.green * light_source[0].colour.green * texc.green * nl + 
         object[cur_obj].material.specular_colour.green * light_source[0].colour.green * rv;

      colour.blue =
         object[cur_obj].material.ambient_colour.blue * ambient_light.blue +
         object[cur_obj].material.diffuse_colour.blue * light_source[0].colour.blue * texc.blue * nl + 
         object[cur_obj].material.specular_colour.blue * light_source[0].colour.blue * rv;
      
   }
   return colour;
}

/*
 *  The main drawing routine.
 */
void renderImage(void) {
   int row, col;
   RayDef ray;
   RGBColour pixelColour;
   FILE  *picfile;
   double pixel_size;
   RGBColour samples[SUPER_SAMPLES];

   /* avoid redrawing it if the window is obscured */
   static bool alreadyDrawn = false;
   if (alreadyDrawn) return;
   alreadyDrawn = true;

   clearScreen();

   /* set up the ppm file for writing */
   picfile = fopen("out.ppm", "w");
   initPPM(image_size, image_size, picfile); 

   /* Calculate the step size */
   pixel_size = camera.view_size/image_size;
   
   /* create the start point for the primary ray */
   ray.start = vector_new(0,0,0,1);
   
   /* create the direction of the primary ray */
   ray.direction = vector_new(0,0,-camera.lens,0);
      
   /* super samples */
   int i, j, grid_size;
   grid_size = sqrt(SUPER_SAMPLES);
   
   for (row = 0; row < image_size; row++) {
      for (col = 0; col < image_size; col++) {

         /* super sampling */
         for(i = 0; i < grid_size; i++){
            for(j = 0; j < grid_size; j++){
               ray.direction.x = -camera.view_size/2 + pixel_size*(col + (double)i/grid_size);
               ray.direction.y = camera.view_size/2 - pixel_size*(row + (double)j/grid_size);
               samples[j + i*grid_size] = ray_trace(ray, 0);
            }
         }
         pixelColour = colour_blend(samples, SUPER_SAMPLES);
         
         /* no super sampling */
         /*
           ray.direction.x = -camera.view_size/2 + pixel_size*(0.5 + col);
           ray.direction.y = -camera.view_size/2 + pixel_size*(0.5 + row);
           pixelColour = ray_trace(ray, 0);
         */
         drawPixel(col+1, image_size-row-1, pixelColour);
         writePPM(pixelColour, picfile);
      }
   }

   /* make sure all of the picture is displayed */
   showScreen();

   /* close the ppm file */
   fclose(picfile);

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
