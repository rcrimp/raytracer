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
   int cur_obj, closest_obj, cur_light, i;   
   RGBColour colour = background_colour;
   double A, B, C, det, t1, t2, t, closest_t, ray_length, temp; //quadratic variables
   Vector intersection, surface_normal, ToLight, ToCamera;
   RayDef obj_ray;

   /* transform ray by the camera orientation
    put this in the main loop maybe ?? */
   ray.start = vector_transform(ray.start, camera.transform);
   
   ray.direction = vector_transform(ray.direction, camera.transform);

   ray.direction = vector_normalise(ray.direction);
   /* setup */
   closest_t = DBL_MAX;
   closest_obj = -1;

   /* for the current ray, find the closest object */
   for(cur_obj = 0; cur_obj < num_objs; cur_obj++){
      /* transform the ray into object space */
      obj_ray.start = vector_transform(ray.start, object[cur_obj].transform);
      obj_ray.direction = vector_transform(ray.direction, object[cur_obj].transform);      
      //obj_ray.direction = vector_normalise(obj_ray.direction);
      ray_length = vector_length(obj_ray.direction);

      obj_ray.direction = vector_normalise(obj_ray.direction);

      //if ((ray_length-1.0f) != 1 )
      //printf("%f ", ray_length);

      A = vector_dot(obj_ray.direction, obj_ray.direction);  /* v.v */
      B = 2 * vector_dot(obj_ray.direction, obj_ray.start);  /* 2 * u.v */
      C = vector_dot(obj_ray.start,     obj_ray.start) - 1;  /* u.u -r */
      det = (B*B) - (4*A*C); /* determinant */
      if (det > 0) { /* if ray collides with the sphere, find the distance (t) to the object */
         if ( B > 0 )
            t1 = (-B - sqrt(det)) / 2*A;
         else
            t1 = (-B + sqrt(det)) / 2*A;
         t2 = C / (A*t1);
         
         /*t1 = (-B + sqrt(det)) / 2*A;
         t2 = (-B - sqrt(det)) / 2*A;
         */
         //t1 /= ray_length;
         //t2 /= ray_length;
         
         t = min(t1,t2) / ray_length;

        //vector_display(ray.direction);
         //vector_display(vector_transform(ray.direction, object[cur_obj].transform));
         //vector_display(obj_ray.direction);

         //if (cur_obj == 0) t /= 5;
         //printf("%f %f %f\n", t1, t2, t);

         if(t > 0 && t < closest_t){
            closest_t = t;
            closest_obj = cur_obj;
         }
      }
   }

   /* if the ray intersects an object, do lighting for the closest */
   if(closest_obj != -1){
      /* ambient light */
      colour = colour_multiply(object[closest_obj].material.ambient_colour, ambient_light);
      
      obj_ray.start = vector_transform(ray.start, object[closest_obj].transform);
      obj_ray.direction = vector_transform(vector_normalise(ray.direction), object[closest_obj].transform);

      surface_normal = (vector_add(obj_ray.start, vector_scale(obj_ray.direction, closest_t)));
      surface_normal = vector_normalise(vector_transform(surface_normal, matrix_transpose(object[closest_obj].transform)));
      
      /* Lighting calculations */      
      intersection = vector_add(ray.start, vector_scale(vector_normalise(ray.direction), closest_t));
      
      ToCamera = vector_normalise(vector_subtract(ray.start, intersection));
      //ToCamera = vector_normalise(vector_scale(ray.direction, -1.0f));
      
      //if (reflective & recurse_depth > 0) {
      //   reflective_colour = ray_trace( Ray(intersection point, reflective vector), n-1);
      //   colour += reflective_coeff * reflective_colour ;
      //}
      //if (refractive & recurse_depth > 0) {
      //   refracted_colour = ray_trace( Ray(intersection point, refracted vector), n-1);
      //   colour += refraction_coeff * refracted_colour ;
      //}
      
      for(cur_light = 0; cur_light < num_lights; cur_light++) {
         if (1){ //cast a shadow ray from intersection to light
            ToLight = vector_normalise(vector_subtract(light_source[cur_light].position, intersection));
            //Vector FromLight = vector_normalise(vector_subtract(intersection, light_source[cur_light].position));
            
            /* max(0, val) may be unnesacary when casting shadow rays */
            double lambert = max(0, vector_dot(surface_normal, ToLight));
            Vector r = vector_normalise(vector_subtract(vector_scale(surface_normal, 2*lambert), ToLight));
            //Vector r = /*v-2n(n.v)*/ vector_subtract(FromLight, vector_scale(surface_normal, 2*vector_dot(surface_normal,FromLight)));
            double phong = max(0, vector_dot(r, ToCamera));
            phong = pow(phong, object[closest_obj].material.phong);
            
#define obj_diff object[closest_obj].material.diffuse_colour
#define obj_spec object[closest_obj].material.specular_colour
#define obj_text object[closest_obj].material.texture
#define light_col light_source[cur_light].colour
#define text_diff texture_diffuse(obj_diff, obj_text, surface_normal)
            
            //colour.red += light_col.red * (lambert * obj_diff.red + phong * obj_spec.red);
            //colour.blue += light_col.blue * (lambert * obj_diff.blue + phong * obj_spec.blue);
            //colour.green += light_col.green * (lambert * obj_diff.green + phong * obj_spec.green);

            colour_add_to(&colour, colour_multiply(light_col, colour_add(colour_scale(lambert, text_diff), colour_scale(phong, obj_spec))));

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
   double pixel_size;
   double px, py;
   
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
   //ray.start = vector_transform(ray.start, camera.transform);   
   vector_set(&ray.direction, 0, 0, 0, 0);

   /* super samples */
   super_samples = max(1, super_samples);
   int i, j, grid_size = sqrt(super_samples);
   
   for (row = 0; row < image_size; row++) {
      for (col = 0; col < image_size; col++) {

         pixelColour = colour_black;
         
         /* super sampling */
         for(i = 0; i < grid_size; i++){
            for(j = 0; j < grid_size; j++){
               /* DOF */
               if(camera.dof_factor){
                  //ray.start.x = camera.dof_factor * 2*(rand() / (double)RAND_MAX)-1;
                  //ray.start.y = camera.dof_factor * 2*(rand() / (double)RAND_MAX)-1;
               }

               px = -camera.view_size/2 + pixel_size*(col + (double)i/grid_size);
               py = camera.view_size/2 - pixel_size*(row + (double)j/grid_size);

               vector_set(&ray.direction, px - ray.start.x, py - ray.start.y, -camera.lens - ray.start.z, 0);
               //ray.direction = vector_transform(ray.direction, camera.transform);              

               colour_add_to(&pixelColour, colour_scale((double)1/super_samples, ray_trace(ray,10)));
            }
         }
         
         /* no super sampling or dof */
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

RGBColour ray_trace_test(RayDef ray, int rec){
   double A, B, C, det;
   RGBColour colour;
   ray.start.z += 5;
   
   A = vector_dot(ray.direction, ray.direction); //v.v
   B = 2 * vector_dot(ray.direction, ray.start ); //2*u.v
   C = vector_dot(ray.start, ray.start) - 1; //u.u -r;
   det = (B*B) - (4*A*C);
   if (det > 0) //hits sphere
      colour.red = colour.green = colour.blue = 1;
   return background_colour;
}
