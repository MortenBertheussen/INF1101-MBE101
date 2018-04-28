#include "index.h"

#define ERROR_MSG "%s : %s(), at line: %d", __FILE__, __func__, __LINE__

struct index
{
    map_t *map;
    char *current;
    double doc_count;
};

typedef struct data
{
    char *path;
    double term_in_document;
    double terms_in_document;
} data_t;

set_t *parse_query(index_t *index, list_t *query, char **errmsg);
set_t *parse_andterm(index_t *index, list_t *query, char **errmsg);
set_t *parse_orterm(index_t *index, list_t *query, char **errmsg);
set_t *parse_term(index_t *index, list_t *query, char **errmsg);
set_t *parse_andnot(index_t *index, list_t *query, char **errmsg);

data_t *data_create()
{
    data_t *d = calloc(1, sizeof(data_t));
    if (d == NULL)
    {
        fatal_error(ERROR_MSG);
    }
    return d;
}

void data_destroy(data_t *d)
{
    if (NULL != d)
    {
        if (NULL != d->path)
        {
            free(d->path);
        }
        free(d);
    }
}

int compare_data(void *a, void *b)
{
    return compare_strings(((data_t *)a)->path, ((data_t *)b)->path);
}

/*
 * Creates a new, empty index.
 */
index_t *index_create()
{
    index_t *index = calloc(1, sizeof(index_t));
    if (index == NULL)
    {
        fatal_error(ERROR_MSG);
    }
    index->map = map_create(compare_strings, hash_string);
    return index;
}

/*
 * Destroys the given index.  Subsequently accessing the index will
 * lead to undefined behavior.
 */
void index_destroy(index_t *index)
{
    //TODO: Potential bug here (may need specialized free functions)
    map_destroy(index->map, free, (void *)set_destroy);
    free(index);
}

/*
 * Adds the given path to the given index, and index the given
 * list of words under that path.
 * NOTE: It is the responsibility of index_addpath() to deallocate (free)
 *       'path' and the contents of the 'words' list.
 */
void index_addpath(index_t *index, char *path, list_t *words)
{
    // Save the size of the 'words' list before popping content of it.
    int document_word_count = list_size(words);

    while (0 < list_size(words))
    {
        // Popping content of the word list such that the function comply with given instructions.
        char *current_word = list_popfirst(words);

        // 'Current_word' found in map.
        if (1 == map_haskey(index->map, current_word))
        {
            // Find the set containing 'current_word'.
            set_t *set = map_get(index->map, current_word);

            // Set contains 'path',
            if (1 == set_contains(set, &(data_t){.path = path}))
            {
                // iterate over set to find data so we can manipulate it
                set_iter_t *set_iter = set_createiter(set);
                while (set_hasnext(set_iter))
                {
                    data_t *data = set_next(set_iter);
                    if (0 == compare_strings(data->path, path))
                    {
                        data->term_in_document++;
                        break;
                    }
                }
                set_destroyiter(set_iter);
            }
            else
            {
                // set didn't contain 'path'
                data_t *data = data_create();
                *data = (data_t){.path = strdup(path), .term_in_document = 1, .terms_in_document = document_word_count};
                set_add(set, data);
            }
        }
        else
        {
            data_t *data = data_create();
            set_t *set = set_create(compare_data);
            *data = (data_t){.path = strdup(path), .term_in_document = 1, .terms_in_document = document_word_count};
            set_add(set, data);
            map_put(index->map, current_word, set);
        }
    }
    free(path);
    // Moon moon does not like to double free on words
    index->doc_count++;
}

int compare_query(void *a, void *b)
{
    double d1 = ((query_result_t *)a)->score;
    double d2 = ((query_result_t *)b)->score;
    if (d1 > d2)
        return -1;
    if (d1 < d2)
        return 1;
    return 0;
}

/*
 * Performs the given query on the given index.  If the query
 * succeeds, the return value will be a list of paths.  If there
 * is an error (e.g. a syntax error in the query), an error message
 * is assigned to the given errmsg pointer and the return value
 * will be NULL.
 */
