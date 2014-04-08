/*
 * File: vector_ops.c
 * Author:  Brendan McCane.
 * Updated: Raymond Scurr. (2001).
 *
 * Description: Vector ops. You'll probably want to add more functions here.
 */

/* ---- INCLUDES ------------------------------------------------------------*/
#include "vector_ops.h"
#include "matrix_ops.h"
#include <math.h>

/* ----- TYPE DECLARATIONS ------------------------------------------------- */

/*
 * this a record structure that contains a 4D vector (x, y, z, w).
 *
 * Note - if w == 1 then it represents a point.
 *        if w == 0 then it represents a vector.
 *
 *   typedef struct _Vector {
 *      double x, y, z, w;
 *   } Vector;
 */


/* ----- FUNCTIONS --------------------------------------------------------- */

/* subtract two vectors: return a-b */
Vector vector_subtract(Vector a, Vector b) {

  Vector result;
  
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;
  result.w = a.w - b.w;

  return(result);
}

/* add two vectors: return a+b */
Vector vector_add(Vector a, Vector b) {
  Vector result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;
  result.w = a.w + b.w;
  return(result);
}

Vector vector_scale(Vector a, double s){
   Vector result;
   result.x = a.x * s;
   result.y = a.y * s;
   result.z = a.z * s;
   result.w = a.w * s;
   return result;
}

double vector_dot(Vector a, Vector b){
   return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vector vector_cross(Vector a, Vector b){
   /* a.x  a.y  a.z  a.x  a.y  --- */
   /* a.y  b.y  b.z  b.x  b.y  --- */

   /* (a.x, a.y, a.z) x (b.x, b.y, b.z) =
   (a.y*b.z - b.y*a.z, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x)
   */
   Vector result;
   result.x = a.y*b.z - b.y*a.z;
   result.y = a.z*b.x - a.x*b.z;
   result.z = a.x*b.y - a.y*b.x;
   result.w = 0;
   
   return result;
}

double vector_length(Vector a){
   double result = sqrt(vector_dot(a,a));
   return result;
}

Vector vector_normalise(Vector a){
   Vector result;
   double length;

   length = vector_length(a);
   result = vector_scale(a, 1/length);
   
   return result;
}

Vector vector_transform(Matrix m, Vector v){
   Vector result;

   result.x = m.element[0][0]*v.x + m.element[0][1]*v.y + m.element[0][2]*v.z + m.element[0][3]*v.w;
   result.y = m.element[1][0]*v.x + m.element[1][1]*v.y + m.element[1][2]*v.z + m.element[1][3]*v.w;
   result.z = m.element[2][0]*v.x + m.element[2][1]*v.y + m.element[2][2]*v.z + m.element[2][3]*v.w;
   result.w = m.element[3][0]*v.x + m.element[3][1]*v.y + m.element[3][2]*v.z + m.element[3][3]*v.w;
   
   return result;
}

/* display a vector
 *
 *  States if the vector is a  POINT   (w == 1.0)
 *  or if it is a direction    VECTOR  (w == 0.0)
 *
 */
#include <stdio.h>
void   vector_display(Vector v) {

  if (v.w == 0.0) {
    fprintf(stdout, "Vector ");
  } else if (v.w == 1.0){
    fprintf(stdout, "Point  ");
  } else {
    fprintf(stdout, "UNKNOWN ");
  }

  fprintf(stdout, "(%1.3f, %1.3f, %1.3f)", v.x, v.y, v.z);

}
