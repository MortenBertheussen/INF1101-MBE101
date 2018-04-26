#include "map.h"

const size_t START_SIZE = 20;


struct map_node;
typedef struct map_node map_node_t;

struct map_node
{
    size_t hashv;
    void *key;
    void *value;
};

struct map
{
    list_t **table;
    cmpfunc_t cmpfunc;
    hashfunc_t hashfunc;
    size_t max_elements;
    size_t cur_elements;
};





map_t *map_create(cmpfunc_t cmpfunc, hashfunc_t hashfunc)
{  
    map_t *map = (map_t*)calloc(1, sizeof(map_t));
    if(NULL == map)
    {
        char str[256];
        sprintf(str, "%s from file %s line number %d error: ENOMEM\n", __func__, __FILE__, __LINE__);
        fatal_error(str);
    } 
    list_t **table  = (list_t**)calloc(START_SIZE, sizeof(list_t*));
    if(NULL == table)
    {
        free(map);
        char str[256];
        sprintf(str, "%s from file %s line number %d error: ENOMEM\n", __func__, __FILE__, __LINE__);
        fatal_error(str);
    }
    *map = (map_t){.table = table, .cmpfunc = cmpfunc, .hashfunc = hashfunc, .max_elements = START_SIZE, .cur_elements = 0};     
    return map;
}

void map_destroy(map_t *map)
{
    if(NULL == map)
    {
        char str[256];
        sprintf(str, "%s from file %s line number %d error: ENOMEM\n", __func__, __FILE__, __LINE__);
        fatal_error(str);
    }
    for(int idx = 0; idx < map->max_elements; idx++)
    {
        if(NULL != map->table[idx])
        {
            list_destroy(map->table[idx]);
        }  
    }
    free(map);
}

void map_put(map_t *map, void *key, void *value)
{

}

int map_haskey(map_t *map, void *key)
{

}

void *map_get(map_t *map, void *key)
{
    
}
