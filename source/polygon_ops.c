/*
 * File: polygon_ops.c
 * Author:  Reuben Crimp
 * Updated: Raymond Scurr. (2001).
 */

/* ---- INCLUDES ------------------------------------------------------------*/
#include "polygon_ops.h"
#include "vector_ops.h"
#include "matrix_ops.h"
#include <math.h>


/* ----- FUNCTIONS --------------------------------------------------------- */
Polygon polygon_new(Vector a, Vector b, Vector c){
   Polygon p;
   p.point[0] = a;
   p.point[1] = b;
   p.point[2] = c;
   p.normal = polygon_normal(p);
   return p;
}
/* calculates the polygon  normal, ignoring the current normal */
Vector polygon_normal(Polygon p){
   /* n = (b-a)x(b-c) */
   return
      vector_cross(
       vector_subtract(p.point[1], p.point[0]),
       vector_subtract(p.point[1], p.point[2])
      );
}

#include <stdio.h>
void polygon_display(Polygon p){
   fprintf(stdout, "triangular Polygon points\n p1: ");
   vector_display(p.point[0]);
   fprintf(stdout, "\np2: ");
   vector_display(p.point[1]);
   fprintf(stdout, "\np3: ");
   vector_display(p.point[2]);
   fprintf(stdout, "\nn : ");
   vector_display(p.normal);
   fprintf(stdout, "\n");
}