list_t *index_query(index_t *index, list_t *query, char **errmsg)
{
    // Check list size?
    set_t *set = parse_query(index, query, errmsg);
    list_t *retval = NULL;
    if (NULL != set)
    {
        set_iter_t *set_iter = set_createiter(set);
        retval = list_create(compare_query);

        while (1 == set_hasnext(set_iter))
        {
            query_result_t *query_result = malloc(sizeof(query_result_t));
            data_t *data = set_next(set_iter);
            query_result->path = data->path;
            query_result->score = (data->term_in_document / data->terms_in_document) * (log(index->doc_count / ((double)set_size(set))));
            list_addfirst(retval, query_result);
        }
        set_destroyiter(set_iter);
        list_sort(retval);
    }
    else
    {
        char *ptr = malloc(sizeof(char) * 100);
        sprintf(ptr, "Internal error: "
                     "expected ')' found '%s' "
                     "from function '%s' in file '%s' line number '%d'",
                strdup(index->current), __func__, __FILE__, __LINE__);
        *errmsg = ptr;
    }
    return retval;
}

/*
    query ::= andterm | andterm "ANDNOT" queryx1|
*/
set_t *parse_query(index_t *index, list_t *query, char **errmsg)
{

    set_t *retval = NULL, *term1 = NULL, *term2 = NULL;
    if (0 != list_size(query))
        index->current = list_popfirst(query);

    term1 = parse_andterm(index, query, errmsg);

    if (0 == compare_strings(index->current, "ANDNOT"))
    {
        if (0 != list_size(query))
            index->current = list_popfirst(query);
        term2 = parse_query(index, query, errmsg);

        if (NULL != term1 && NULL != term2)
        {
            retval = set_difference(term1, term2);
        }
    }
    else
    {
        retval = term1;
    }
    return retval;
}

/*
    andterm ::= orterm | orterm "AND" andterm
*/
set_t *parse_andterm(index_t *index, list_t *query, char **errmsg)
{
    set_t *retval = NULL, *term1 = NULL, *term2 = NULL;
    term1 = parse_orterm(index, query, errmsg);

    if (0 == compare_strings(index->current, "AND"))
    {
        if (0 != list_size(query))
            index->current = list_popfirst(query);
        term2 = parse_andterm(index, query, errmsg);

        if (NULL != term1 && NULL != term2)
        {
            retval = set_intersection(term1, term2);
        }
    }
    else
    {
        retval = term1;
    }
    return retval;
}

/*
    orterm ::= term | term "OR" orterm
*/
set_t *parse_orterm(index_t *index, list_t *query, char **errmsg)
{
    set_t *retval = NULL, *term1 = NULL, *term2 = NULL;
    term1 = parse_term(index, query, errmsg);

    if (0 == compare_strings(index->current, "OR"))
    {
        if (0 != list_size(query))
            index->current = list_popfirst(query);
        term2 = parse_orterm(index, query, errmsg);

        if (NULL != term1 && NULL != term2)
        {
            retval = set_union(term1, term2);
        }
    }
    else
    {
        retval = term1;
    }
    return retval;
}

/*
    term ::= "("query")"| <word>
*/

set_t *parse_term(index_t *index, list_t *query, char **errmsg)
{
    set_t *retval = NULL;

    if (0 == compare_strings(index->current, "("))
    {
        retval = parse_query(index, query, errmsg);
        if (0 != compare_strings(index->current, ")"))
        {
            char *ptr = malloc(sizeof(char) * 100);
            sprintf(ptr, "Internal error: "
                         "expected ')' found '%s'"
                         "from function '%s' in file '%s' line number '%d'",
                    strdup(index->current), __func__, __FILE__, __LINE__);
            *errmsg = ptr;
        }
    }
    //
    else
    {
        // 'current' found in map, get results from 'map' and store it in 'retval'
        if (1 == map_haskey(index->map, index->current))
        {
            retval = map_get(index->map, index->current);
        }
        else
        {
            char *ptr = malloc(sizeof(char) * 100);
            sprintf(ptr, "Internal error: "
                         "expected ')' found '%s'"
                         "from function '%s' in file '%s' line number '%d'",
                    strdup(index->current), __func__, __FILE__, __LINE__);
            *errmsg = ptr;
        }
    }
    // Pop ')' from 'query' into 'index->current'
    if (0 != list_size(query))
        index->current = list_popfirst(query);
    return retval;
}
