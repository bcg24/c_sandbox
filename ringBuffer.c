/*
Let's make a simple ring buffer implementation. 

What it's going to do is take in a user input,
and then break up the input into substrings
by using the space as a delimiter -- we can
use strrtok for this. It's going to go through
each substring and store it in
buffer at the location of the write pointer 
and advance the write pointer by the length of the substring.
We'll choose to replace the space with a '\0'
as a separator between the substrings.
It'd be easier to add each letter to the 
buffer letter-by-letter, but we want to 
practice reading and writing arrays of data
to and from the buffer

If enter is pressed, then what it's going
to do is print out the substring at the read
pointer up until the null separator.

This is kinda the behaviour that you might
want in say, an audio sequencer -- records an
array of samples up until a key is pressed,
then returns (part of) what was recorded when another
key is pressed.

One thing I'm a bit curious about is the mechanics
of getchar. So far, what I've noticed is that
it doesn't seem that the input is passed to
the program until enter is pressed. But then,
the input is processed letter by letter. 
Is there a dynamic array behind the scenes
of getchar that allocates an array big enough
to store the whole input?

So what's going on is that the OS is the one
with the buffer for the input, and the 
program is reading from that buffer 
character-by-character.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARR_SZ 4096
enum{ N = ARR_SZ };

/*
One of the idiosynchrocies of C
is that a const is somehow not a
compile time constant, and define
is obviously just a purely textual
pre-compiler directive, so that
evaluates to a compile-time
constant, but one that you
can't debug. Therefore, you
might choose to use an enum
instead
*/

int main(){
    char *token, *pRestInp, *pRestRing, *pRead, *pWrite, inpBuff[N], ringBuff[N];
    pRead = pWrite = ringBuff;
    int running = 1;
    while(running){
        fgets(inpBuff, N, stdin);
        if(inpBuff[0] == '\n'){                             // If the only thing that was input is 'Enter'
            if(strcmp(pRead, "stop") == 0) running = 0;
            printf("%s", pRead);                            // Print from the ring buffer
                                                            // If the string is split across the end and the
                                                            // beginning of the ring buffer, I think this breaks
                                                            // Maybe it's best not to split a string between
                                                            // the end and beginning for this reason
            pRead = ringBuff + ((pRead - ringBuff) + strlen(pRead) + 1) % N;
        } else {
            inpBuff[strcspn(inpBuff, "\n")] = '\0';         // strcspn returns the number of characters before
                                                            // the first occurrence of the specified character
            pRestInp = inpBuff;
            while(token = strtok_s(pRestInp, " ", &pRestInp)){    // While there's a non-null token left
                int idxsLeft = (&ringBuff[N] - pWrite);      // Calculate the number of unfilled idxs left to end
                int numChars = strlen(token) + 1;
                if(numChars > idxsLeft){               // If the token would overflow
                    memcpy(pWrite, token, idxsLeft);        // Copy the first half to the end of the buffer
                    pWrite = ringBuff;                      // Reset the write pointer to the start
                                                            // Don't think we can use modulo for write pointer
                                                            // update becasue we have to spilt the copies
                    memcpy(pWrite, &token[idxsLeft], numChars - idxsLeft);    
                    pWrite += (numChars - idxsLeft);
                } else{
                    // This logic works as long as the token isn't going to be
                    // split across the end and beginning
                    memcpy(pWrite, token, numChars);
                    pWrite += numChars;
                }
            }
        }
    }
    exit(EXIT_SUCCESS);
}