#include <stdio.h>
#include <stdlib.h>

#define BOILING 212     /* [degF] */

main(){
    float f_var; double d_var; long double l_d_var;
    int ii;

    ii = 0;
    printf("Farenheit to Centigrade\n");
    while(ii <= BOILING){
        l_d_var = 5*(ii-32);
        l_d_var = l_d_var/9;
        d_var = l_d_var;
        f_var = l_d_var;
        printf("%d %f %f %lf\n", ii,
                f_var, d_var, l_d_var);
        ii = ii + 1;
    }
    exit(EXIT_SUCCESS);
}