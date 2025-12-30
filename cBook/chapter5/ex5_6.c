#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINELNG 100                                     // max length of input line

main(){
    char in_line[LINELNG];                              // character array of length LINELNG, uninitiallized
    char *cp;                                           // pointer called cp to char, uninitialized
    int c;                                              // int c, uninitiallized

    cp = in_line;                                       // effectively set the pointer to &in_line[0]
    while((c = getc(stdin)) != EOF){                    // haven't seen getc yet, but assuming its kinda like getchar, the loop
                                                        // will stop when the input (is that what stdin is?) == Ctrl+D
        if(cp == &in_line[LINELNG-1] || c == '\n'){     // when the pointer reaches the address of the last element
                                                        // or the read character is newline
            // Insert end-of-line marker
            *cp = 0;                                    // then set the value at cp = 0
            if(strcmp(in_line, "stop") == 0 )           // assuming this works like matlab, if in_line is not 'stop'
                exit(EXIT_SUCCESS);                     // exit the program
            else
                printf("line was %d characters long\n", // otherwise, print how many characters we typed
                    (int)(cp-in_line));                 // looking at line 28, cp gets incremented, while
                                                        // in_line stays at the first element, so this gives
                                                        // the difference
            cp = in_line;                               // move cp back to the begining of the array
        }
        else
            *cp++ = c;                                  // while (*cp)++ would increment the value pointed to by cp,
                                                        // in this case, we are storing c at the memory address pointed
                                                        // to by cp, then incrementing the address to the next element
    }
    exit(EXIT_SUCCESS);
}