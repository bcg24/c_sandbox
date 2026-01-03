struct x;   // incomplete type

// valid uses of the tag
struct x *p, func(void) ; // a pointer to a struct x, a function that takes nothing and returns a struct x

void f1(void){
    struct x{int i;};   // redeclaration! this is treated like a different struct
}

// full declaration now, this time not in a block
struct x{
    float f;
}s_x;   // giving this one a name s_x

void f2(void){
    // valid statements
    p = &s_x;   // initializing the pointer
    *p = func();    // setting the value of s_x by pointer
    s_x = func();   // setting the value of s_x directly
}

struct x
func(void){
    struct x tmp;   // declaring a new instance of struct x called tmp
    tmp.f = 0;  // setting the value of tmp's field
    return (tmp);
}