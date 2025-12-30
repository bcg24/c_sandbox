#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSTRING       50
#define MAXLEN          80

void print_arr(const char *p_array[]);  // a function that returns nothing and takes in an array of pointers to character constants
void sort_arr(const char *p_array[]);   // ditto
char *next_string(char *destination);   // a function that returns a character and takes in a pointer to a character as its argument

main(){
    // leave room for null at end
    char *p_array[MAXSTRING+1]; // create an array of pointers to char that is one bigger than the max number of strings
                                // stores a starting memory address to each one of the strings

    // storage for strings
    char strings[MAXSTRING][MAXLEN];    // create a character array of the size of the maximum number of strings, each containing an array of the maximum number of characters per string

    // count of strings read
    int nstrings;   // declare a counter

    nstrings = 0;   // initialize the counter
    while(nstrings < MAXSTRING &&   // while the counter is less than the maximum number of strings
        next_string(strings[nstrings]) != 0){   // and the next input doesn't return a null pointer, i.e. an input remains
                                                // implicitly taking &strings[nstrings][0] as its arg
            p_array[nstrings] = strings[nstrings];  // populate the pointer array with &strings[nstrings][0], i.e. the starting memory address of each one of the strings
            nstrings++; // increment the counter
    }
    // terminate p_array
    p_array[nstrings] = 0;  // once we either max out the string array or encounter an empty input

    sort_arr(p_array);
    printf("Sorted input: \n");
    print_arr(p_array);
    exit(EXIT_SUCCESS);
}

void print_arr(const char *p_array[]){  // returns nothing and takes an array of pointers to character constants as its argument
    int index;  // declare an int counter
    for(index = 0; p_array[index] != 0; index++)    // while the pointer in the current index is not the null pointer, print the string
        printf("%s\n", p_array[index]);
}

void sort_arr(const char *p_array[]){   // returns nothing by value and takes in an array of poiners to character constants
    int comp_val, low_index, hi_index;  // declare ints
    const char *tmp;    // declare pointer tmp to a character constant

    for(low_index = 0;  // start low index from 0
        p_array[low_index] != 0 &&  // as long as there's not a null pointer in the low index
            p_array[low_index+1] != 0;  // and there's not a null pointer in the next index (for the high index)
                low_index++){   // increment the low index
        for(hi_index = low_index+1; // initialize the high index one above the low index
            p_array[hi_index] != 0; // as long as the high index doesn't contain a null pointer
                hi_index++){    // increment the high index
            comp_val=strcmp(p_array[hi_index],  // compare the string (character array) at the low and high index, returns 0 if the same
                                                // returns positive if the high index string comes after the low index string
                p_array[low_index]);
            if(comp_val >= 0)   // if the strings are the same or the high index string comes after the low index string
                continue;   // increment the high index to the next char array/string
                            // Question: what would it look like for the high index string to come after? is this in an alphabetic sense?
            // swap strings
            tmp = p_array[hi_index];
            p_array[hi_index] = p_array[low_index];
            p_array[low_index] = tmp;
        }
    }
}

char *next_string(char *destination){   // a function that returns a pointer to a char, takes pointer to char as arg
    char *cp;   // declare a pointer to char
    int c;  // declare and int c (to hold extended characters)

    cp = destination;   // copy the destination pointer from the arg
    while((c = getchar()) != '\n' && c != EOF){ // while the input isn't a newline or EOF (Ctrl+D)
        if(cp-destination < MAXLEN-1)   // destination must be the starting memory address of an array
                                        // terminates when the input reaches one less than the maximum length (to save room for the null)
            *cp++ = c;  // set the destination array that is pointed to to c and post-increment the pointer
            // seems like there could be a break here, otherwise we'll keep collecting input but not storing it
    }
    *cp = 0;    // once we enter a newline or EOF character, set the next value to null
    if(c == EOF && cp == destination)   // if we encountered an EOF as the first char and didn't store anything, return null pointer
        return(0);
    return(destination);    // return the pointer to char
}