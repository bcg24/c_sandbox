#include <stdio.h>
#include <stdlib.h>

main(){
    int x,y;
    x = 0; y = ~0;

    while(x != y){
        printf("%x & %x = %x\n", x, 0xff, x&0xff);      //0 & 1111 1111 = 0 = 0
        printf("%x | %x = %x\n", x, 0x10f, x|0x10f);    //0 | 0001 0000 1111 = 0001 0000 1111 = 1 0 F
        printf("%x ^ %x = %x\n", x, 0xf00f, x^0xf00f);  //0 ^ 1111 0000 0000 1111 = 1111 0000 0000 1111 = F 0 0 F
        printf("%x >> 2 = %x\n", x, x >> 2);            //0 >> 2 = 0
        printf("%x << 2 = %x\n", x, x << 2);            //0 << 2 = 0
        x = (x << 1) | 1;                               // Starts with 0, then 0 | 1, which gives 1, then 10 | 01, which gives 11, 111, etc.
    }
    exit(EXIT_SUCCESS);
}