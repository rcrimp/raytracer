/*
 * File: vector_ops.h
 *
 * Description: Header file for vector operations.
 */

#ifndef _VECTOR_OPS_H_
#define _VECTOR_OPS_H_
#include "matrix_ops.h"

/* ---- TYPE DECLARATIONS -------------------------------------------------- */

/*
 * this a record structure that contains a 4D vector (x, y, z, w).
 *
 * Note - if w == 1 then it represents a point.
 *        if w == 0 then it represents a vector.
 *
 */

typedef struct Vector {
   double x, y, z, w;
} Vector;


/* ---- FUNCTION HEADERS --------------------------------------------------- */

Vector vector_new(double x, double y, double z, double w);
Vector vector_subtract(Vector a, Vector b);
Vector vector_add(Vector a, Vector b);
Vector vector_scale(Vector a, double s);
double vector_dot(Vector a, Vector b);
Vector vector_cross(Vector a, Vector b);
double vector_length(Vector a);
Vector vector_normalise(Vector a);
Vector vector_transform(Matrix m, Vector v);
Vector vector_transform_inv(Matrix m, Vector v);
void vector_display(Vector v);

#endif /* _VECTOR_OPS_H_ */


