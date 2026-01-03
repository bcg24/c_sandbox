struct s_1; // incomplete type is here...

struct s_2{
    int something;
    struct s_1 *sp; // so we can use this...
};

struct s_1{ // before the full declaration here
    float something;
    struct s_2 *sp;
}