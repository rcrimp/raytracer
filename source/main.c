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
#include <limits.h>

#include "typedefs.h"
#include "vector_ops.h"
#include "colour_ops.h"
#include "fileio.h"
#include "mygl.h"
#include "ppm.h"

/* ---- Global Variables ----------------------------------------------------*/
/* Must match declarations in typedefs.h
 * Values are loaded in fileio.c */
#define         EPSILON 0.000001f

int             MAX_RECURSE_DEPTH;

int             super_samples;
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

/* quadratic solver
   returns the smallest positive solution to
   At^2 + Bt + C = 0
   if such a solution exist
*/
double quadratic(double A, double B, double C){
   double t1, t2, det;
   if ( B*B > 4*A*C ) {
      det = (B*B) - (4*A*C);
      if ( B > 0 )
         t1 = (-B - sqrt(det)) / 2*A;
      else
         t1 = (-B + sqrt(det)) / 2*A;
      t2 = C / (A*t1);
      
      if(t1 < t2){
         if (t1 > EPSILON) return t1;
         if (t2 > EPSILON) return t2;
      } else {
         if (t2 > EPSILON) return t2;
         if (t1 > EPSILON) return t1;
      }
   } 
   return -1; /* anything negative */
}

/* returns 1 if in shadow; is overkill, should simplify */
int shadow_ray(Vector start, Vector light) {
   int cur_obj;
   RayDef ray;
   Vector end;
   double t;
   for(cur_obj = 0; cur_obj < num_objs; cur_obj++){
      end = vector_transform(light, object[cur_obj].transform);
      ray.start = vector_transform(start, object[cur_obj].transform);
      //ray.direction = vector_transform( vector_subtract( end, start)  , object[cur_obj].transform);
      ray.direction = vector_subtract(end, ray.start);

      double len = vector_length(ray.direction);
      ray.direction = vector_normalise(ray.direction);
      
      t = quadratic(vector_dot(ray.direction, ray.direction),
                    2 * vector_dot(ray.direction, ray.start),
                    vector_dot(ray.start,     ray.start) - 1);
      if ( EPSILON < t && t < len ){
         return 1; /* in shadow */
      }
   }
   return 0; /* not in shadow */
}

