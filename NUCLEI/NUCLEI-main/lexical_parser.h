#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#define SINGLEQUOTE 39
#define LEXEMEMAXLEN 100000

typedef enum {
    s_start,
    s_in_literal,
    s_in_string,
    s_in_invalid,
    s_in_variable,
    s_C = 'C', s_E = 'E', s_G = 'G', s_I = 'I', s_L = 'L', 
    s_N = 'N', s_P = 'P', s_S = 'S', s_W = 'W', s_NI, 
    s_NIL, s_WH, s_WHI, s_WHIL, s_WHILE, s_IF, s_PR, 
    s_PRI, s_PRIN, s_PRINT, s_PL, s_PLU, s_PLUS, s_SE, 
    s_SET, s_LE, s_LES, s_LESS, s_LEN, s_LENG, s_LENGT, 
    s_LENGTH, s_GR, s_GRE, s_GREA, s_GREAT, s_GREATE, 
    s_GREATER, s_EQ, s_EQU, s_EQUA, s_EQUAL, s_CO, s_CON, 
    s_CONS, s_CD, s_CDR, s_CA, s_CAR
} automata_state;

typedef enum {
    t_literal, 
    t_string,
    t_invalid, 
    t_variable, 
    t_nil = s_NIL, 
    t_while = s_WHILE, 
    t_if = s_IF, 
    t_print = s_PRINT, 
    t_set = s_SET,
    t_equal = s_EQUAL,
    t_greater = s_GREATER,
    t_less = s_LESS,
    t_length = s_LENGTH,
    t_plus = s_PLUS,
    t_CONS = s_CONS,
    t_CDR = s_CDR,
    t_CAR = s_CAR,
    t_l_parenthesis = '(', 
    t_r_parenthesis = ')'
} token_type; 

typedef struct {
    token_type type;
    char* lexeme;
    char var_name;
    int line;
    int col;
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

typedef struct {
    automata_state state;
    Token* token;
    int line;
    int col;
} Automata;

Token_list* run_lexical_analyser(int argc, char* argv[]);
char* get_file_name(int argc, char* argv[]);
Token_list* get_tokens_from_file(FILE* fp);
void update_tokens(Token_list* tokens, Automata* automata, char c);
bool is_white_space(char c) ;
void add_variable(Token_list* tokens, Automata* automata, char name) ;
void make_and_add_simple_token(Token_list* tokens, Automata* automata, token_type type);
void handle_s_start(Token_list* tokens, Automata* automata, char c);
void start_word(Automata* automata, char c);
bool is_start_of_reserved_word(char c);
void start_variable(Automata* automata, char c);
void start_lexeme(Automata* automata, char c);
void check_for_var(Token_list* tokens, Automata* automata, char name, char c);
void check_end_of_token(Token_list* tokens, Automata* automata, token_type type, char c);
void handle_lexeme(Token_list* tokens, Automata* automata, char c);
void handle_s_invalid(Token_list* tokens, Automata* automata, char c);
void handle_s_variable(Token_list* tokens, Automata* automata, char c);
void print_tokens(Token_list* tokens);
void print_token(Token_node* node);
void add_token(Token_list* tokens, Token* token);
void free_token_list(Token_list* list);
void free_token_node(Token_node* node);
void lexical_analysis_test(void);
