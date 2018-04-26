/* Author: Steffen Viken Valvaag <steffenv@cs.uit.no> */
#include "list.h"
#include "set.h"
#include "common.h"

/*
 * Case-insensitive comparison function for strings.
 */
static int compare_words(void *a, void *b)
{
	return strcasecmp(a, b);
}

/*
 * Returns the set of (unique) words found in the given file.
 */
static set_t *tokenize(char *fp)
{
	set_t *wordset = set_create(compare_words);
	list_t *wordlist = list_create(compare_words);
	list_iter_t *it;
	FILE *f = fopen(fp, "r");
	if (f == NULL)
	{
		perror("fopen");
		fatal_error("fopen() failed");
	}
	tokenize_file(f, wordlist);

	it = list_createiter(wordlist);
	while (list_hasnext(it))
	{
		set_add(wordset, list_next(it));
	}
	list_destroyiter(it);
	list_destroy(wordlist);

	return wordset;
}

/*
 * Prints a set of words.
 */
static void set_print_words(char *prefix, set_t *words)
{
	set_iter_t *it = set_createiter(words);
	printf("%s: ", prefix);
	while (set_hasnext(it))
	{
		printf(" %s", (char *)set_next(it));
	}
	printf("\n");
	set_destroyiter(it);
}

static void list_print_words(char *prefix, list_t *list)
{
	list_iter_t *iter = list_createiter(list);
	printf("%s: ", prefix);

	while (list_hasnext(iter))
	{
		printf(" %s", (char *)list_next(iter));
	}
	printf("\n");
	list_destroyiter(iter);
}

/*
 * Main entry point.
 */
int main(int argc, char **argv)
{
	if (argc != 4)
	{
		fprintf(stderr, "usage: %s <spamdir> <nonspamdir> <maildir>\n",
				argv[0]);
		return 1;
	}

	char *fp;
	list_t *directories[3];
	set_t *spam[10], *nonspam[10], *mail[10];
	set_t *spamIntersection, *filter, *mailUnion, *l, *r;
	list_iter_t *listIter;
	set_iter_t *setIter;

#pragma region tokenize files
	/* Tokenize all spam-, nonspam and mail files */
	for (int i = 0; i < argc - 1; i++)
	{
		directories[i] = find_files(argv[i + 1]);
		listIter = list_createiter(directories[i]);
		for (int j = 0; list_hasnext(listIter); j++)
		{
			fp = list_next(listIter);
			switch (i)
			{
			case 0:
				spam[j] = tokenize(fp);
				break;
			case 1:
				nonspam[j] = tokenize(fp);
				break;
			case 2:
				mail[j] = tokenize(fp);
				break;
			}
		}
		list_destroyiter(listIter);
	}
#pragma endregion tokenize files


#pragma region create filter parameters	
	/*Find all common spam words*/
	l = set_intersection(spam[0], spam[1]);
	r = set_intersection(spam[2], spam[3]);
	spamIntersection = set_intersection(l, r);
	set_destroy(l);
	set_destroy(r);

	/*Find all normal words*/
	l = set_union(nonspam[0], nonspam[1]);
	r = set_union(nonspam[2], nonspam[3]);
	mailUnion = set_union(l, r);
	set_destroy(l);
	set_destroy(r);

	/*Subtract normal words from spam words, the rest is true spam words*/
	filter = set_difference(spamIntersection, mailUnion);
	set_destroy(spamIntersection);
	set_destroy(mailUnion);
#pragma endregion create filter parameters

	int count = 0;
	char *message = NULL;
	listIter = list_createiter(directories[2]);

	/*print result to screen*/
	for(int i = 0; i < 5; i++)
	{
		spamIntersection = set_intersection(mail[i], filter);
		count = set_size(spamIntersection);
		message = (count > 0) ? "SPAM" : "Not spam";
		printf("%s: %d spam words(s) -> %s\n", (char*)list_next(listIter), count, message);
	}
	set_destroy(filter);
	list_destroyiter(listIter);
}