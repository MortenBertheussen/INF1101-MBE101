## Author: Steffen Viken Valvaag <steffenv@cs.uit.no> 
LIST_SRC=linkedlist.c
COMPARE_SRC=compare.c
MAP_SRC=hashmap.c
WORDFREQS_SRC=wordfreqs.c common.c $(LIST_SRC) $(MAP_SRC) $(COMPARE_SRC)
HEADERS=common.h list.h map.h compare.h

all: wordfreqs

wordfreqs: $(WORDFREQS_SRC) $(HEADERS) Makefile
	gcc -o $@ $(WORDFREQS_SRC)

clean:
	rm -f *~ *.o *.exe wordfreqs
