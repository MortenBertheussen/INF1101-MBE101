#include "compare.h"

void swap(void *a, void*b)
{
    void *tmp = a;
    a = b;
    b = tmp;    
}

int8_t _stringcmp(void *a, void *b)
{ 
    int8_t r;
    if((r = strcmp(a,b)) == 0)
        return 0;
    else if(r > 0)
        return 1;
    else
        return -1;
}

int8_t _numcmp(void *a, void *b)
{
    int8_t r;
    if((r = (*(double*)a - *(double*)b)) == 0)
        return 0;
    else if(r > 0)
        return 1;
    else
        return -1;
}
 