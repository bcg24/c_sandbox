#include <stdio.h>
#include <stdlib.h>

int main(){
    int ch;
    
    ch = getchar();

    while(ch != '\n'){
        ch = getchar();
        printf("Character: %c Integer: %d\n", ch, ch);
    }
    exit(EXIT_SUCCESS);
}