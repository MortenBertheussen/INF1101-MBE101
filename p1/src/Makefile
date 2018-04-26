## Author: Steffen Viken Valvaag <steffenv@cs.uit.no> 
LIST_SRC=linkedlist.c
SET_SRC=set_list.c   # Insert the file name of your set implementation here

SPAMFILTER_SRC=spamfilter.c common.c $(LIST_SRC) $(SET_SRC)
NUMBERS_SRC=numbers.c common.c $(LIST_SRC) $(SET_SRC)
ASSERT_SRC=assert_set.c common.c $(LIST_SRC) $(SET_SRC)
TIME_SRC= time.c common.c $(LIST_SRC) $(SET_SRC)

HEADERS=common.h list.h set.h

all: spamfilter numbers

spamfilter: $(SPAMFILTER_SRC) $(HEADERS) Makefile
	gcc -o $@ $(SPAMFILTER_SRC) -g

numbers: $(NUMBERS_SRC) $(HEADERS) Makefile
	gcc -o $@ $(NUMBERS_SRC) -g

assert: $(ASSERT_SRC) $(HEADERS) Makefile
	gcc -o $@ $(ASSERT_SRC) -g

time: $(TIME_SRC) $(HEADERS) Makefile
	gcc -o $@ $(TIME_SRC) -g

clean:
	rm -f *~ *.o *.exe spamfilter numbers assert time
# 