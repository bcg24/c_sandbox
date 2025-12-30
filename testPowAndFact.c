float pow(float, int);
int fact(int);

void main(){
    for(int i = 0; i < 10; i++){
        printf("power of 2^(%d): %f\n", i, pow(2.0, i));
        printf("fact of %d: %d\n", i, fact(i));
    }
}

float
pow(float x, int n){
    float result = 1;
    for(int i = 0; i < n; i++)
        result *= x;
    return(result);
}

int
fact(int n){
    int result = 1;
    for(int i = 1; i <= n; i++)
        result *= i;
    return(result);
}