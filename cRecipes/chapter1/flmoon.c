#include <math.h>
#define PI (355.0/113.0)
#define RAD (PI/180.0)
/*
This routine calculates the phases of the moon. Given an integer nand
a code nphfor the phase desired (nph=0for new moon, 1 for first quarter, 2 for full, 3 for last
quarter), the routine returns the Julian Day Number jd, and the fractional part of a day frac
to be added to it, of the nthsuch phase since January, 1900. Greenwich Mean Time is assumed.
*/

void nerror(char error_text[]); // Declaring a function -- is this part of a standard library?

void flmoon(int n, int nph, long *jd, float *frac){ // takes a pointer to a long called jd
    int i;
    float am,as,c,t,t2,xtra;
    c=n+nph/4.0;
    t=c/1236.85;
    t2=t*t;
    as=359.2242+29.105356*c;
    am=306.0253+385.816918*c+0.010730*t2;
    *jd=2415020+28L*n+7L*nph;       // Populates the value of the variable for the julian day
    xtra=0.75933+1.53058868*c+((1.178e-4)-(1.55e-7)*t)*t2;
    if (nph == 0 || nph == 2)
        xtra += (0.1734-3.93e-4*t)*sin(RAD*as)-0.4068*sin(RAD*am);
    else if (nph == 1 || nph == 3)
        xtra += (0.1721-4.0e-4*t)*sin(RAD*as)-0.6280*sin(RAD*am);
    else nerror("nph is unknown in flmoon");
    i=(int)(xtra >= 0.0 ? floor(xtra) : ceil(xtra-1.0));    //casts as and int this expression -- where should I read in the C book about the ? and : operator?
    *jd += i;   // adds i to the value at jd -- how is this that different then just adding i to jd?
    *frac=xtra-i;
}