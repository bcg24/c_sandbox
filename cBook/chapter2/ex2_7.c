#include <stdlib.h>
#include <stdio.h>
main(){
    int ii;
    unsigned int stop_val;

    stop_val = 0;
    ii = -10;

    while(ii <= stop_val){
        printf("%d\n", ii);
        ii = ii + 1;
    }   
    
    exit(EXIT_SUCCESS);
}