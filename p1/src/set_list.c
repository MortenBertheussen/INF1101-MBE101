/* Author: Morten Bertheussen <mbe101@post.uit.no> */
#include "set.h"
#include "list.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#define DEBUG 1

struct set{
    list_t *list;
    cmpfunc_t cmpfunc;
};

struct set_iter{
    list_iter_t *iter;
};

/*
 * Creates a new set using the given comparison function
 * to compare elements of the set.
 */
set_t *set_create(cmpfunc_t cmpfunc){
    list_t *list = list_create(cmpfunc);
    set_t *set = malloc(sizeof(set_t));
    if(list == NULL || set == NULL){
        fatal_error("OOM: set_create()");
    }
    set->list = list;
    set->cmpfunc = cmpfunc;
    return set;
}

/*
 * Destroys the given set.  Subsequently accessing the set
 * will lead to undefined behavior.
 */
void set_destroy(set_t *set){
    if(set == NULL){
        fatal_error("set NULL: set_destroy()");
    }
    list_destroy(set->list);
    free(set);
}

/*
 * Returns the size (cardinality) of the given set.
 */
int set_size(set_t *set){
    if(set == NULL){
        fatal_error("set NULL: set_size()");        
    }
    return list_size(set->list);
}

/*
 * Adds the given element to the given set.
 */
void set_add(set_t *set, void *elem){
    if(set == NULL){
        fatal_error("set NULL: set_add()");
    }
    if(list_contains(set->list, elem))
        return;
    list_addfirst(set->list, elem);
    list_sort(set->list);
}

/*
 * Returns 1 if the given element is contained in
 * the given set, 0 otherwise.
 */
int set_contains(set_t *set, void *elem){
    if(set == NULL){
        fatal_error("set NULL: set_contains()");
    }
    return list_contains(set->list, elem);
}

/*
 * Returns the union of the two given sets; the returned
 * set contains all elements that are contained in either
 * a or b.
 */
set_t *set_union(set_t *a, set_t *b){
    set_t *set = set_create(a->cmpfunc);
    set_iter_t *itera = set_createiter(a);
    set_iter_t *iterb = set_createiter(b);

    while(set_hasnext(itera)){
        set_add(set, set_next(itera));
    }
    while(set_hasnext(iterb)){
        set_add(set, set_next(iterb));
    }
    set_destroyiter(itera);
    set_destroyiter(iterb);
    return set;
}

/*
 * Returns the intersection of the two given sets; the
 * returned set contains all elements that are contained
 * in both a and b.
 */
set_t *set_intersection(set_t *a, set_t *b){
    if( a == NULL || b == NULL){
        fatal_error("a||b NULL: set_intersection()");
    }
    set_iter_t *itera = set_createiter(a);
    set_t *set = set_create(a->cmpfunc);
    void *ptr = NULL;
    while(set_hasnext(itera)){
        if(set_contains(b, ptr = set_next(itera))){
            set_add(set, ptr);
        }
    }
    set_destroyiter(itera);    
    return set;
}

/*
 * Returns the set difference of the two given sets; the
 * returned set contains all elements that are contained
 * in a and not in b.
 */
set_t *set_difference(set_t *a, set_t *b){
    if( a == NULL || b == NULL){
        fatal_error("a||b NULL: set_intersection()");
    }
    set_iter_t *itera = set_createiter(a);
    set_t *set = set_create(a->cmpfunc);
    void *ptr = NULL;
    while(set_hasnext(itera)){
        ptr = set_next(itera);
        if(!set_contains(b, ptr)){
            set_add(set, ptr);
        }
    }
    set_destroyiter(itera);
    return set;
}

/*
 * Returns a copy of the given set.
 */
set_t *set_copy(set_t *set){
    if(set == NULL){
        fatal_error("set NULL: set_copy()");
    }
    set_t *cpy = set_create(set->cmpfunc);
    list_t *list = list_create(set->cmpfunc);
    list_iter_t *iter = list_createiter(set->list);

    while(list_hasnext(iter)){
        list_addlast(list, list_next(iter));
    }
    cpy->list = list;
    list_destroyiter(iter);
    return cpy;
}

/*
 * Creates a new set iterator for iterating over the given set.
 */
set_iter_t *set_createiter(set_t *set){
    if(set == NULL){
        fatal_error("set NULL: set_createiter()");
    }    
    set_iter_t *iter = malloc(sizeof(set_iter_t));
    iter->iter = list_createiter(set->list);
    return iter;
}

/*
 * Destroys the given set iterator.
 */
void set_destroyiter(set_iter_t *iter){
    if(iter == NULL){
        fatal_error("iter NULL: set_destroyiter()");
    }
    list_destroyiter(iter->iter);
    free(iter);
}

/*
 * Returns 0 if the given set iterator has reached the end of the
 * set, or 1 otherwise.
 */
int set_hasnext(set_iter_t *iter){
    if(iter == NULL){
        fatal_error("iter NULL: set_hasnext()");
    }
    return list_hasnext(iter->iter);
}

/*
 * Returns the next element in the sequence represented by the given
 * set iterator.
 */
void *set_next(set_iter_t *iter){
    if(iter == NULL){
        fatal_error("iter NULL: set_next()");
    }
    return list_next(iter->iter);
}
