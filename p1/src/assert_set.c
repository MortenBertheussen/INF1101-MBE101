/* Author: Magnus Stenhaug <magnus.stenhaug@uit.no> */
#include "set.h"
#include <stdlib.h>

/* 
 * Parameters for the test case:
 * TEST_SEED_VALUE is the initial seed to randomly generate values
 * TEST_SET_SIZE is the number of elements to add to a test set
 * TEST_MODULUS is the modulus of the numbers generated and should
 * be larger than TEST_SET_SIZE to generate duplicates
 * TEST_PRINT_SET prints the sets
 * TEST_RUNS number of times a test is run per seed
 */
 
#define TEST_SEED_VALUE 0xabcd
#define TEST_SET_SIZE 50
#define TEST_MODULUS ( TEST_SET_SIZE )
#define TEST_PRINT_SET 1
#define TEST_RUNS 1

int compare_ints(void *a, void *b)
{
    int *ia = a;
    int *ib = b;

    return (*ia)-(*ib);
}

static void *newint(int i)
{
    int *p = malloc(sizeof(int));
    *p = i;
    return p;
}

/*
 * Generates a set from a seed value
 */

set_t *generate_set(unsigned int seed, int num)
{
	set_t *a;
	int i;
	
	a = set_create(compare_ints);
	/* Adding random numbers based on the seed value */
	for(i = 0; i < num; i++)
	{
		set_add(a, newint(rand_r(&seed) % TEST_MODULUS));
	}
	
	return a;
}

/*
 * Deletes a set
 */

void delete_generated_set(set_t *set)
{
	set_iter_t *iter;
	int *elem;
	
	/* Validate the result sets */
	iter = set_createiter(set);
	while(set_hasnext(iter))
	{
		elem = (int *)set_next(iter);
		free(elem);
	}

	set_destroyiter(iter);
	set_destroy(set);
}

/*
 * Checks the set integrity
 */
 
int check_set_integrity(set_t *set)
{
	set_iter_t *iter;
	int *curr, *prev, size;
	
	iter = set_createiter(set);
	if(iter == NULL)
	{
		printf("Iter returns an invalid address\n");
		return 0;
	}
	
	size = 0;
	if(set_hasnext(iter))
	{
		prev = set_next(iter);
		size++;
	
		/* Iterating through all values */
		while(set_hasnext(iter))
		{
			curr = set_next(iter);
			size++;
                // XXX: DUPLICATES SHOULD BE ALLOWED!			
			if(compare_ints(curr, prev) == 0)
			{
				printf("Duplicates in set\n");
				return 0;
			}
			else if(compare_ints(curr, prev) < 0)
			{
				printf("Set is not ordered\n");
				return 0;
			}
		}
	}
	
	/* Validating size */
	if(set_size(set) != size)
	{
		printf("Set size is invalid (is %d but iterated %d elements)", set_size(set), size);
		return 0;
	}
	
	set_destroyiter(iter);
	
	return 1;
}


/*
 * Prints a set
 */

void printset(char *name, set_t *a)
{
	set_iter_t *iter;
	int *elem;
	
	printf("%s:   [", name);
	
	/* Validate the result sets */
	iter = set_createiter(a);
	while(set_hasnext(iter))
	{
		elem = (int *)set_next(iter);
		printf("%d", *elem);
		if(set_hasnext(iter))
			printf(", ");
	}
	
	set_destroyiter(iter);

	printf("]\n");
}

/*
 * Ascertains that the set contains the values 
 * generated from a seed
 */

int assert_set(set_t *a, unsigned int seed, int num)
{
	int i, val;
	
	for(i = 0; i < num; i++)
	{
		val = rand_r(&seed) % TEST_MODULUS;
		if(!set_contains(a, &val))
		{
			return -1;
		}
	}
	
	return 0;
}

/*
 * Validates the constructs
 */

void validate_constructs()
{
	set_t *a;
	
	if((a = set_create(compare_ints)) == NULL)
	{
		fatal_error("set_create does not return a valid memory address");
	}
	
	set_destroy(a);
}

/*
 * Validates insertion
 */

