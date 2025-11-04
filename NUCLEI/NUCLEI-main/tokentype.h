#pragma once 

typedef enum {
    t_literal, 
    t_string, 
    t_variable, 
    t_nil, 
    t_while, 
    t_if, 
    t_print, 
    t_set,
    t_equal,
    t_greater,
    t_less,
    t_length,
    t_plus,
    t_CONS,
    t_CDR,
    t_CAR,
    t_l_parenthesis, 
    t_r_parenthesis,
    t_invalid
} token_type; 

typedef struct {
    token_type type;
    char* lexeme;
    char var_name;
} Token;

typedef struct Token_node {
    Token* value;
    struct Token_node* next;
    struct Token_node* previous;
} Token_node;

typedef struct Token_list {
    Token_node* start;
    Token_node* end;
} Token_list;
