#include <stdio.h>
#include <stdlib.h>

main(){
    char c = getchar();
    while(c != '\n'){
        printf("%c", c);
        c = getchar();
    }
    exit(EXIT_SUCCESS);
}