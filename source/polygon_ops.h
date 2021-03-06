/*
 * File: vector_ops.h
 *
 * Description: Header file for vector operations.
 */

#ifndef _POLYGON_OPS_H_
#define _POLYGON_OPS_H_
#include "vector_ops.h"

/* ---- FUNCTION HEADERS --------------------------------------------------- */

/* triangular polygon defined by 3 points and a normal */
typedef struct Polygon {
   Vector point[3];
   Vector normal;
} Polygon;

Polygon polygon_new(Vector a, Vector b, Vector c);
Vector polygon_normal(Polygon p);
void polygon_display(Polygon p);

#endif


