CC = clang

BASEFLAGS:= -Wall -Wextra -Wpedantic -std=c99 -Wvla -Wfloat-equal 

LINKLIBS:= -lm

PROD:= -o bingrid $(BASEFLAGS) -O3 $(LINKLIBS)

DEBUG:= -o debug $(BASEFLADS) -fsanitize=address -fsanitize=undefined -g3 $(LINKLIBS)

all: bingrid debug

bingrid: bingrid.c bingrid.h bingrid_driver.c
	$(CC) bingrid.c bingrid_driver.c $(PROD)
	@echo "___ Bingrid made ___"

debug: bingrid.c bingrid.h bingrid_driver.c
	$(CC) bingrid.c bingrid_driver.c $(DEBUG)
	@echo "___ Debug made ___"

rundebug:
	./debug
	valgrind --leak-check=full --show-leak-kinds=all ./bingrid

clean:
	rm -f bingrid debug

