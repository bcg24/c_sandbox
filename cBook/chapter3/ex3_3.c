#include <stdio.h>
#include <stdlib.h>
main(){
    int i;

    // initialize
    i = 0;
    // check
    while(i <= 10){
        printf("%d\n", i);
        // update
        i++;
    }
    exit(EXIT_SUCCESS);
}