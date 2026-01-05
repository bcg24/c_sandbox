#include <stdio.h>
#include <cblas.h>

extern char* openblas_get_corename(void);

int main(void) {
    printf("OpenBLAS detected CPU: %s\n", openblas_get_corename());
    return 0;
}