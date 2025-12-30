#include <stdio.h>
#include <stdlib.h>

main(){
    int i, j;

    for(i=0, j=0; i <= 10; i++, j = i*i){
        printf("i %d j %d\n", i, j);
    }

    printf("Overall: %d\n", ("abc", 1.2e6, 4*3+2));
    exit(EXIT_SUCCESS);
}