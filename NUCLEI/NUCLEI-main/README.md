# NUCLEI-interpreter

## Project parameters
An interpreter and parser for a programming language called 'NUCLEI'

NUCLEI is a new prgramming language that is inspired by LISP, though has a more limited set of intructions.
Like LISP, NUCLEI uses a CAR/CDR structure for storing its data.

The formal grammar of NUCLEI is as follows:

*PROG ::= "(" INSTRCTS*

*INSTRCTS ::= INSTRCT INSTRCTS | ")"*

*INSTRCT ::= "(" FUNC ")"*

*FUNC ::= RETFUNC | IOFUNC | IF | LOOP*

*RETFUNC ::= LISTFUNC | INTFUNC | BOOLFUNC*

*LISTFUNC ::= "CAR" LIST | "CDR" LIST | "CONS" LIST LIST*

*INTFUNC ::= "PLUS" LIST LIST | "LENGTH" LIST*

*BOOLFUNC ::= "LESS" LIST LIST | "GREATER" LIST LIST | "EQUAL" LIST LIST*

*IOFUNC ::= SET | PRINT*

*SET ::= "SET" VAR LIST*

*PRINT ::= "PRINT" LIST | "PRINT" STRING*

*IF ::= "IF" "(" BOOLFUNC ")" "(" INSTRCTS "(" INSTRCTS*

*LOOP ::= "WHILE" "(" BOOLFUNC ")" "(" INSRCTS*

*LIST ::= VAR | LITERAL | "NIL" | "(" RETFUNC ")"*

*VAR ::= [A-Z]*

*STRING ::= Double-quoted string constant e.g. "Hello, World!", or "FAILURE ?"*

*LITERAL ::= Single-quoted list e.g. ’(1)’, ’(1 2 3 (4 5))’, or ’2’*

  
NUCLEI ignores white space.

## Summary of approach

The parser/interpretter runs by doing a first lexical analysis-parse of the file to turn it into a linked list of tokens.  The code for doing this is int the 'lexical_parser.c' and 'lexical_parser.h' files.  From this list of tokens, the parser/interpretter is run.

The lexical parser is basically a finite state automata, which takes the characters of the input file as inputs, and then adds tokens to the linked list of tokens as an output.  Because of the variety of possible characters and tokens, there are 39 states to the automata.  The easiest way for me to code this automata was with a switch statement.  This switch statement is very large, but I think easy enough to understand.



