#include "list.h"
#include "set.h"
#include "common.h"
#include <sys/time.h>
#include <stdlib.h>

/* Author: Aage Kvalnes <aage@cs.uit.no> */
unsigned long long gettime(void)
{
    struct timeval tp;
    unsigned long long ctime;

    gettimeofday(&tp, NULL);
    ctime = tp.tv_sec;
    ctime *= 1000000;
    ctime += tp.tv_usec;

    return ctime;
}

/* Author: Steffen Viken Valvaag <steffenv@cs.uit.no> */
static int compare_ints(void *a, void *b)
{
    int *ia = a;
    int *ib = b;

    return (*ia)-(*ib);
}

/* Author: Steffen Viken Valvaag <steffenv@cs.uit.no> */
static void *newint(int i)
{
    int *p = malloc(sizeof(int));
    *p = i;
    return p;
}


int main(int argc, char **argv)
{
    int i, j;
    /*BUG: Causes memory leak*/
    set_t *set =set_create(compare_ints);
    unsigned long long t1, t2, t3;
    int *number;
#pragma region add
    printf("Random add\n");
    for (j = 0; j < 10; j++)
    {
        t1 = gettime();
        for (i = 0; i <= 50*j ; i++)
        {
            number = newint(rand());
            set_add(set, number);
        }
        t1 = gettime() - t1;
        printf("%d,%lld\n",i-1, t1);        
    }
    
    set_destroy(set);
    set = set_create(compare_ints);
    puts("");

    printf("Reverse stair add\n");    
    for (j = 0; j < 10; j++)
    {
        t1 = gettime();
        for (i = 0; i <= 50*j ; i++)
        {
            number = newint(i);
            set_add(set, number);
        }
        t1 = gettime() - t1;
        printf("%d,%lld\n",i-1, t1);        
    }

    set_destroy(set);
    set = set_create(compare_ints);
    puts("");

    printf("Constant add\n");        
    for (j = 0; j < 10; j++)
    {
        t1 = gettime();
        for (i = 0; i <= 50*j ; i++)
        {
            number = newint(1337);
            set_add(set, number);
        }
        t1 = gettime() - t1;
        printf("%d,%lld\n",i-1, t1);                   
    }
#pragma endregion add

    set_t *left = set_create(compare_ints);
    set_t *right = set_create(compare_ints);
    
    for (i = 0; i <= 50 ; i++)
    {
        set_add(left, newint(rand()));
        set_add(right, newint(rand()));
    }
    printf("Constant union/intersection/difference\n");        
    for (j = 0; j < 10; j++)
    {

        t1 = gettime();
        for (i = 0; i <= 50*j ; i++)
        {
            set_union(left, right);
        }
        t1 = gettime() - t1;
        printf("%d,%lld\n",i-1, t1);                   
    }

}