#include <stdio.h>
#include <stdlib.h>

main(){
    int i;
    for(i = 0; i <= 10; i++){
        switch(i){
            case 1:
            case 2:
                printf("1 or 2: %d\n", i);
            case 7:
                printf("7: %d\n", i);
            default:
                printf("default: %d\n", i);
        }
    }
    exit(EXIT_SUCCESS);
}