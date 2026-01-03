int streq(char s1, char s2){
    char *ps1, *ps2;
    unsigned long sz1 = sizeof(s1), sz2 = sizeof(s2);
    for(ps1 = s1, ps2 = s2; ps1 < &s1[sz1] && ps2 < &s2[sz2]; ps1++, ps2++)
        if(*ps1 != *ps2)
            return((int)*ps1 - (int)*ps2);
        if(ps1 == 0)
            return(0);
}