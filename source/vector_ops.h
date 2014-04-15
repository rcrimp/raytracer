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
 */

typedef struct Vector {
   double x, y, z, w;
} Vector;

/* ---- FUNCTION HEADERS --------------------------------------------------- */

Vector vector_new       (double x, double y, double z, double w);
Vector vector_subtract  (Vector v, Vector u);
Vector vector_add       (Vector v, Vector u);
Vector vector_scale     (Vector v, double s);
double vector_dot       (Vector v, Vector u);
Vector vector_cross     (Vector v, Vector u);
double vector_length    (Vector v);
Vector vector_normalise (Vector v);
Vector vector_transform (Vector v, Matrix m);
void   vector_display   (Vector v);

#endif /* _VECTOR_OPS_H_ */


