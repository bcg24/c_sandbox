#include <stdio.h>
#include <stdlib.h>
main(){
    int ch;

    ch = getchar();
    while(ch != 'a'){
        if(ch != '\n'){
            printf("ch was %c, value %d \n", ch, ch);
            printf("ch value minus '0' is %d \n", ch - '0');
            printf("ch value minus 0 is %d \n", ch - 0);
            printf("ch value minus '0' is %c \n", ch - '0');
            printf("ch value minus 0 is %c \n", ch - 0);
        }
        ch = getchar();
    }
    exit(EXIT_SUCCESS);
}