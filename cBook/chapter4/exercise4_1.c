#include <stdio.h>
#include <stdlib.h>

main(){
    int abs_val(int);
    int input_checker(int);
    int form_inp(void);
    int val = 0;
    val = form_inp();
    printf("input: %d\n", val);
    printf("result: %d\n", abs_val(val));
    exit(EXIT_SUCCESS);
}

int
abs_val(int val){
    if(val < 0)
        val *= -1;
    return(val);
}

int
input_checker(int val){
    int out = 0;
    if(val >= '0' && val <= '9')
        out = 1;
    if(val == '-')
        out = -1;
    return(out);
}
    
int
form_inp(void){
    int c;
    int result = 0, i = 0;
    int neg = 1;
    while((c = getchar()) != '\n'){
        switch(input_checker(c)){
            case 1:
                result = 10*result + (c - '0');
                i = i + 1;
                break;
            case 0:
                continue;
            case -1:
                if(i == 0){
                    neg = -1;
                    i = i + 1;
                } else
                    continue;
        }
    }
    return(result*neg);
}