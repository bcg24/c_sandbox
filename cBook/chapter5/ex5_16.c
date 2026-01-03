#include <stdio.h>
#include <stdlib.h>

void func(int); // function prototype

main(){
    void (*fp)(int);    // so this looks like it'd be a pointer
                        // to a function that takes an int as its arg
                        // and returns void

    fp = func;  // pointer and function are the same due to the decay to function address

    (*fp)(1);   // This seems to be calling the function by pointer
    fp(2);  // This is equivalent due to the decay to function address

    exit(EXIT_SUCCESS);
}

void
func(int arg){
    printf("%d\n", arg);
}