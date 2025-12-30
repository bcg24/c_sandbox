#include <stdio.h>
#include <stdlib.h>

main(){
    for(int i = 0; i < 10; i++){
        printf("%d: %d\n", i, ((i%4) > 1)?-1:1);
        printf("%d: %d\n", i, (((i+1)%4) > 1)?-1:1);
    }
    exit(EXIT_SUCCESS);
}
