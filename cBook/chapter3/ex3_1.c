#include <stdlib.h>
#include <stdio.h>

main(){
    int i = -10;

    while(i <= 5){
        printf("valued of i is %d, ", i);
        printf("i == 0 = %d, ", i==0 );
        printf("i > -5 = %d\n", i > -5);
        i++;
    }
    exit(EXIT_SUCCESS);
}