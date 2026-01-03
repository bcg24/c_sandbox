#include <stdio.h>
#include <stdlib.h>

struct list_ele{
    int data;
    struct list_ele *pointer;
}ar[3];

main(){
    
    struct list_ele *lp;
    
    ar[0].data = 5;
    ar[0].pointer = &ar[1];
    ar[1].data = 99;
    ar[1].pointer = &ar[2];
    ar[2].data = -7;
    ar[2].pointer = 0;  // to mark the end of the list
    
    // follow pointers
    lp = ar;    // collapses to first array element address
    while(lp){  // keeps going until the terminating null pointer is reached
        printf("contents %d\n", lp->data);
        lp = lp->pointer;   // assigns the pointer to the pointer of the next element by pointer (lol)
    }
    exit(EXIT_SUCCESS);
}