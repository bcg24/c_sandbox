#include <stdlib.h>
#include <stdio.h>

main(){
    int input_c;    // Has to hold more than char

    /* The Classic Bit */
    while( (input_c = getchar()) != EOF){
        printf("%c was read\n", input_c);
    }
    exit(EXIT_SUCCESS);
}