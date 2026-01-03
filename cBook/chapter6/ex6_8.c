#include <stdio.h>
#include <stdlib.h>
struct tree_node{
    int data;
    struct tree_node *left_p, *right_p;
}tree[7];   // create an array called tree of 7 tree nodes
/*
* Tree search algorithm.
* Searches for a value 'v' in tree,
* returns pointer to first node found containing
* the value 0 otherwise.
*/
struct tree_node *  // returns a pointer to the type struct tree_node
t_search(struct tree_node *root, int v){

    while(root){
        if(root->data == v)
            return(root);   // return the address of the search root if its value is equal to the search value
        if(root->data > v)  // if the value at the root is greate than the earch value
            root = root->left_p;    // move the root of our search to the left pointer
        else
            root = root->right_p;   // move the root "                  " right pointer
    }
    /* value not found, no tree left */
    return(0);
}

main(){
    /* construct tree by hand */
    struct tree_node *tp, *root_p;
    int i;
    for(i = 0; i < 7; i++){ // iterate throught the size of the tree node array
        int j;
        j = i+1;    // intiailize the j counter to always be one ahead of the i counter (values 1:7 for indices 0:6)

        tree[i].data = j;   // set the tree array element to the corresponding value
        if(j == 2 || j == 6){   // 2 and 6 are the values of the nodes to the left and right of the root
            tree[i].left_p = &tree[i-1];    // this just creates the branches for values 1 and 3 and 5 and 7
            tree[i].right_p = &tree[i+1];
        }
    }
    /* root */
    root_p = &tree[3];  // make the root the element corresponding to value 4
    root_p->left_p = &tree[1];  // assign the pointers to the left and right elements of the root
    root_p->right_p = &tree[5];

    /* try the search */
    tp = t_search(root_p, 1);
    if(tp)
        printf("found at position %d\n", tp-tree);
    else
        printf("value not found\n");
    exit(EXIT_SUCCESS);
}