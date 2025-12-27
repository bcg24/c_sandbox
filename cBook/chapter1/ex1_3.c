#include <stdio.h>
#include <stdlib.h>

int returnInt();

main(){
    int numbers;

    numbers = returnInt();
    printf("%d\n", numbers);
    exit(EXIT_SUCCESS);
}

int returnInt(){
    int result, stop, ch;
    stop = 0;

    while(stop == 0){
        ch = getchar();
        result = result * 10 + (ch - '0');  
        if(ch != '\n'){
            stop = 1;
        }
    }
    return(result);
}