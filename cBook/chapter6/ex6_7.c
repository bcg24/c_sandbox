struct list_ele *
sort( struct list_ele *list)    // a function that takes a pointer to a type struct list_ele
                                // named list as its argument and returns a pointer to the
                                // same type of pointer
{
    int exchange;
    struct list_ele *nextp, *thisp, dummy;  // dummy is an instance of the struct that we are sorting

    /*
    * Algorithm is this:
    * Repeatedly scan list.
    * If two list items are out of order,
    * link them in the other way round.
    * Stop if a full pass is made and no
    * exchanges are required.
    * The whole business is confused by
    * working one element behind the
    * first one of interest.
    * This is because of the simple mechanics of
    * linking and unlinking elements.
    */

    dummy.pointer = list;   // list is almost certainly the first element address of the linked list
                            // initializing the pointer 'field' of the struct -- data is still uninitialized

    do{
        exchange = 0;   
        thisp = &dummy; // So dummy.pointer points to the first struct element of the list,
                        // and thisp points to the dummy struct
        while( (nextp = thisp->pointer) && nextp->pointer){ // so nextp is accessing the pointer field of dummy through the pointer thisp
                                                            // which would point it to the next element, in this case the first element of list
                                                            // Then we are accessing the pointer of the second element of the list through
                                                            // the pointer in next p
                                                            // So what this does is only continues when both the address of the next element
                                                            // and the next-next element are not null
            if(nextp->data < nextp->pointer->data){ // if the data in the next element is less than the data in the next-next element
                /* exchange */
                exchange = 1;
                thisp->pointer = nextp->pointer;    // make the pointer field of the current element (dummy on the first iteration)
                                                    // equal to the address of the next-next element
                nextp->pointer = thisp->pointer->pointer;   // make the pointer field of the next element equal to the
                                                            // adress of the next-next-next element
                thisp->pointer->pointer = nextp;    // point the next-next element to the next element
            }
            thisp = thisp->pointer; // increment the pointer to the next element
                                    // if the index order was 0 (dummy), 1, 2, 3, what this all did was make it
                                    // 0, 2, 1, 3
        }
    }while(exchange);

    // Overall, the flow of the algorith is:
    // go all the way through, reordering the next element and next next element to put the higher value nearer dummy
    // do this until the next-next element is 0 (we've reached the end of the list)
    // reset and go back to the start of the list
    // Go throught the entire list again, looking for elements that are out of order
    // if none are out of order, exchange stays 0 and the second loop exits

    return(dummy.pointer); // dummy.pointer returns the address of the first element of the sorted list
}