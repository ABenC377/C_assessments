# In *my* code, I had the interpreter code added to the parser code.
# This additional code is protected by #ifdef INTERP / #endif clauses.
# This code is only "seen" by the compiler if you have #defined INTERP
# This #define would be annoying to change inside nuclei.c/.h, so instead it is
# set in the gcc/clang statement using -DINTERP
# In this way ./parse & ./interp can both be built from the same source file.

CC      := clang
DEBUG   := -g3
OPTIM   := -O3
CFLAGS  := -Wall -Wextra -Wpedantic -Wfloat-equal -Wvla -std=c99 -Werror
RELEASE := $(CFLAGS) $(OPTIM)
SANI    := $(CFLAGS) -fsanitize=undefined -fsanitize=address $(DEBUG)
VALG    := $(CFLAGS)  $(DEBUG)
NCLS    := $(wildcard *.ncl)
PRES := $(NCLS:.ncl=.pres)
IRES := $(NCLS:.ncl=.ires)
ERES := $(NCLS:.ncl=.eres)
LIBS    := -lm


parse: nuclei.c nuclei.h lexical_parser.c lexical_parser.h lisp.c lisp.h
	$(CC)  nuclei.c lexical_parser.c lisp.c $(RELEASE) -o parse $(LIBS)

parse_s: nuclei.c nuclei.h lexical_parser.c lexical_parser.h lisp.c lisp.h
	$(CC) nuclei.c lexical_parser.c lisp.c $(SANI) -o parse_s -DDEBUG $(LIBS)

parse_v: nuclei.c nuclei.h lexical_parser.c lexical_parser.h lisp.c lisp.h
	$(CC) nuclei.c lexical_parser.c lisp.c $(VALG) -o parse_v $(LIBS)

all: parse parse_s parse_v interp interp_s interp_v

interp: nuclei.c nuclei.h lexical_parser.c lexical_parser.h lisp.c lisp.h
	$(CC) nuclei.c lexical_parser.c lisp.c $(RELEASE) -DINTERP -o interp $(LIBS)

interp_s: nuclei.c nuclei.h lexical_parser.c lexical_parser.h lisp.c lisp.h
	$(CC) nuclei.c lexical_parser.c lisp.c $(SANI) -DINTERP -o interp_s -DDEBUG $(LIBS)

interp_v: nuclei.c nuclei.h lexical_parser.c lexical_parser.h lisp.c lisp.h
	$(CC) nuclei.c lexical_parser.c lisp.c $(VALG) -DINTERP -o interp_v $(LIBS)
	
extension: nuclei.c nuclei.h lexical_parser.c lexical_parser.h lisp.c lisp.h
	$(CC) nuclei.c lexical_parser.c lisp.c $(RELEASE) -DINTERP -DEXT -o extension $(LIBS)

extension_s: nuclei.c nuclei.h lexical_parser.c lexical_parser.h lisp.c lisp.h
	$(CC) nuclei.c lexical_parser.c lisp.c $(SANI) -DINTERP -DEXT -o extension_s -DDEBUG $(LIBS)

extension_v: nuclei.c nuclei.h lexical_parser.c lexical_parser.h lisp.c lisp.h
	$(CC) nuclei.c lexical_parser.c lisp.c $(VALG) -DINTERP -DEXT -o extension_v $(LIBS)

# For all .ncl files, run them and output result to a .pres (prase result) 
# or .ires (interpretted result) file.
# or .eres for extension results
runall : ./parse_s ./interp_s ./extension_s $(PRES) $(IRES) $(ERES)

%.pres:
	-./parse_s  $*.ncl > $*.pres 2>&1
%.ires:
	-./interp_s $*.ncl > $*.ires 2>&1
%.eres:
	-./extension_s $*.ncl > $*.eres 2>&1

clean:
	rm -f parse parse_s parse_v interp interp_s interp_v extension extension_s extension_v $(PRES) $(IRES) $(ERES)
	
zip: nuclei.c nuclei.h lexical_parser.c lexical_parser.h lisp.c lisp.h Makefile testing.txt extension.txt
	zip nuclei.zip nuclei.c nuclei.h lexical_parser.c lexical_parser.h lisp.c lisp.h Makefile testing.txt extension.txt

