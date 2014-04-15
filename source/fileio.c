/*
 * File:    fileio.c
 * Author:  Brendan McCane
 * Updated: Scott   2003
 * Updated: Raymond 2005
 * Updated: David   2014
 *
 * Description: Contains the I/O routines.
 */


/* ----- INCLUDES ---------------------------------------------------------- */

/* standard libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* prototypes, and type information */
#include "fileio.h"
#include "typedefs.h"

/* allow us to display colours, vectors, and matrices */
#include "colour_ops.h"
#include "vector_ops.h"
#include "matrix_ops.h"


/* ---- FUNCTIONS ---------------------------------------------------------- */

/*
 *  read the details from named file (or prompt for name if none given).
 *  this assumes that the details are 'clean', and that all necessary
 *  details are given in the file (such as ambient light etc...).
 */
void fileio_readfile(char *fname) {

   FILE *description_file;

   char filename[32];
   char descriptor[20];
   bool tried_opening_file = false;
  
   double x, y, z, angle;
   char axis;
   char ch;

   Matrix transformation;

   /* --- prepare the file for reading --- */

   /* see if a filename has been passed in at the command line! */
   if (fname) {
      /* dme FIX: complete with buffer overflow... */
      strcpy(filename, fname);
      tried_opening_file = true;
   } else {
      filename[0] = '\0';
   }

   printf("Going to try to open the file\n");
  
   while (!(description_file=fopen(filename, "r"))) {
      if (tried_opening_file) {
         fprintf(stderr,
                 "Error: couldn't open file %s - try again\n", filename);
      } else {
         tried_opening_file = true;
      }
      printf("Enter the name of the description file: ");
      fscanf(stdin, "%s", filename);
      printf("\n");
   };

   printf("Opened file\n");

   /* --- prepare default values --- */

   /* maximum number of reflections + Primary Ray */
   MAX_RECURSE_DEPTH = 5;

   /* size of image on screen */
   image_size = 320;

   /* background colour == (0, 0, 0) */
   background_colour.red = 0.0;
   background_colour.green = 0.0;
   background_colour.blue = 0.0;

   /* ambient light == (0, 0, 0) */
   ambient_light.red = 0.0;
   ambient_light.green = 0.0;
   ambient_light.blue = 0.0;

   /* default camera */
   /* viewplane size is constant!! unless we do stupid FOV stuff
    * -- it is 32 units by 32 units (i.e., [-16.0, 16.0] )
    */
   camera.view_size = 32.0;
   camera.lens = 35.0;
   matrix_loadIdentity(&camera.transform);

   /* initial count */	
   num_lights = 0;	
   num_objs = 0;



   /* --- Now read the rest of the values from the file --- */
	
   fscanf(description_file, "%s", descriptor);

   while (strcmp(descriptor, "endview")) {

      /* Comment skipper */
      if (descriptor[0] == '#') {
         ch = descriptor[0];
         while ((ch != 10) && (ch != 13) && (ch != -1)) {
            fscanf(description_file, "%c", &ch);
         }

      } /* pixel size of the output image */
      else if (!strcmp(descriptor, "imagesize")) {
         fscanf(description_file, "%d",  &image_size);

      } /* background colour */
      else if (!strcmp(descriptor, "background")) {
         fscanf(description_file, "%lf", &background_colour.red);
         fscanf(description_file, "%lf", &background_colour.green);
         fscanf(description_file, "%lf", &background_colour.blue);

      }  /* ambient light */
      else if (!strcmp(descriptor, "ambient")) {
         fscanf(description_file, "%lf", &ambient_light.red);
         fscanf(description_file, "%lf", &ambient_light.green);
         fscanf(description_file, "%lf", &ambient_light.blue);

      } /* camera lens */
      else if (!strcmp(descriptor, "lens")) {
         fscanf(description_file, "%lf", &camera.lens);

      } /* lights */
      else if (!strcmp(descriptor, "light")) {

         if (num_lights>=MAX_NUM_LIGHTS) {
            fprintf(stderr,
                    "Error: max number of lights exceeded in description file\n");
            exit(EXIT_FAILURE);
         }
         fscanf(description_file,"%lf",
                &light_source[num_lights].position.x);
         fscanf(description_file,"%lf",
                &light_source[num_lights].position.y);
         fscanf(description_file,"%lf",
                &light_source[num_lights].position.z);
         light_source[num_lights].position.w = 1.0f;
         fscanf(description_file,"%lf",
                &light_source[num_lights].colour.red);
         fscanf(description_file,"%lf",
                &light_source[num_lights].colour.green);
         fscanf(description_file,"%lf",
                &light_source[num_lights].colour.blue);
         num_lights++;

      } /* spheres */
      else if (!strcmp(descriptor, "sphere")) {

         if (num_objs>=MAX_NUM_OBJS) {
	    fprintf(stderr,
                    "Error: max number of objects exceeded in description file\n");
	    exit(EXIT_FAILURE);
         }

         /* read the ambient colour details */
         fscanf(description_file, "%s", descriptor);
         if (strcmp(descriptor, "ambient")) {
	    fprintf(stderr,
		    "ERROR IN FILE: expected keyword ambient for sphere\n");
	    exit(EXIT_FAILURE);
         }
         fscanf(description_file, "%lf",
                &object[num_objs].material.ambient_colour.red);
         fscanf(description_file, "%lf",
                &object[num_objs].material.ambient_colour.green);
         fscanf(description_file, "%lf",
                &object[num_objs].material.ambient_colour.blue);

         /* read the diffuse colour details */
         fscanf(description_file, "%s", descriptor);
         if (strcmp(descriptor, "diffuse")) {
	    fprintf(stderr,
		    "ERROR IN FILE: expected keyword diffuse for sphere\n");
	    exit(EXIT_FAILURE);
         }
         fscanf(description_file, "%lf",
                &object[num_objs].material.diffuse_colour.red);
         fscanf(description_file, "%lf",
                &object[num_objs].material.diffuse_colour.green);
         fscanf(description_file, "%lf",
                &object[num_objs].material.diffuse_colour.blue);

         /* read the specular colour details */
         fscanf(description_file, "%s", descriptor);
         if (strcmp(descriptor, "specular")) {
	    fprintf(stderr,
		    "ERROR IN FILE: expected keyword specular for sphere\n");
	    exit(EXIT_FAILURE);
         }
         fscanf(description_file, "%lf",
                &object[num_objs].material.specular_colour.red);
         fscanf(description_file, "%lf",
                &object[num_objs].material.specular_colour.green);
         fscanf(description_file, "%lf",
                &object[num_objs].material.specular_colour.blue);
         fscanf(description_file, "%lf", &object[num_objs].material.phong);

         /* read the mirror colour details */
         fscanf(description_file, "%s", descriptor);
         if (strcmp(descriptor, "mirror")) {
	    fprintf(stderr,
		    "ERROR IN FILE: expected keyword mirror for sphere\n");
	    exit(EXIT_FAILURE);
         }
         fscanf(description_file, "%lf",
                &object[num_objs].material.mirror_colour.red);
         fscanf(description_file, "%lf",
                &object[num_objs].material.mirror_colour.green);
         fscanf(description_file, "%lf",
                &object[num_objs].material.mirror_colour.blue);
	  
         /* read the texture ID */
         fscanf(description_file, "%s", descriptor);
         if (strcmp(descriptor, "texture")) {
	    fprintf(stderr,
		    "ERROR IN FILE: expected keyword texture for sphere\n");
	    exit(EXIT_FAILURE);
         }
         fscanf(description_file, "%d",
                &object[num_objs].material.texture);


         matrix_loadIdentity(&object[num_objs].transform);
         num_objs++;

      } /* translate last defined object (camera or sphere) */
      else if (!strcmp(descriptor, "translate")) {
         fscanf(description_file, "%lf", &x);
         fscanf(description_file, "%lf", &y);
         fscanf(description_file, "%lf", &z);

         if(num_objs == 0){ // translation of the camera
            matrix_make(&transformation,
                        1.0, 0.0, 0.0, x,
                        0.0, 1.0, 0.0, y,
                        0.0, 0.0, 1.0, z,
                        0.0, 0.0, 0.0, 1.0);
            matrix_multiply_right(&camera.transform, transformation);
         } else { // inverse translation of the current object
            matrix_make(&transformation,
                        1.0, 0.0, 0.0, -x,
                        0.0, 1.0, 0.0, -y,
                        0.0, 0.0, 1.0, -z,
                        0.0, 0.0, 0.0, 1.0);
            matrix_multiply_left(&object[num_objs-1].transform, transformation);
         }
      } /* stretch last defined object (camera or sphere) */
      else if (!strcmp(descriptor, "stretch")) {
         fscanf(description_file, "%lf", &x);
         fscanf(description_file, "%lf", &y);
         fscanf(description_file, "%lf", &z);

         if(num_objs == 0){ //stretching of the camera
            matrix_make(&transformation,
                        x, 0.0, 0.0, 0.0,
                        0.0, y, 0.0, 0.0,
                        0.0, 0.0, z, 0.0,
                        0.0, 0.0, 0.0, 1.0);
            matrix_multiply_right(&camera.transform, transformation);
         } else { //Inverse stretching of the current object
            matrix_make(&transformation,
                        1/x, 0.0, 0.0, 0.0,
                        0.0, 1/y, 0.0, 0.0,
                        0.0, 0.0, 1/z, 0.0,
                        0.0, 0.0, 0.0, 1.0);
            matrix_multiply_left(&object[num_objs-1].transform, transformation);
         }
      } /* rotate last defined object (camera or sphere) */
      else if (!strcmp(descriptor, "rotate")) {
         fscanf(description_file, " %c", &axis);
         if ((axis != 'x') && (axis != 'y') && (axis != 'z')) {
	    fprintf(stderr,
		    "ERROR IN FILE: expected axis for rotation\n");
	    exit(EXIT_FAILURE);
         }
         fscanf(description_file, "%lf", &angle);

         // deg to rads
         angle *= (num_objs == 0) ? (M_PI/180) : -(M_PI/180);
         
         switch(axis){
         case 'x':
            matrix_make(&transformation,
                        1.0, 0.0       , 0.0        , 0.0,
                        0.0, cos(angle), -sin(angle), 0.0,
                        0.0, sin(angle), cos(angle) , 0.0,
                        0.0, 0.0       , 0.0        , 1.0);
            break;
         case 'y':
            matrix_make(&transformation,
                        cos(angle), 0.0, sin(angle), 0.0,
                        0.0       , 1.0, 0.0        , 0.0,
                        -sin(angle), 0.0, cos(angle) , 0.0,
                        0.0       , 0.0, 0.0        , 1.0);
            break;
         case 'z':
            matrix_make(&transformation,
                        cos(angle) , -sin(angle), 0.0, 0.0,
                        sin(angle), cos(angle), 0.0, 0.0,
                        0.0        , 0.0       , 1.0, 0.0,
                        0.0        , 0.0       , 0.0, 1.0);
            break;
         }

         matrix_display(camera.transform);
         matrix_display(transformation);
         
         if(num_objs == 0){ //translate the camera
            //camera.transform = transformation;
            matrix_multiply_right(&camera.transform, transformation);
            matrix_display(camera.transform);
         } else { //translate the current object
            matrix_multiply_left(&object[num_objs-1].transform, transformation);
         }

      } /* unknown descriptor in file */
      else {

         fprintf(stderr, "Error: Unknown descriptor in description file\n");
         exit(-1);
      }

      /* read the next string in the file */
      fscanf(description_file, "%s", descriptor);
   }

   /* close the file*/
   fclose(description_file);
}


