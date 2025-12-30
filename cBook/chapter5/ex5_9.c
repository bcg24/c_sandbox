#include <stdio.h>
#include <stdlib.h>

#define ARLEN 10

int main(void) {
    int ar[ARLEN], *ip;

    ip = ar;
    while(ip < &ar[ARLEN])
        *(ip++) = 0;
    for(ip = ar; ip < &ar[ARLEN]; ip++)
        printf("%d", (int)*ip);
    exit(EXIT_SUCCESS);
}