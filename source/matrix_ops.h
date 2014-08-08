/*
 * File: matrix_ops.h
 *
 * Description: Header file for matrix operations.
 */

#ifndef _MATRIX_OPS_H
#define _MATRIX_OPS_H

/* ---- INCLUDES ------------------------------------------------------------*/
#include <stdio.h>       /* for io */
#include <math.h>        /* for maths functions, and M_PI */

/* ---- CONSTANT DECLARATION ------------------------------------------------*/
#define  MATRIX_SIZE   4

#ifndef M_PI
#define M_PI           3.141592653589793238462643383279
#endif

#define  Pi                M_PI
#define  radiansPerDegree  (M_PI/180.0)


/* ---- TYPE DECLARATION ----------------------------------------------------*/
typedef struct Matrix {
  double element[MATRIX_SIZE][MATRIX_SIZE];
} Matrix;

/* ---- FUNCTION HEADERS ----------------------------------------------------*/

Matrix matrix_identity(void);
void matrix_loadIdentity(Matrix *ident);

void matrix_make(Matrix *m,
		double a00, double a01, double a02, double a03,
		double a10, double a11, double a12, double a13,
		double a20, double a21, double a22, double a23,
		double a30, double a31, double a32, double a33);

void matrix_add(Matrix *m, Matrix n);
void matrix_subtract(Matrix *m, Matrix n);
void matrix_scale(Matrix *m, double s);
void matrix_multiply_right(Matrix *m, Matrix n);
void matrix_multiply_left(Matrix *m, Matrix n);
void matrix_display(Matrix m);
Matrix matrix_transpose(Matrix m);


#endif /*  _MATRIX_OPS_H  */





