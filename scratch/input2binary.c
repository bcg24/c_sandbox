#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
C doesn't have dynamic arrays, that is, the size of arrays should be known at compile time.
We can reallocate memory based on the size of the input to get arround this, but for now, let's
start with a static array of 8 bits. We know that the number of digits is not the same as the
number of bits, but we can fix this later.
*/

/*
Let's build an array from the user input using getchar(). We need to reject non-numeric inputs,
so lets make a stopping condition based on an alphabetical input or newline.
*/

#define ARRSIZE 8

int decimal2binary(int decimalRep);

int main(){
    int inputArray[ARRSIZE], binaryOutput;

    for(int ii = 0; ii < ARRSIZE; ii++){
        /*
        Equivalent to -1*ones(ARRSIZE)
        */

        inputArray[ii] = -1;
    }

    int stop = 0;
    for (int ii = 0; ii < ARRSIZE; ii++){
        char tempChar = getchar();
        int tempInt = tempChar - '0';
        if(stop == 1){
            break;
        }
        if(tempChar == '\n'){
            break;
        } else if((tempInt > -1) && (tempInt < 10)){
            inputArray[ii] = tempInt;
        } else {
            /*
            If the input minus '0' is not between 0 and 9, then it's not numeric
            */

            printf("Ignoring non-numeric input: %c \nTry again...\n", tempChar);

            /*
            Keep forcing the user to put in a numeric input, or terminate the whole thing if 
            a newline is entered.
            */
           
            int tempInt1 = -1;
            while ((tempInt1 < 0) || (tempInt1 > 9)){
                char tempChar1 = getchar();
                tempInt1 = tempChar1 - '0';
                if(tempChar1 == '\n'){
                    stop = 1;
                    break;
                }
                printf("Ignoring non-numeric input: %c \nTry again...\n", tempChar1);
            }
            inputArray[ii] = tempInt1;
        }
    }

    /*
    Build the decimal representation from the input array until we find a nan
    */

    int decimalRep = 0;

    for(int ii = 0; ii < ARRSIZE; ii++){
        if(inputArray[ii] == -1){
            break;
        }
        decimalRep = decimalRep*10 + inputArray[ii];
    }

    binaryOutput = decimal2binary(decimalRep);
    printf("Binary representation of %d is: %d", decimalRep, binaryOutput);
    exit(EXIT_SUCCESS);
}

int decimal2binary(int decimalRep){
    if(decimalRep == 0){
        return 0;
    }
    int highBitwiseOneIdx = floor(log2(decimalRep));
    int remainder = decimalRep - pow(2, highBitwiseOneIdx);

    int flippedBinaryBitArray[ARRSIZE];

    for(int ii = 0; ii < ARRSIZE; ii++){
        /*
        Equivalent to zeros(ARRSIZE)
        */

        flippedBinaryBitArray[ii] = 0;
    }

    flippedBinaryBitArray[highBitwiseOneIdx] = 1;

    while(remainder > 0){
        int nextHighestBitwiseOneIdx = floor(log2(remainder));
        flippedBinaryBitArray[nextHighestBitwiseOneIdx] = 1;
        remainder = remainder - pow(2, nextHighestBitwiseOneIdx);
    }

    /*
    flippedBinaryBitArray has the highest bit in the highest index, so we need to
    flip this around to build the decimal represenation of the binary bitwise
    values
    */

    int decimalRepBinary = 0;
    for(int ii = 0; ii < ARRSIZE; ii++){
        decimalRepBinary = decimalRepBinary + flippedBinaryBitArray[ii]*pow(10, ii);
    }
    
    return(decimalRepBinary);
}