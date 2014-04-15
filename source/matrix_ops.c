
/* ----- INCLUDES ---------------------------------------------------------- */

#include "matrix_ops.h"

/* ---- TYPE DECLARATION ----------------------------------------------------*/
/*
  typedef struct Matrix {
  double element[MATRIX_SIZE][MATRIX_SIZE];
  } Matrix;
*/


/* ---- FUNCTIONS -----------------------------------------------------------*/

/* Create an identity matrix */
Matrix matrix_identity(void) {

   Matrix result;

   matrix_make(&result,1.0, 0.0, 0.0, 0.0,
               0.0, 1.0, 0.0, 0.0,
               0.0, 0.0, 1.0, 0.0,
               0.0, 0.0, 0.0, 1.0);

   return result;
}


/* Replace the passed matrix with the identity matrix*/
void matrix_loadIdentity(Matrix *ident) {
   (*ident) = matrix_identity();
}


/* Construct the matrix from the passed values */
void matrix_make(Matrix *m,
                 double a00, double a01, double a02, double a03,
                 double a10, double a11, double a12, double a13,
                 double a20, double a21, double a22, double a23,
                 double a30, double a31, double a32, double a33) {

   (*m).element[0][0] = a00;(*m).element[0][1] = a01;
   (*m).element[0][2] = a02;(*m).element[0][3] = a03;

   (*m).element[1][0] = a10;(*m).element[1][1] = a11;
   (*m).element[1][2] = a12;(*m).element[1][3] = a13;

   (*m).element[2][0] = a20;(*m).element[2][1] = a21;
   (*m).element[2][2] = a22;(*m).element[2][3] = a23;

   (*m).element[3][0] = a30;(*m).element[3][1] = a31;
   (*m).element[3][2] = a32;(*m).element[3][3] = a33;
}

/*
 * Add the matrix toAdd to the current sum matrix
 *
 * Sample call:
 *    matrix_add(&result, temp);
 *
 * will update the values in the matrix result.
 */
void matrix_add(Matrix *m, Matrix n) {
   int row, col;
   for (row = 0; row < MATRIX_SIZE; row++){
      for (col = 0; col < MATRIX_SIZE; col++){
         (*m).element[row][col] += n.element[row][col];
      }
   }
}
void matrix_subtract(Matrix *m, Matrix n) {
   int row, col;
   for (row = 0; row < MATRIX_SIZE; row++){
      for (col = 0; col < MATRIX_SIZE; col++){
         (*m).element[row][col] -= n.element[row][col];
      }
   }
}

void matrix_scale(Matrix *m, double s){
   int row, col;
   for (row = 0; row < MATRIX_SIZE; row++){
      for (col = 0; col < MATRIX_SIZE; col++){
         m->element[row][col] *= s;
      }
   }
}

void matrix_multiply_right(Matrix *m, Matrix n){
   int row, col;

   for (row = 0; row < MATRIX_SIZE; row++) { // for each row of m
      for (col = 0; col < MATRIX_SIZE; col++) { // for each column of n
         m->element[row][col] =
            m->element[0][row] * n.element[col][0] +
            m->element[1][row] * n.element[col][1] +
            m->element[2][row] * n.element[col][2] +
            m->element[3][row] * n.element[col][3];
      }
   }
}

void matrix_multiply_left(Matrix n, Matrix *m){
   int row, col;

   for (row = 0; row < MATRIX_SIZE; row++) { // for each row of m
      for (col = 0; col < MATRIX_SIZE; col++) { // for each column of n
         m->element[row][col] =
            m->element[0][row] * n.element[col][0] +
            m->element[1][row] * n.element[col][1] +
            m->element[2][row] * n.element[col][2] +
            m->element[3][row] * n.element[col][3];
      }
   }
}

Matrix matrix_transpose(Matrix m){
   Matrix result;
   matrix_make(&result,
                    m.element[0][0], m.element[1][0], m.element[2][0], m.element[0][3],
                    m.element[0][1], m.element[1][1], m.element[2][1], m.element[3][1],
                    m.element[0][2], m.element[2][1], m.element[2][2], m.element[3][2],
                    m.element[0][3], m.element[3][1], m.element[3][2], m.element[3][3]);
   return result;
}

/*
 *  Output Matrix - for testing and diagnostics
 */
void matrix_display(Matrix m) {
   int row, col;

   for (row = 0; row < MATRIX_SIZE; row++) {
      printf("    ");
      for (col = 0; col < MATRIX_SIZE; col++)
         printf("%2.4f ", m.element[row][col]);
      printf("\n");
   }
}





