#include <stdio.h>
#define PI (22.0/7.0)

/*
* Tell the compiler that we intend
* to use a function called showMessage.
* It has no arguments and returns no value
* This is the "declaration".
*/

void showMessage(int count);

/*
* Another function, but this includes the body of
* the function. This is a "definition".
*/
int main(){
    int count;

    count = 0;
    while(count < 10){
        showMessage(count);
        count = count + 1;
    }   

    return(0);
}

/*
* The body of the simple funtion.
* This is now a "definition".
*/
void showMessage(count){
    printf("%f\n", PI*count);
}