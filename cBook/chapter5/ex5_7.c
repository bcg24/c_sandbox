// Compare two strings for equality
// Returns 'false' if they are
int
str_eq(const char *s1, const char *s2){
    while(*s1 == *s2){
        // At the end of string, return 0
        if(*s1 == 0)    // if the last read character is the null terminator
            return(0);
        s1++; s2++;
    }
    // Difference detected!
    return(1);
}