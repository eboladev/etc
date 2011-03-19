#include <stdlib.h>
#include <stdio.h>

#define SIZE (60L*1024L*1024L)

int main(int argc, char **argv)
{
    unsigned long i;
    char *a = malloc(SIZE);
    if (a)
        for (i = 0; i < SIZE; i++) 
            a[i] = 1;
    return 0;
}
