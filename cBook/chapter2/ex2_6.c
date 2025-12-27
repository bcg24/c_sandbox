#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

void delay(int num_seconds);

int main(){
    printf("Size of unsigned int: %zu bytes\n", sizeof(unsigned short int));
    printf("Maximum unsigned int: %u\n", USHRT_MAX);
    printf("That's %zu bits\n", sizeof(unsigned short int) * 8);
    delay(5);
    unsigned short int x;
    x = 0;
    while(x >= 0){
        printf("%u\n", x);
        x = x + 1;
    }

    exit(EXIT_SUCCESS);
}

void delay(int num_seconds){
    time_t start_time = time(NULL);
    while(time(NULL) < start_time + num_seconds)
        ;
}