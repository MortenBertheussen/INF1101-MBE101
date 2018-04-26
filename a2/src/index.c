#include "index.h"

#define ERROR_MSG "%s : %s(), at line: %d", __FILE__, __func__, __LINE__

struct index
{
    map_t *map;
    char *current;
    list_iter_t *query_iterator;
    double doc_count;
};

typedef struct data
{
    char *path;
    double term_in_document;
    double terms_in_document;
    double tf;
    double idf;
} data_t;

set_t *parse_query(index_t *index, char **errmsg);
set_t *parse_andterm(index_t *index, char **errmsg);
set_t *parse_orterm(index_t *index, char **errmsg);
set_t *parse_term(index_t *index, char **errmsg);
set_t *parse_andnot(index_t *index, char **errmsg);

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
    map_destroy(index->map, free, free);
    if (index->query_iterator != NULL)
    {
        list_destroyiter(index->query_iterator);
    }
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
    list_iter_t *list_iter = list_createiter(words);

    //
    while (list_hasnext(list_iter))
    {
        char *current_word = list_next(list_iter);

        if (map_haskey(index->map, current_word) == 1)
        {
            set_t *set = map_get(index->map, current_word);

            // if set contains data_t
            if (set_contains(set, &(data_t){.path = path}) == 1)
            {
                // iterate over set to find data so we can manipulate it
                set_iter_t *set_iter = set_createiter(set);
                while (set_hasnext(set_iter))
                {
                    data_t *data = set_next(set_iter);
                    if (compare_strings(data->path, path) == 0)
                    {
                        data->term_in_document++;
                        break;
                    }
                }
                set_destroyiter(set_iter);
            }
            else
            {
                // set didn't contain data_t
                data_t *data = data_create();
                *data = (data_t){.path = strdup(path), .term_in_document = 1, .terms_in_document = list_size(words)};
                set_add(set, data);
            }
        }
        else
        {
            data_t *data = data_create();
            set_t *set = set_create(compare_data);

            *data = (data_t){.path = strdup(path), .term_in_document = 1, .terms_in_document = list_size(words)};
            set_add(set, data);
            map_put(index->map, current_word, set);
        }
    }

    list_destroyiter(list_iter);
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
    index->query_iterator = list_createiter(query);

    if (list_hasnext(index->query_iterator))
        index->current = list_next(index->query_iterator);

    list_t *retval = NULL;
    set_t *set = parse_query(index, errmsg);

    if (set != NULL)
    {
        set_iter_t *set_iter = set_createiter(set);
        retval = list_create(compare_query);

        while (set_hasnext(set_iter))
        {
            query_result_t *query_result = malloc(sizeof(query_result_t));
            data_t *data = set_next(set_iter);
            data->tf = data->term_in_document / data->terms_in_document;
            data->idf = log(index->doc_count / ((double)set_size(set)));

            query_result->path = data->path;
            query_result->score = data->tf * data->idf;
            list_addfirst(retval, query_result);
        }
        set_destroyiter(set_iter);
    }
    list_destroyiter(index->query_iterator);
    list_sort(retval);
    return retval;
}

/*
    query ::= andterm | andterm "ANDNOT" query
*/
set_t *parse_query(index_t *index, char **errmsg)
{

    set_t *retval = NULL, *term1, *term2;
    term1 = parse_andterm(index, errmsg);

    if (compare_strings(index->current, "ANDNOT") == 0)
    {
        if (list_hasnext(index->query_iterator))
        {
            index->current = list_next(index->query_iterator);
            term2 = parse_query(index, errmsg);
            if (term1 != NULL && term2 != NULL)
                retval = set_difference(term1, term2);
            else
                *errmsg = "parse_andterm, term1 || term2 not found";
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
set_t *parse_andterm(index_t *index, char **errmsg)
{
    set_t *retval = NULL, *term1, *term2;
    term1 = parse_orterm(index, errmsg);
    if (compare_strings(index->current, "AND") == 0)
    {
        if (list_hasnext(index->query_iterator))
        {
            index->current = list_next(index->query_iterator);
            term2 = parse_andterm(index, errmsg);
            if (term1 != NULL && term2 != NULL)
                retval = set_intersection(term1, term2);
            else
                *errmsg = "parse_andterm, term1 || term2 not found";
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
set_t *parse_orterm(index_t *index, char **errmsg)
{
    set_t *retval = NULL, *term1, *term2;
    term1 = parse_term(index, errmsg);

    if (compare_strings(index->current, "OR") == 0)
    {
        if (list_hasnext(index->query_iterator))
        {
            index->current = list_next(index->query_iterator);
            term2 = parse_orterm(index, errmsg);
            if (term1 != NULL && term2 != NULL)
                retval = set_union(term1, term2);
            else
                *errmsg = "parse_orterm, term1 || term2 not found";
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
set_t *parse_term(index_t *index, char **errmsg)
{
    set_t *retval = NULL;

    if (compare_strings(index->current, "(") == 0 && list_hasnext(index->query_iterator))
    {
        if (list_hasnext(index->query_iterator))
            index->current = list_next(index->query_iterator);

        retval = parse_query(index, errmsg);

        if (compare_strings(index->current, ")") != 0)
            sprintf(*errmsg, "Internal error: "
                             "expected ')' found '%s'"
                             "from function '%s' in file '%s' line number '%d'",
                    index->current, __func__, __FILE__, __LINE__);
        else if (list_hasnext(index->query_iterator))
            index->current = list_next(index->query_iterator);
    }
    else
    {
        if (map_haskey(index->map, index->current) == 1)
        {
            retval = map_get(index->map, index->current);
            if (list_hasnext(index->query_iterator))
                index->current = list_next(index->query_iterator);
        }
        else
        {
            sprintf(*errmsg, "Internal error: "
                             "key not found in map: '%s'  "
                             "from function '%s' in file '%s' line number '%d'",
                    index->current, __func__, __FILE__, __LINE__);
        }
    }
    return retval;
}
