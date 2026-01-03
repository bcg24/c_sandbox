#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GROW_BY 5      // string grows by 10 chars

main(){
    char *str_p, *next_p, *tmp_p;   // declaring pointers to character
    int ch, need, chars_read;

    if(GROW_BY < 2){
        fprintf(stderr, "Growth constant too small\n");
        exit(EXIT_FAILURE);
    }

    str_p = (char *)malloc(GROW_BY);    // allocate space for 2 bytes and cast to pointer to char
    if(str_p == NULL){
        fprintf(stderr,"No intial store\n");
        exit(EXIT_FAILURE);
    }

    next_p = str_p; // initialize the next address as the memory address that we just allocated
    chars_read = 0; // intialize counter
    while((ch = getchar()) != EOF){
        /*
        * Completely restart at each new line.
        * There will always be room for the
        * terminating zero in the string,
        * because of the check further down,
        * unless GROW_BY is less than 2,
        * and that has already been checked.
        */
        if(ch == '\n'){
            *next_p = 0;    // if we see a newline, then null terminate it
            printf("%s\n", str_p);  // remember that a pointer to the first element of a character array is the same as a string
            free(str_p);    // free the memory for the string once it is printed
            chars_read = 0; // reinitiallize the counter
            str_p = (char *)malloc(GROW_BY);    // allocate a new batch of memory same as before
            if(str_p == NULL) {
                fprintf(stderr,"No intial store\n");
                exit(EXIT_FAILURE);
            }
            next_p = str_p;
            continue;   // go to the next loop iteration
        }
        /*
        * Have we eache the end of the current
        * allocation?
        */
        if(chars_read == GROW_BY-1){    // if we reached the length of the allocated memory (minus one for a null terminator)
            *next_p = 0;    // mark end of string
            /*
            * use pointer subtraction
            * to find length of
            * current string.
            */
            need = next_p - str_p +1;   // str_p stayed at the first memory address, so this expresion gives the difference 
                                        // plus one, which is the length of the string that we stored so far with one added for the null
            tmp_p = (char *)malloc(need+GROW_BY);   // allocate memory for the existing string plus the growth constant
            if(tmp_p == NULL){
                fprintf(stderr,"No more store\n");
                exit(EXIT_FAILURE);
            }
            // Copy the string using library
            strcpy(tmp_p, str_p);  
            free(str_p);    // free the old array
            str_p = tmp_p;  // overwrite the old address with the new, enlarged memory array
            // and reset next_p, charcter count
            next_p = str_p + need-1;
            chars_read = 0;
        }
        // Put chaacter at end of current string
        *next_p++ = ch; // access via pointer and post increment pointer
                        // writes the input character to the string that we allocated in memory
        chars_read++;   // increment the counter
    }
    // EOF - but do unprinted characters exist?
    if(str_p - next_p){
        *next_p = 0;
        fprintf(stderr,"Incomplete last line\n");
        printf("%s\n", str_p);
    }
    exit(EXIT_SUCCESS);
}