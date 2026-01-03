#include <stdio.h>
#include <stdlib.h>

#define ARSIZE  10

struct wp_char{
    char wp_cval;
    short wp_font;
    short wp_psize;
}ar[ARSIZE];

void infun(struct wp_char *);

main(){
    struct wp_char wp_tmp, *lo_indx, *hi_indx, *in_p;

    for (in_p = ar; in_p < &ar[ARSIZE]; in_p++){    // usual array collapse; up to last index + 1 address; increment
        infun(in_p);    // pass pointer to a single element of the array of structs to infun
        if(in_p->wp_cval == '\n'){  // same as before
            /*
            * Leave the loop.
            * not incrementing in_p means that the
            * '\n' is ignored in the sort
            */
           break;   // becaue we exit the loop this way, in_p is not incremented
        }
    }

    /*
    * Now a simple exchange sort.
    * We must be careful to avoid the danger of pointer underflow,
    * so check that there are at least two entries to sort.
    */

    if(in_p-ar > 1) // ar collapses to first address, in_p is the index of the last valid struct plus one, so this ensures that there
                    // are at least 2 indices to sort: one at 0 and one at 1, wherein in_p would be at 2 (which must have, in this case,
                    // had a newline there that got ignored)
        for(lo_indx = ar; lo_indx <= in_p-2; lo_indx++){
            for(hi_indx = lo_indx+1; hi_indx <= in_p-1; hi_indx++){ // remember that we're allowed to use one past the address of the
                                                                    // last element, so this is unconditionally safe from overflow
                                                                    // even if hi_index is inncremented after the loop body to in_p
                                                                    // becasue we're not trying to evaluate the value there --
                                                                    // the loop condition would fail first.
                if(lo_indx->wp_cval > hi_indx->wp_cval){
                    /*
                    * Swap the structures.
                    */
                    struct wp_char wp_tmp = *lo_indx;
                    *lo_indx = *hi_indx;
                    *hi_indx=  wp_tmp;
                }
            }
        }

    /* now print */
    for(lo_indx = ar; lo_indx < in_p; lo_indx++){
        printf("%c %d %d\n", lo_indx->wp_cval, lo_indx->wp_font, lo_indx->wp_psize);
    }
    exit(EXIT_SUCCESS);
}

void
infun( struct wp_char *inp){
    inp->wp_cval = getchar();   // write the standard input to field wp_cval
    inp->wp_font = 2;
    inp->wp_psize = 10;

    return;
}