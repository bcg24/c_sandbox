#include <stdio.h>
#include <cblas.h>

int main(void) {
    double x[] = {1.0, 2.0, 3.0, 4.0};
    double y[] = {5.0, 6.0, 7.0, 8.0};
    
    // Compute dot product: 1*5 + 2*6 + 3*7 + 4*8 = 70
    double result = cblas_ddot(4, x, 1, y, 1);
    
    printf("Dot product: %f\n", result);
    printf("Expected:    70.0\n");
    
    return 0;
}