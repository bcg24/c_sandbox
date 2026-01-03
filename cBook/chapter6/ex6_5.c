struct list_ele{
    int data;
    struct list_ele *pointer;   // pointer to type struct list_ele (type of itself, legal)
}ar[3]; // declares an array of 3 structures

main(){
    ar[0].data = 5;
    ar[0].pointer = &ar[1];
    ar[1].data = 99;
    ar[1].pointer = &ar[2];
    ar[2].data = -7;
    ar[2].pointer = 0;  // to mark the end of the list
    return(0);
}