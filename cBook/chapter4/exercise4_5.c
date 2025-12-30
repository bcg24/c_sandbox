#include <stdio.h>
#include <stdlib.h>

#define PI (355.0/113.0)

float pow(float, int);
int fact(int);

main(){
    float x = PI/4;
    int i = 0;
    float sin = 0, cos = 0;
    float nth_term = 1;
    while((nth_term / sin) > 1e-6 && (nth_term / cos) > 1e-6){       

        nth_term = pow(x, i)/(float)fact(i);

        if(i%2){
            sin += (((i%4) > 1)?-1:1)*nth_term;
        } else{
            cos += ((((i+1)%4) > 1)?-1:1)*nth_term;
        }
        
        i++;
    }
    printf("sin(%f) = %f\n", x, sin);
    printf("cos(%f) = %f\n", x, cos);
    exit(EXIT_SUCCESS);
}

float
pow(float x, int n){
    float result = 1;
    for(int i = 0; i < n; i++)
        result *= x;
    return(result);
}

int
fact(int n){
    int result = 1;
    for(int i = 1; i <= n; i++)
        result *= i;
    return(result);
}