/*
* Program that generates primes
*/
#include <stdio.h>
#include <stdlib.h>

main(){
    int thisNumber, divisor, notPrime;
    int previousPrime, difference;

    thisNumber = 3;
    previousPrime = 3;
    difference = 0;

    while(thisNumber < 10000){
        divisor = thisNumber / 2;
        notPrime = 0;
        while(divisor > 1){
            if(thisNumber % divisor == 0){
                notPrime = 1;
                divisor = 0;
            }
            else
                divisor = divisor - 1;
        }

        if(notPrime == 0){
            difference = thisNumber - previousPrime;
            if(difference == 2)
                printf("%d and %d are a prime pair\n", previousPrime, thisNumber);
            previousPrime = thisNumber;
            difference = 0;
        }
        thisNumber = thisNumber + 1;
        
    }
    exit(EXIT_SUCCESS);
}