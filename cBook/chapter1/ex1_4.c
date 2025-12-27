#include <stdio.h>
#include <stdlib.h>
#define ARSIZE 10
int main(){
    int chArr[ARSIZE],count1;
    int count2, stop, lastChar;

    lastChar = 0;
    stop = 0;
    while(stop == 0){
        chArr[lastChar] = getchar();
        if(chArr[lastChar] == '\n'){
            stop = 1;
        }
        else
            printf("Last character: %c at index: %d\n", chArr[lastChar], lastChar);
            lastChar = lastChar + 1;
        if(lastChar == ARSIZE)
            stop = 1;
    }
    lastChar = lastChar - 1;

    /*
    * Bubble sort
    */
   count1 = 0;
   while(count1 < lastChar){
        count2 = count1 + 1;
        while(count2 <= lastChar){
            if(chArr[count1] > chArr[count2]){
                /* swap */
                int temp;
                temp = chArr[count1];
                chArr[count1] = chArr[count2];
                chArr[count2] = temp;
            }
            count2 = count2 + 1;
        }
        count1 = count1 + 1;
    }

    count1 = 0;
    while(count1 <= lastChar){
        printf("%c\n", chArr[count1]);
        count1 = count1 + 1;
    }
    exit(EXIT_SUCCESS);
}
