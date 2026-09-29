#include "../include/matrix_calc.h"
#include <stddef.h>

int process_matrix_3x3(const double input[3][3], double *determinant, double inverse[3][3]) {
    if (input == NULL || determinant == NULL || inverse == NULL) {
        return 0;
    }

    *determinant = input[0][0] * (input[1][1] * input[2][2] - input[1][2] * input[2][1])
                 - input[0][1] * (input[1][0] * input[2][2] - input[1][2] * input[2][0])
                 + input[0][2] * (input[1][0] * input[2][1] - input[1][1] * input[2][0]);

    if (*determinant == 0.0) {
        return 0; 
    }
    double invDet = 1.0 / *determinant;

    inverse[0][0] = (input[1][1] * input[2][2] - input[1][2] * input[2][1]) * invDet;
    inverse[0][1] = (input[0][2] * input[2][1] - input[0][1] * input[2][2]) * invDet;
    inverse[0][2] = (input[0][1] * input[1][2] - input[0][2] * input[1][1]) * invDet;
    
    inverse[1][0] = (input[1][2] * input[2][0] - input[1][0] * input[2][2]) * invDet;
    inverse[1][1] = (input[0][0] * input[2][2] - input[0][2] * input[2][0]) * invDet;
    inverse[1][2] = (input[0][2] * input[1][0] - input[0][0] * input[1][2]) * invDet;
    
    inverse[2][0] = (input[1][0] * input[2][1] - input[1][1] * input[2][0]) * invDet;
    inverse[2][1] = (input[0][1] * input[2][0] - input[0][0] * input[2][1]) * invDet;
    inverse[2][2] = (input[0][0] * input[1][1] - input[0][1] * input[1][0]) * invDet;

    return 1; 
}