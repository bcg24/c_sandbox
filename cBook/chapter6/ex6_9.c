#include <stdio.h>
#include <stdlib.h>

struct tree_node{
    int data;
    struct tree_node *left_p, *right_p;
};
void
t_walk(struct tree_node *root_p){

    if(root_p == 0)
        return;
    t_walk(root_p->left_p); // go all the way left plus one
                            // when it reaches the last node,
                            // it follows the null left pointer
                            // there, returning
    printf("%d\n", root_p->data);   // print the data at the left-most
                                    // upon return
    t_walk(root_p->right_p);    // there's no right pointer
                                // when we're at the end of a branch,
                                // so this returns to one level up of
                                // the recursion, which is the local root
                                // of the branch, e.g. 2
                                // Then we try to go all the way right,
                                // print 3, then hit another null pointer
}
/*
* Tree search algorithm
* Searches for value 'v' in tree,
* reutrns pointer to first node containing
* the value otherwise 0.
*/
struct tree_node *
t_search(struct tree_node *root, int v){
    
    while(root){
        printf("looking for %d, looking at %d\n", v, root->data);
        if(root->data == v)
            return(root);
        if(root->data > v)
            root = root->left_p;
        else
            root = root->right_p;
    }
    /* value not found, no tree left */
    return(0);
}
/*
* Insert node into tree.
* Return 0 for success,
* 1 for value already in tree,
* 2 for malloc error
*/
int
t_insert(struct tree_node **root, int v){   // takes a pointer to a pointer to the root node
                                            // this is so we can modify the pointer to the root node's address
    
    while(*root){   // while the pointer that root points to is not null
                    // evaluates the address in the left/right pointer field of the root node
                    // if the value stored at the left/right pointer is null, exit the loop
        if((*root)->data == v)  // if the data at the root node is the value we're inserting
            return(1);
        if((*root)->data > v)   // if the data at the root is greater than the value that we're inserting
            root = &((*root)->left_p);  // Then go into the root node and get the address of where the left pointer is stored
        else
            root = &((*root)->right_p);
    }  
    /* value not found, no tree left */
    if((*root = (struct tree_node *)malloc(sizeof(struct tree_node))) == 0) // if malloc returns a null pointer
                                                                            // the address to create the new node at 
                                                                            // didn't exist until this point
        return(2);
    (*root)->data = v;  // root is the address of the left/right pointer field in the previously created root node
                        // so by doing *root, we are going to the struct that the pointer in the field is pointing
                        // to and actually setting the values in that new node we allocated
    (*root)->left_p = 0;
    (*root)->right_p = 0;
    return(0);
}

main(){
    /* construct tree by hand */
    struct tree_node *tp, *root_p = 0;
    int i;

    /* we ignore the return value of t_insert */
    t_insert(&root_p, 4);
    t_insert(&root_p, 2);
    t_insert(&root_p, 6);
    t_insert(&root_p, 1);
    t_insert(&root_p, 3);
    t_insert(&root_p, 5);
    t_insert(&root_p, 7);

    /* try the search */
    for(i = 1; i < 9; i++){
        tp = t_search(root_p, i);
        if(tp)
            printf("%d found\n", i);
        else
            printf("%d not found\n", i);
    }
    t_walk(root_p);
    exit(EXIT_SUCCESS);
}