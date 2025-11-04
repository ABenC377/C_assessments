#pragma once

// #include "specific.h"
#include "./lexical_parser.h"
#include "./lisp.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#define MAXERR 20
#define NUMVARS 26
#define MAXSTR 1000

typedef enum {
    PROG,
    INSTRCTS,
    INSTRCT,
    FUNC,
    RETFUNC,
    LISTFUNC,
    INTFUNC,
    BOOLFUNC,
    IOFUNC,
    SET,
    PRINT,
    IF,
    LOOP,
    LIST,
    VAR,
    STRING,
    LITERAL,
    NIL,
    ERROR_NODE
} grammar_type;

typedef struct Tree_node {
    grammar_type type;
    char var_name;
    char* string_value;
    token_type func_type;
    Lisp* list;
    struct Tree_node* child1;
    struct Tree_node* child2;
    struct Tree_node* child3;
} Tree_node;

typedef struct {
    Tree_node* program;
} Syntax_tree;

typedef struct {
    char* message;
    int line;
    int col;
} Error;

typedef struct {
    bool parser_error;
    bool interp_error;
    int num_errors;
    Error* errors[MAXERR];
    bool overflow;
    Lisp* variables[NUMVARS];
    bool executing;
} Prog_log;

void check_inputs(int argc, char* argv[]);
void parse_list(Token_list* list);
Tree_node* descend_recursively(Token_node** current, Prog_log* program_log);
Tree_node* handle_INSTRCTS(Token_node** current, Prog_log* program_log);
Tree_node* handle_INSTRCT(Token_node** current, Prog_log* program_log);
Tree_node* handle_FUNC(Token_node** current, Prog_log* program_log);
bool is_RETFUNC(Token_node* current);
Tree_node* handle_RETFUNC(Token_node** current, Prog_log* program_log);
bool is_IOFUNC(Token_node* current);
Tree_node* handle_IOFUNC(Token_node** current, Prog_log* program_log);
Tree_node* handle_SET(Token_node** current, Prog_log* program_log);
bool is_IF(Token_node* current);
Tree_node* handle_IF(Token_node** current, Prog_log* program_log);
bool is_LOOP(Token_node* current);
Tree_node* handle_LOOP(Token_node** current, Prog_log* program_log);
bool is_LISTFUNC(Token_node* current);
Tree_node* handle_LISTFUNC(Token_node** current, Prog_log* program_log);
Lisp* evaluate_list_function(token_type type, Lisp** arg1, Lisp** arg2);
bool is_INTFUNC(Token_node* current);
Tree_node* handle_INTFUNC(Token_node** current, Prog_log* program_log);
Lisp* evaluate_int_function(Token_node* node, token_type type, Prog_log* log, Lisp** arg1, Lisp** arg2);
Lisp* evaluate_plus(Token_node* node, Lisp* arg1, Lisp* arg2, Prog_log* log);
Lisp* evaluate_length(Lisp* arg);
bool is_BOOLFUNC(Token_node* current);
Tree_node* handle_BOOLFUNC(Token_node** current, Prog_log* program_log);
bool evaluate_bool(Token_node* node, Lisp* arg1, Lisp* arg2, Prog_log* log);
Tree_node* handle_LIST(Token_node** current, Prog_log* program_log);
void check_initialised(Token_node* token_node, Tree_node* tree_node, Prog_log* log);
Lisp* move_list(Lisp** original);
Tree_node* handle_VAR(Token_node** current, Prog_log* program_log);
Tree_node* handle_LITERAL(Token_node** current, Prog_log* program_log);
Tree_node* handle_NIL(Token_node** current, Prog_log* program_log);
Tree_node* handle_PRINT(Token_node** current, Prog_log* program_log);
void print_lisp(Lisp** lisp, Prog_log* log);
bool is_LIST(Token_node* current);
Tree_node* handle_STRING(Token_node** current, Prog_log* program_log);
Tree_node* make_node(grammar_type type);
Tree_node* parser_fails(Prog_log* program_log, Token* token, char* error_message);
bool next_token_is(Token_node** current, int num_possible_tokens, ...);
void print_tree(Syntax_tree* tree);
void print_tree_node(Tree_node* node);
char* get_node_type(Tree_node* node);
void free_tree(Syntax_tree* tree);
void free_node(Tree_node* node);
void print_log(Prog_log* log);
void free_log(Prog_log* log);
void add_error(Prog_log* log, Error* error, bool parsing);
void test(void);
void parse_test(void);
void interp_test(void);
void ext_test(void);
