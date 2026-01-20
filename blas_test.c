#include <stdio.h>
#include <cblas.h>

int main(void) {
    double x[] = {1.0, 2.0, 3.0, 4.0};
    double y[] = {5.0, 6.0, 7.0, 8.0};
    
    // Compute dot product: 1*5 + 2*6 + 3*7 + 4*8 = 70
    double result = cblas_ddot(4, x, 1, y, 1);
    
    printf("Dot product: %f\n", result);
    printf("Expected:    70.0\n");
    

    int i=0;
    double A[6] = {1.0,2.0,1.0,-3.0,4.0,-1.0};         
    double B[6] = {1.0,2.0,1.0,-3.0,4.0,-1.0};  
    double C[9] = {.5,.5,.5,.5,.5,.5,.5,.5,.5}; 
    cblas_dgemm(CblasColMajor, CblasNoTrans, 
        CblasTrans,3,3,2,1,A, 3, B, 3,2,C,3);

    for(i=0; i<9; i++)
        printf("%lf ", C[i]);
    printf("\n");

    return 0;

}