/* Display everything read from the file */		
void fileio_printscene() {

   int i;

   printf("\n");
   /* print out the size of the resultant picture */
   printf("Image size      %d\n", image_size);
   printf("\n");

   /* print information about the viewing camera */
   printf("Camera is:\n===\n");
   printf("Viewplane size  %3.1f\n", camera.view_size);
   printf("Lens            %3.1f\n", camera.lens);
   printf("Transformation\n");
   matrix_display(camera.transform);
   printf("\n");

   /* print the background colour */
   printf("background is: ");
   colour_display(background_colour);
   printf("\n");

   /* print the ambient lighting */
   printf("ambient is:    ");
   colour_display(ambient_light);
   printf("\n\n");

   /* print the lights */
   for (i=0; i<num_lights; i++) {
      printf("light is:  ");
      vector_display(light_source[i].position);
      printf("   ");
      colour_display(light_source[i].colour);
      printf("\n");
   }
   printf("\n");


   /* print the objects */
   for (i=0; i<num_objs; i++) {
    
      printf("\nSphere is:\n===\n");

      /* print this object's material details */
      printf("Materials:\n    Ambient:  ");
      colour_display(object[i].material.ambient_colour);
      printf("\n    Diffuse:  ");
      colour_display(object[i].material.diffuse_colour);
      printf("\n    Phong:    ");
      colour_display(object[i].material.specular_colour);
      printf("   coeff [%2.4lf] \n", object[i].material.phong);
      printf("    Mirror:   ");
      colour_display(object[i].material.mirror_colour);

      printf("\nTransformation\n");
      matrix_display(object[i].transform);
      printf("\n");
   }
}