void validate_insertion(unsigned int seed)
{
	set_t *a, *b;
	
	a = generate_set(seed, TEST_SET_SIZE);
	if(assert_set(a, seed, TEST_SET_SIZE))
	{
		fatal_error("Invalid set, check set_add and set_contains");
	}
	
	/* Make a copy and validate that a and b are the same */
	b = set_copy(a);
	if(assert_set(b, seed, TEST_SET_SIZE) || set_size(a) != set_size(b))
	{
		fatal_error("Invalid copy, check set_copy");
	}
	
	delete_generated_set(a);
	
	set_destroy(b);
}

/*
 * Validates insertion
 */

void validate_iterator(unsigned int seed)
{
	set_t *a;
	set_iter_t *iter;
	int *curr, *prev, size;
	char *errmsg;
	
	a = generate_set(seed, TEST_SET_SIZE);
	
	if(!check_set_integrity(a))
		fatal_error("iterator");
	
	delete_generated_set(a);
}

/*
 * Splits a set into two separate sets
 */

void split_set(set_t *dest, set_t *src_a, set_t *src_b)
{
	set_iter_t *iter;
	void *elem;
	int start, end, i;

	/* Splitting the sets into three regions */
	start = rand() % set_size(dest);
	end = start + (rand() % (set_size(dest) - start));
	
	/* Add these regions to a and b */
	iter = set_createiter(dest);
	for(i = 0; i < set_size(dest); i++)
	{
		elem = set_next(iter);
	
		/* Only in a */
		if(i < start)
		{
			set_add(src_a, elem);
		}
		/* In a and b */
		else if(i < end)
		{
			set_add(src_a, elem);
			set_add(src_b, elem);
		}
		/* Only in b */
		else
		{
			set_add(src_b, elem);
		}
	}
	set_destroyiter(iter);
	
	/* Swap a and b */
	if(rand() % 2)
	{
		set_t *tmp = src_a;
		src_a = src_b;
		src_b = tmp;
	}
}

void validate_set_operations(unsigned int seed)
{
	set_t *testset, *a, *b, *res_union, *res_inter, *res_diff;
	void *elem;
	set_iter_t *iter;

	/* Generate test set */
	testset = generate_set(seed, TEST_SET_SIZE);
	a = set_create(compare_ints);
	b = set_create(compare_ints);
	
	split_set(testset, a, b);
	
	/* Run the set operations */
	res_union = set_union(a, b);
	res_inter = set_intersection(a, b);
	res_diff = set_difference(a, b);
	
	if(!check_set_integrity(res_union))
		fatal_error("Union set is invalid");
	if(!check_set_integrity(res_inter))
		fatal_error("Intersection set is invalid");
	if(!check_set_integrity(res_diff))
		fatal_error("Difference set is invalid");
		
	if(TEST_PRINT_SET)
	{
		printset("\tfull set", testset);
		printset("\tset a", a);
		printset("\tset b", b);
		
		printset("\tunion", res_union);
		printset("\tinter", res_inter);
		printset("\tdiff", res_diff);
	}
	
	/* Validate the result sets */
	iter = set_createiter(testset);
	while(set_hasnext(iter))
	{
		elem = set_next(iter);
	
		/* Validating union (should contain all) */
		if(!set_contains(res_union, elem))
			fatal_error("Set union is not correct");
	
		/* Validating intersection (always in both)*/
		if(set_contains(a, elem) && set_contains(b, elem) && !set_contains(res_inter, elem))
			fatal_error("Set intersection is not correct");
		
		/* Validating difference (only in a) */
		if(set_contains(a, elem) && !set_contains(b, elem) && !set_contains(res_diff, elem))
			fatal_error("Set difference is not correct");
	}
	set_destroyiter(iter);
	
	/* Cleanup */
	set_destroy(res_diff);
	set_destroy(res_inter);
	set_destroy(res_union);
	
	delete_generated_set(testset);
}

int main(int argc, char **argv)
{
	int i;
	
	srand(1);
	
	printf("Running a series of tests to validate the set implementation:\n");

	/* Validating set create */
	printf("Validating set constructs...\n");
	validate_constructs();
	
	/* Validating set add and contains */
	printf("Validating set insertion and copy...\n");
	for(i = 0; i < TEST_RUNS; i++)
		validate_insertion(i);
	
	/* Validating iterator operations */
	printf("Validating set iterator...\n");
	for(i = 0; i < TEST_RUNS; i++)
		validate_iterator(i);
	
	/* Validating set operations */
	printf("Validating set operations...\n");
	for(i = 0; i < TEST_RUNS; i++)
		validate_set_operations(i);
	
	return 0;
}









