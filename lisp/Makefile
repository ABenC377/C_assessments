CC=clang
COMMON= -Wall -Wextra -Wfloat-equal -Wpedantic -Wvla -std=c99 -Werror -Wmissing-prototypes
DEBUG= -g3
SANITIZE= $(COMMON) -fsanitize=undefined -fsanitize=address $(DEBUG)
VALGRIND= $(COMMON) $(DEBUG)
# You'll probably need to change this path to somewhere you've stored general.c/.h
GENERAL= ./general
PRODUCTION= $(COMMON) -O3
LDLIBS =

all: testlinked_s testlinked_v testlinked

testlinked_s: lisp.h specific.h linked.c testlisp.c $(GENERAL)/general.h $(GENERAL)/general.c
	$(CC) testlisp.c linked.c $(GENERAL)/general.c -o testlinked_s -I./$(GENERAL) $(SANITIZE) $(LDLIBS)

testlinked_v: lisp.h specific.h linked.c testlisp.c $(GENERAL)/general.h $(GENERAL)/general.c
	$(CC) testlisp.c linked.c $(GENERAL)/general.c -o testlinked_v -I./$(GENERAL) $(VALGRIND) $(LDLIBS)

testlinked: lisp.h specific.h linked.c testlisp.c $(GENERAL)/general.h $(GENERAL)/general.c
	$(CC) testlisp.c linked.c $(GENERAL)/general.c -o testlinked -I./$(GENERAL) $(PRODUCTION) $(LDLIBS)

clean:
	rm -f testlinked_s testlinked_v testlinked

run: all
	./testlinked_s
	valgrind ./testlinked_v

zip: all
	zip lisp.zip linked.c specific.h

