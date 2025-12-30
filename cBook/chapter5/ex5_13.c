#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GROW_BY 10      // string grows by 10 chars

main(){
    char *str_p, *next_p, *tmp_p;
    int ch, need, chars_read;

    if(GROW_BY < 2){
        fprintf(stderr, "Growth constant too small\n");
        exit(EXIT_FAILURE);
    }

    str_p = (char *)malloc(GROW_BY);
    if(str_p == NULL){
        fprintf(stderr,"No intial store\n");
        exit(EXIT_FAILURE);
    }

    next_p = str_p
}