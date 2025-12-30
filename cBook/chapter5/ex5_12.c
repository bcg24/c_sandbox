#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSTRING       50
#define MAXLEN          80

void print_arr(const char **p_array);   // takes a pointer to pointer to charater
void sort_arr(const char **p_array);
char *next_string(void);

main(){
    char **p_array; // a pointer to a pointer to a character
    int nstrings;

    p_array = (char **)malloc(sizeof(char *[MAXSTRING+1])); // allocate memory big enough for an array of pointers to string (one bigger than string for terminating null) 
                                                            // returns a single pointer to the starting memory address of that array of pointers

    if(p_array == 0){   // malloc returns a null pointer when there isn't enough memory
        printf("No memory\n");
        exit(EXIT_FAILURE);
    }

    nstrings = 0;
    while(nstrings < MAXSTRING && (p_array[nstrings] = next_string()) != 0){    // assign a pointer to the staring address of the input character array to each index of p_array
                                                                                // I thougt that p_array was a pointer to the first element of the pointer array?
                                                                                // Now we're using parray as if it was an array itself?
                                                                                // If we use the address to the address to the start of the array (is that what is
                                                                                // being done here?), I guess that's equivalent to just using the array at the destination?
                                                                                // next_string returns a memory address (pointer) to the starting address of the character array
        nstrings++;
    }
    p_array[nstrings] = 0;

    sort_arr(p_array);
    print_arr(p_array);
    exit(EXIT_SUCCESS);
}

void print_arr(const char **p_array){
    while(*p_array) // while the contents of the index address of p_array that we are iterating through is not null
                    // why didn't we need to evaluate **p_array to use *p_array?
        printf("%s\n", *p_array++);
}

void sort_arr(const char **p_array){
    const char **lo_p, **hi_p, *tmp;

    for(lo_p = p_array; *lo_p != 0 && *(lo_p+1) != 0; lo_p++){
        for(hi_p = lo_p+1; *hi_p != 0; hi_p++){
            if(strcmp(*hi_p, *lo_p) >= 0)
                continue;
            tmp = *hi_p;
            *hi_p = *lo_p;
            *lo_p = tmp;
        }
    }
}

char *next_string(void){
    char *cp, *destination;
    int c;

    destination = (char *)malloc(MAXLEN);
    if(destination != 0){
        cp = destination;
        while((c = getchar()) != '\n' && c != EOF){
            *cp++ = c;
        }
        *cp = 0;
        if(c == EOF && cp == destination)
            return(0);
    }
    return(destination);
}