/* the main ray tracing procedure */
RGBColour ray_trace(RayDef ray, int recurse_depth) {
   int cur_obj, closest_obj, cur_light, reflective;   
   double t, closest_t, ray_length, reflective_factor = 0.0f; 
   Vector intersection, surface_normal, ToLight, ToCamera;
   RayDef obj_ray;
   RGBColour colour = background_colour;
   
   /* setup */
   closest_t = DBL_MAX;
   closest_obj = -1;

   ray.direction = vector_normalise(ray.direction);

   /* for the current ray, find the closest object */
   for(cur_obj = 0; cur_obj < num_objs; cur_obj++){
      /* transform the ray into object space */
      obj_ray.start = vector_transform(ray.start, object[cur_obj].transform);
      obj_ray.direction = vector_transform(ray.direction, object[cur_obj].transform);      

      ray_length = vector_length(obj_ray.direction);
      obj_ray.direction = vector_normalise(obj_ray.direction);

      t = quadratic(vector_dot(obj_ray.direction, obj_ray.direction),
                    2 * vector_dot(obj_ray.direction, obj_ray.start),
                    vector_dot(obj_ray.start, obj_ray.start) - 1) / ray_length;
      /* check bounds of t */
      if ( EPSILON < t && t < closest_t ){
         closest_t = t;
         closest_obj = cur_obj;
      }
   }

   /* if the ray intersects an object, do lighting for the closest */
   if(closest_obj != -1){
      /* ambient light */
      colour = colour_multiply(object[closest_obj].material.ambient_colour, ambient_light);

      /* transform the ray into object space */
      obj_ray.start = vector_transform(ray.start, object[closest_obj].transform);
      obj_ray.direction = vector_transform(ray.direction, object[closest_obj].transform);

      /* n = (u + vt) . T^(T) */
      surface_normal = vector_normalise(vector_transform( vector_add(obj_ray.start,vector_scale(obj_ray.direction,closest_t)),
                                                          matrix_transpose(object[closest_obj].transform)));
      
      intersection = vector_add(ray.start, vector_scale(ray.direction, closest_t));

      /* move the intersection point slightly above the surface */
      /* intersection = vector_add(intersection, vector_scale(surface_normal, EPSILON)); */
      
      ToCamera = vector_scale(ray.direction, -1);
      //ToCamera = vector_normalise(vector_subtract(ray.start, intersection));
      
#define obj_ref object[closest_obj].material.mirror_colour

      reflective = obj_ref.red > 0.0f || obj_ref.blue > 0.0f || obj_ref.green > 0.0f;
      if (reflective && recurse_depth > 0) { /* if reflective */
         reflective_factor = obj_ref.red * obj_ref.blue * obj_ref.green; /* hack value since the python script doesn't get the actual value */

#undef obj_ref
         
         /*make reflection ray */
         RayDef rec_ray;
         rec_ray.start = intersection;
         rec_ray.direction = vector_subtract( ray.direction, vector_scale( surface_normal, 2*vector_dot( ray.direction, surface_normal)));
         rec_ray.direction.w = 0;
         /* recursively trace the reflection ray, and add it to the colour */
         colour_add_to(&colour, colour_multiply(object[closest_obj].material.mirror_colour, ray_trace(rec_ray, recurse_depth-1)));
      }
      
      for(cur_light = 0; cur_light < num_lights; cur_light++) {
         /* if not in shadow then do lighting */
         if (!shadow_ray(intersection, light_source[cur_light].position)){
            ToLight = vector_normalise(vector_subtract(light_source[cur_light].position, intersection));       
            /* max(0, val) is for attached shadows */
            double lambert = max(0, vector_dot(surface_normal, ToLight));
            Vector r = vector_normalise(vector_subtract(vector_scale(surface_normal, 2*lambert), ToLight));
            double phong = pow(max(0, vector_dot(r, ToCamera)), object[closest_obj].material.phong);
            
#define obj_diff object[closest_obj].material.diffuse_colour
#define obj_spec object[closest_obj].material.specular_colour
#define obj_text object[closest_obj].material.texture
#define light_col light_source[cur_light].colour
#define text_diff texture_diffuse(obj_diff, obj_text, vector_transform(intersection, object[closest_obj].transform))
            /*#define text_diff texture_diffuse(obj_diff, obj_text, surface_normal)*/
               colour_add_to(&colour,colour_scale(1-reflective_factor,colour_multiply(light_col,colour_add(colour_scale(lambert, text_diff),colour_scale(phong, obj_spec)))));    
#undef obj_diff
#undef obj_spec
#undef obj_text
#undef light_col
#undef text_diff
         }
      }
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
   double pixel_size, dof1, dof2;
   int i, j, grid_size;
   
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
   
   /* create the ray */
   vector_set(&ray.start, 0, 0, 0, 1);
   ray.start = vector_transform(ray.start, camera.transform);

   /* super samples / DOF */
   super_samples = max(1, super_samples);
   grid_size = sqrt(super_samples);
   dof1 = dof2 = 0;

   for (row = 0; row < image_size; row++) {
      for (col = 0; col < image_size; col++) {
         /* reset the colour */
         pixelColour = colour_black;
         /* super sampling */
         for(i = 0; i < grid_size; i++){
            for(j = 0; j < grid_size; j++){
               /* DOF */
               if(camera.dof_factor){
                  dof1 = camera.dof_factor * (2 * (rand() / (double)RAND_MAX)-1);
                  dof2 = camera.dof_factor * (2 * (rand() / (double)RAND_MAX)-1);
                  vector_set(&ray.start, dof1, dof2, 0, 1);
                  ray.start = vector_transform(ray.start, camera.transform);
               }
               /*primary ray direction */
               vector_set(&ray.direction,
                          -camera.view_size/2 + pixel_size*(col + (double)i/grid_size) - dof1,
                          camera.view_size/2 - pixel_size*(row + (double)j/grid_size) - dof2,
                          -camera.lens, 0);
               ray.direction = vector_transform(ray.direction, camera.transform);
               /* cast rays, and blend supersample colours */
               colour_add_to(&pixelColour, colour_scale((double)1/super_samples, ray_trace(ray, MAX_RECURSE_DEPTH)));
            }
         }
         drawPixel(col, image_size-row-1, pixelColour);
         writePPM(pixelColour, picfile);
      }
   }
   showScreen();
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
