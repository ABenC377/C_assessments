CC=clang

LINKLIBS:= -lm

BASEFLAGS:= -Wall -Wextra -Wfloat-equal -Wvla -Wpedantic -std=c99 $(LINKLIBS)

PRODUCTION:= -O3

DEBUG:= $(BASEFLAGS) -fsanitize=address -fsanitize=undefined -g3 

all: carpark debug extension extdebug

carpark: carpark.c
	@$(CC) carpark.c -o carpark $(PRODUCTION)
	@echo "___ carpark made ___"

debug: carpark.c
	@$(CC) carpark.c -o debug $(DEBUG)
	@echo "___ debug made ___"

extension: extension.c
	@$(CC) extension.c -o extension $(PRODUCTION)
	@echo "___ extension made ___"

extdebug: extension.c
	@$(CC) extension.c -o extdebug $(DEBUG)
	@echo "___ extdebug made ___"

zip: carpark.c extension.c extension.txt
	zip carpark.zip carpark.c extension.c extension.txt

runAll: 
	./carpark carparks/10x7_8c_16t.prk	
	./carpark carparks/10x7_8c_none.prk
	./carpark carparks/10x8_5c_13t.prk
	./carpark carparks/11x9_10c_26t.prk
	./carpark carparks/11x9_10c_none.prk
	./carpark carparks/4x4_1c_1t.prk
	./carpark carparks/6x6_2c_3t.prk
	./carpark carparks/7x7_4c_11t.prk
	./carpark carparks/extrachar.prk
	./carpark carparks/nocars.prk
	./carpark carparks/nonconsec.prk
	./carpark carparks/wrongshape2.prk
	./carpark carparks/wrongshape.prk
