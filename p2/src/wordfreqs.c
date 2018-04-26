/* Author: Steffen Viken Valvaag <steffenv@cs.uit.no> */
#include "common.h"
#include "list.h"
#include "map.h"
#include "compare.h"

//https://gist.github.com/badboy/6267743
unsigned long hashv(void *a) 
{
    return 1;
}

int main(int argc, char **argv)
{
    map_t *map = map_create(compare_strings, hashv);
    map_destroy(map);


}
