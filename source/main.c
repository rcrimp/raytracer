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
#define SUPER_SAMPLES 1 /* needs to be a square number */

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
int shadow_ray(Vector point1, Vector point2, int obj_caster);
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

   //printf("%3.20f\n", sv-cv);
   //if(abs(cv - sv) > 0.0000000001 )
   //printf("different\n"); /* it seems cv = sv for unit spheres at least */
   
   switch(texture){
   case 0: /* default: don't change diffuse colour at all */
      break; 
   case 1: /* checkerboard -- scale original colour */
      /* condition below is Boolean exclusive or */
      if(((int)(sv+90) % 40 < 20) == (((int)su+10) % 40 < 20))
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

/* returns 1 if shadow, overkill will simplify */
int shadow_ray(Vector point1, Vector point2, int obj_caster) {
   int cur_obj, closest_obj;
   RayDef ray, cur_ray;
   ray.start = point1;
   ray.direction = vector_subtract(point2,point1);

   //double t = INT_MAX;
   //int nearest_object = -1;
   double A, B, C, det, t1, t2, t, temp;
   t = DBL_MAX;
   closest_obj = -1;
   
   for(cur_obj = 0; cur_obj < num_objs; cur_obj++){
      if (cur_obj == obj_caster) break; /* assume no self shadowing */
      cur_ray.start = vector_transform(ray.start, object[cur_obj].transform);
      cur_ray.direction = vector_transform(ray.direction, object[cur_obj].transform);
      temp = vector_length(cur_ray.direction);
      cur_ray.direction = vector_normalise(cur_ray.direction);
      
      A =     vector_dot(cur_ray.direction, cur_ray.direction);  /* v.v */
      B = 2 * vector_dot(cur_ray.direction, cur_ray.start);      /* 2 * u.v */
      C =     vector_dot(cur_ray.start,     cur_ray.start) - 1;  /* u.u -r */
      det = (B*B) - (4*A*C); /* determinant */
      
      if (det > 0) { /* if ray collides with the sphere */
         if ( B > 0 )
            t1 = (-B - sqrt(det)) / 2*A;
         else
            t1 = (-B + sqrt(det)) / 2*A;
         t2 = C / (A*t1);

         t1 /= temp;
         t2 /= temp;
         
         /* if the current object is closer than any prior objects, then set it as the closest */
         temp = min(t1,t2);
         if(temp < t){
            t = temp;
            closest_obj = cur_obj;
         }
      }
   }
   if ( t < 1 ) return 1;
   return 0;
}

/* the main ray tracing procedure */
RGBColour ray_trace(RayDef ray, int recurse_depth) {
   int cur_obj, closest_obj, cur_light, i;   
   RGBColour colour = background_colour;
   double A, B, C, det, t1, t2, t, temp; //quadratic variables
   Vector SurfaceNormal, ToLight, ToCamera;

   Vector cur_light_pos;

   RayDef cur_ray;
   
   ray.start = vector_transform(ray.start, camera.transform);
   ray.direction = vector_transform( vector_normalise(ray.direction), camera.transform);
   
   /* setup */
   t = DBL_MAX;
   closest_obj = -1;

   /* for the current ray, find the closest object */
   for(cur_obj = 0; cur_obj < num_objs; cur_obj++){
      cur_ray.start = vector_transform(ray.start, object[cur_obj].transform);
      cur_ray.direction = vector_transform(ray.direction, object[cur_obj].transform);

      temp = vector_length(cur_ray.direction);

      cur_ray.direction = vector_normalise(cur_ray.direction);

      /* quadratic representation of the line-sphere intersection */
      A =     vector_dot(cur_ray.direction, cur_ray.direction);  /* v.v */
      B = 2 * vector_dot(cur_ray.direction, cur_ray.start);      /* 2 * u.v */
      C =     vector_dot(cur_ray.start,     cur_ray.start) - 1;  /* u.u -r */
      det = (B*B) - (4*A*C); /* determinant */
      
      if (det > 0) { /* if ray collides with the sphere, find the distance (t) to the object */
         if ( B > 0 )
            t1 = (-B - sqrt(det)) / 2*A;
         else
            t1 = (-B + sqrt(det)) / 2*A;
         t2 = C / (A*t1);

         t1 /= temp;
         t2 /= temp;
         
         /* if the current object is closer than any prior objects, then set it as the closest */
         temp = min(t1,t2);
         if(temp < t){
            t = temp;
            closest_obj = cur_obj;
         }
      }
   }

   /* if the ray intersects an object, do lighting for the closest */
   if(closest_obj != -1){
      /* ambient light */
      colour.red   = object[closest_obj].material.ambient_colour.red   * ambient_light.red;
      colour.green = object[closest_obj].material.ambient_colour.green * ambient_light.green;
      colour.blue  = object[closest_obj].material.ambient_colour.blue  * ambient_light.blue;

      cur_ray.start = vector_transform(ray.start, object[closest_obj].transform);
      cur_ray.direction = vector_transform(ray.direction, object[closest_obj].transform);
      
      /* Lighting calculations */
      SurfaceNormal = (vector_add(cur_ray.start, vector_scale(cur_ray.direction, t)));
      //SurfaceNormal = SurfaceNormal;
      //SurfaceNormal = vector_transform(SurfaceNormal, matrix_transpose(object[closest_obj].transform));

      ToCamera = vector_normalise(vector_subtract(cur_ray.start, SurfaceNormal));

      //if (reflective & recurse_depth >= 0) {
      //   reflected_colour = ray_trace( ray(intersection point, reflection vector), n-1);
      //   colour += reflection_coeff * reflected_colour ;
      //}
      //if (refractive & recurse_depth >= 0) {
      //   refracted_colour = ray_trace( Ray(intersection point, refracted vector), n-1);
      //   colour += refraction_coeff * refracted_colour ;
      //}
      
      /* for each light */
      for(cur_light = 0; cur_light < num_lights; cur_light++) {
         
         
         cur_light_pos = vector_transform(light_source[cur_light].position, object[closest_obj].transform);

         Vector rename_me =
            //vector_subtract(vector_transform(SurfaceNormal, object[closest_obj].transform),SurfaceNormal);
            //SurfaceNormal;
            //vector_transform(SurfaceNormal, object[closest_obj].transform);
            vector_new(
                       SurfaceNormal.x += object[closest_obj].transform.element[0][3],
                       SurfaceNormal.y += object[closest_obj].transform.element[1][3],
                       SurfaceNormal.z += object[closest_obj].transform.element[2][3],
                       SurfaceNormal.w = 1);
         //if (shadow_ray(rename_me, cur_light_pos, closest_obj) == 0 ){
         if (shadow_ray(rename_me, light_source[cur_light].position, closest_obj) == 1){
         ToLight = vector_normalise(vector_subtract(cur_light_pos, SurfaceNormal));

            double nl = vector_dot(SurfaceNormal, ToLight);
            Vector r = vector_normalise(vector_subtract(vector_scale(SurfaceNormal, 2*nl), ToLight));
            double rv =  vector_dot(r, ToCamera);
         
            /* range: 0-1 */
            nl = max(0, nl);
            rv = pow( max(0, rv) , object[closest_obj].material.phong);

#define obj_diff object[closest_obj].material.diffuse_colour
#define obj_spec object[closest_obj].material.specular_colour
#define obj_text object[closest_obj].material.texture
#define light_col light_source[cur_light].colour
         
            RGBColour texc = texture_diffuse(obj_diff, obj_text, SurfaceNormal);
            colour.red   += light_col.red   * ( texc.red   * nl + obj_spec.red   * rv);
            colour.green += light_col.green * ( texc.green * nl + obj_spec.green * rv);
            colour.blue  += light_col.blue  * ( texc.blue  * nl + obj_spec.blue  * rv);

#undef obj_diff
#undef obj_spec
#undef obj_tex
#undef light_col
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

   int load_count_max = (image_size/100);
   int load_count;
   fprintf(stderr, "Loading \n<");
   
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
      if(load_count++ > load_count_max){
         load_count = 0;
         fprintf(stderr, "=");
      }
   }
   fprintf(stderr, ">\n");

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
