#include "tokentype.h"
#include "syntaxtree.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>


void parse_list(Token_list* list);
Tree_node* descend_recursively(Token_node** current, bool* parses_correctly);
Tree_node* handle_INSTRCTS(Token_node** current, bool* parses_correctly);
Tree_node* handle_INSTRCT(Token_node** current, bool* parses_correctly);
Tree_node* handle_FUNC(Token_node** current, bool* parses_correctly);
bool is_RETFUNC(Token_node* current);
Tree_node* handle_RETFUNC(Token_node** current, bool* parses_correctly);
bool is_IOFUNC(Token_node* current);
Tree_node* handle_IOFUNC(Token_node** current, bool* parses_correctly);
Tree_node* handle_SET(Token_node** current, bool* parses_correctly);
bool is_IF(Token_node* current);
Tree_node* handle_IF(Token_node** current, bool* parses_correctly);
bool is_LOOP(Token_node* current);
Tree_node* handle_LOOP(Token_node** current, bool* parses_correctly);
bool is_LISTFUNC(Token_node* current);
Tree_node* handle_LISTFUNC(Token_node** current, bool* parses_correctly);
bool is_INTFUNC(Token_node* current);
Tree_node* handle_INTFUNC(Token_node** current, bool* parses_correctly);
bool is_BOOLFUNC(Token_node* current);
Tree_node* handle_BOOLFUNC(Token_node** current, bool* parses_correctly);
Tree_node* handle_LIST(Token_node** current, bool* parses_correctly);
Tree_node* handle_VAR(Token_node** current, bool* parses_correctly);
Tree_node* handle_LITERAL(Token_node** current, bool* parses_correctly);
Tree_node* handle_NIL(Token_node** current, bool* parses_correctly);
Tree_node* handle_PRINT(Token_node** current, bool* parses_correctly);
bool is_LIST(Token_node* current);
Tree_node* handle_STRING(Token_node** current, bool* parses_correctly);
Tree_node* make_node(grammar_type type);
Tree_node* parser_fails(bool* parses_correctly);
bool next_token_is(Token_node** current, int num_possible_tokens, ...);
void print_tree(Syntax_tree* tree);
void print_tree_node(Tree_node* node);
char* get_node_type(Tree_node* node);
void free_tree(Syntax_tree* tree);
void free_node(Tree_node* node);

