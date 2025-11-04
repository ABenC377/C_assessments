#include "nuclei.h"

Token_list* run_lexical_analyser(int argc, char* argv[]) {
    char* file_name = get_file_name(argc, argv);
    if (!file_name) {
        throw_error("ERROR: incorrect usage - please provide an input file\n");
    }
    
    FILE* fp = fopen(file_name, "r");
    
    Token_list* tokens = get_tokens_from_file(fp);
    
    fclose(fp);
    
    return tokens;
}

char* get_file_name(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        char* argument = argv[i];
        int arg_length = strlen(argument);
        for (int j = 0; j < (arg_length - 1); j++) {
            if (argument[j] == '.') {
                return argument;
            }
        }
    }
    return NULL;
} 

Token_list* get_tokens_from_file(FILE* fp) {
    Token_list* tokens = (Token_list*)allocate_space(1, sizeof(Token_list));
    Automata* automata = (Automata*)allocate_space(1, sizeof(Automata));
    automata->line = 1; automata->col = 1;
    char c = (char)fgetc(fp);
    bool in_comment = false;
    while (c != EOF) {
        in_comment = (c == '#') ? true : in_comment;
        in_comment = (c == '\n') ? false : in_comment;
        if (!in_comment) {
            update_tokens(tokens, automata, c);
        }
        c = (char)fgetc(fp);
        if (c == '\n' || c == '\r') {
            (automata->line)++;
            automata->col = 0;
        } else {
            (automata->col)++;
        }
    }
    free(automata);
    return tokens;
}

void update_tokens(Token_list* tokens, Automata* automata, char c) {
    switch (automata->state) {
        case s_start:
            handle_s_start(tokens, automata, c);
            break;
        case s_in_literal:
        case s_in_string:
            handle_lexeme(tokens, automata, c);
            break;
        case s_in_invalid:
            handle_s_invalid(tokens, automata, c);
            break;
        case s_in_variable:
            handle_s_variable(tokens, automata, c);
            break;
        case s_NIL:
        case s_WHILE:
        case s_IF:
        case s_PRINT:
        case s_PLUS:
        case s_SET:
        case s_LESS:
        case s_LENGTH:
        case s_GREATER:
        case s_EQUAL:
        case s_CAR:
        case s_CDR:
        case s_CONS:
            check_end_of_token(tokens, automata, (token_type)automata->state, c);
            break;
        case s_N:
            if (c == 'I') {
                automata->state = s_NI;
            } else {
                check_for_var(tokens, automata, 'N', c);
            }
            break;
        case s_NI:
            if (c == 'L') {
                automata->state = s_NIL;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_W:
            if (c == 'H') {
                automata->state = s_WH;
            } else {
                check_for_var(tokens, automata, 'W', c);
            }
            break;
        case s_WH:
            if (c == 'I') {
                automata->state = s_WHI;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_WHI:
            if (c == 'L') {
                automata->state = s_WHIL;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_WHIL:
            if (c == 'E') {
                automata->state = s_WHILE;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_I:
            if (c == 'F') {
                automata->state = s_IF;
            } else {
                check_for_var(tokens, automata, 'I', c);
            }
            break;
        case s_P:
            if (c == 'R') {
                automata->state = s_PR;
            } else if (c == 'L') {
                automata->state = s_PL;
            } else {
                check_for_var(tokens, automata, 'P', c);
            }
            break;
        case s_PR:
            if (c == 'I') {
                automata->state = s_PRI;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_PRI:
            if (c == 'N') {
                automata->state = s_PRIN;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_PRIN:
            if (c == 'T') {
                automata->state = s_PRINT;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_PL:
            if (c == 'U') {
                automata->state = s_PLU;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_PLU:
            if (c == 'S') {
                automata->state = s_PLUS;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_S:
            if (c == 'E') {
                automata->state = s_SE;
            } else {
                check_for_var(tokens, automata, 'S', c);
            }
            break;
        case s_SE:
            if (c == 'T') {
                automata->state = s_SET;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_L:
            if (c == 'E') {
                automata->state = s_LE;
            } else {
                check_for_var(tokens, automata, 'L', c);
            }
            break;
        case s_LE:
            if (c == 'S') {
                automata->state = s_LES;
            } else if (c == 'N') {
                automata->state = s_LEN;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_LES:
            if (c == 'S') {
                automata->state = s_LESS;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_LEN:
            if (c == 'G') {
                automata->state = s_LENG;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_LENG:
            if (c == 'T') {
                automata->state = s_LENGT;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_LENGT:
            if (c == 'H') {
                automata->state = s_LENGTH;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_G:
            if (c == 'R') {
                automata->state = s_GR;
            } else {
                check_for_var(tokens, automata, 'G', c);
            }
            break;
        case s_GR:
            if (c == 'E') {
                automata->state = s_GRE;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_GRE:
            if (c == 'A') {
                automata->state = s_GREA;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_GREA:
            if (c == 'T') {
                automata->state = s_GREAT;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_GREAT:
            if (c == 'E') {
                automata->state = s_GREATE;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_GREATE:
            if (c == 'R') {
                automata->state = s_GREATER;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_E:
            if (c == 'Q') {
                automata->state = s_EQ;
            } else {
                check_for_var(tokens, automata, 'E', c);
            }
            break;
        case s_EQ:
            if (c == 'U') {
                automata->state = s_EQU;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_EQU:
            if (c == 'A') {
                automata->state = s_EQUA;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_EQUA:
            if (c == 'L') {
                automata->state = s_EQUAL;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_C:
            if (c == 'A') {
                automata->state = s_CA;
            } else if (c == 'D') { 
                automata->state = s_CD;
            } else if (c == 'O') {
                automata->state = s_CO;
            } else {
                check_for_var(tokens, automata, 'C', c);
            }
            break;
        case s_CA:
            if (c == 'R') {
                automata->state = s_CAR;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_CD:
            if (c == 'R') {
                automata->state = s_CDR;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_CO:
            if (c == 'N') {
                automata->state = s_CON;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
        case s_CON:
            if (c == 'S') {
                automata->state = s_CONS;
            } else {
                handle_s_invalid(tokens, automata, c);
            }
            break;
    }
}

bool is_white_space(char c) {
    return (c == ' ' || c == '\n' || c == '\t' || c == '\v' || c == '\f' || c == '\r');
}

void add_variable(Token_list* tokens, Automata* automata, char name) {
    automata->state = s_start;
    if (!(automata->token)) {
        automata->token = (Token*)allocate_space(1, sizeof(Token));
        automata->token->line = automata->line;
        automata->token->col = automata->col;
    }
    automata->token->type = t_variable;
    automata->token->var_name = name;
    add_token(tokens, automata->token);
    automata->token = NULL;
}

void make_and_add_simple_token(Token_list* tokens, Automata* automata, token_type type) {
    automata->state = s_start;
    if (!(automata->token)) {
        automata->token = (Token*)allocate_space(1, sizeof(Token));
        automata->token->line = automata->line;
        automata->token->col = automata->col;
    }
    automata->token->type = type;
    add_token(tokens, automata->token);
    automata->token = NULL;
}

void handle_s_start(Token_list* tokens, Automata* automata, char c) {
    if (!is_white_space(c)) {
        switch (c) {
            case ')':
            case '(':
                make_and_add_simple_token(tokens, automata, c);
                break;
            case SINGLEQUOTE:
            case '"':
                start_lexeme(automata, c);
                break;
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': 
            case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': 
            case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': 
            case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': 
            case 'Y': case 'Z': 
                start_word(automata, c);
                break;
            default:
                handle_s_invalid(tokens, automata, c);
                break;
        }
    }
}

void start_word(Automata* automata, char c) {
    if  (is_start_of_reserved_word(c)) {
        automata->state = c;
    } else {
        start_variable(automata, c);
    }
}

bool is_start_of_reserved_word(char c) {
    bool CDG = (c == 'C' || c == 'E' || c == 'G');
    bool ILN = (c == 'I' || c == 'L' || c == 'N');
    bool PSW = (c == 'P' || c == 'S' || c == 'W');
    return (CDG || ILN || PSW);
}

void start_variable(Automata* automata, char c) {
    if (!(automata->token)) {
        automata->token = (Token*)allocate_space(1, sizeof(Token));
        automata->token->line = automata->line;
        automata->token->col = automata->col;
    }
    automata->token->type = t_variable;
    automata->token->var_name = c;
    automata->state = s_in_variable;
}

void start_lexeme(Automata* automata, char c) {
    if (!(automata->token)) {
        automata->token = (Token*)allocate_space(1, sizeof(Token));
        automata->token->line = automata->line;
        automata->token->col = automata->col;
    }
    automata->token->type = (c == '"') ? t_string : t_literal;
    automata->token->lexeme = (char*)allocate_space(LEXEMEMAXLEN, sizeof(char));
    automata->state = (c == '"') ? s_in_string : s_in_literal;
}

void check_for_var(Token_list* tokens, Automata* automata, char name, char c) {
    if (is_white_space(c)) {
        add_variable(tokens, automata, name);
    } else if (c == ')') {
        add_variable(tokens, automata, name);
        make_and_add_simple_token(tokens, automata, t_r_parenthesis);
    } else {
        handle_s_invalid(tokens, automata, c);
    }
}

void check_end_of_token(Token_list* tokens, Automata* automata, token_type type, char c) {
    if (is_white_space(c)) {
        make_and_add_simple_token(tokens, automata, type);
    } else if (c == ')') {
        make_and_add_simple_token(tokens, automata, type);
        make_and_add_simple_token(tokens, automata, t_r_parenthesis);
    } else {
        handle_s_invalid(tokens, automata, c);
    }
}


void handle_lexeme(Token_list* tokens, Automata* automata, char c) {
    if ((automata->state == s_in_literal) && (c == SINGLEQUOTE)) {
        add_token(tokens, automata->token);
        automata->token = NULL;
        automata->state = s_start;
    } else if ((automata->state == s_in_string) && (c == '"')) {
        add_token(tokens, automata->token);
        automata->token = NULL;
        automata->state = s_start;
    } else {
        int index = 0;
        while ((index < LEXEMEMAXLEN) && (automata->token->lexeme[index] != '\0')) {
            index++;
        }
        if (index == LEXEMEMAXLEN) {
            throw_error("ERROR: literal/string lexeme greater than maximum allowed string length\n");
        }
        automata->token->lexeme[index] = c;
    }
}

void handle_s_invalid(Token_list* tokens, Automata* automata, char c) {
    automata->state = s_in_invalid;
    if (is_white_space(c)) {
        automata->state = s_start;
        make_and_add_simple_token(tokens, automata, t_invalid);
    } else if (c == ')') {
        automata->state = s_start;
        make_and_add_simple_token(tokens, automata, t_invalid);
        make_and_add_simple_token(tokens, automata, t_r_parenthesis);
    }
}

void handle_s_variable(Token_list* tokens, Automata* automata, char c) {
    if (is_white_space(c)) {
        add_token(tokens, automata->token);
        automata->token = NULL;
        automata->state = s_start;
    } else if (c == ')') {
        add_variable(tokens, automata, automata->token->var_name);
        make_and_add_simple_token(tokens, automata, t_r_parenthesis);
    } else {
        automata->token->type = t_invalid;
        automata->state = s_in_invalid;
    }
}

void print_tokens(Token_list* tokens) {
    if (!(tokens) || !(tokens->start)) {
        printf("Empty list - cannot print\n");
    }
    Token_node* current = tokens->start;
    while (current) {
        print_token(current);
        printf(" -> ");
        current = current->next;
    }
    printf("END\n");
}

void print_token(Token_node* node) {
    if (node) {
        switch (node->value->type) {
            case t_literal:
                printf("<LITERAL>");
                break;
            case t_string:
                printf("<STRING>");
                break;
            case t_variable:
                printf("<VARIABLE>");
                break;
            case t_nil:
                printf("NIL");
                break;
            case t_while:
                printf("WHILE");
                break;
            case t_if:
                printf("IF");
                break;
            case t_print:
                printf("PRINT");
                break;
            case t_set:
                printf("SET");
                break;
            case t_equal:
                printf("EQUAL");
                break;
            case t_greater:
                printf("GREATER");
                break;
            case t_less:
                printf("LESS");
                break;
            case t_length:
                printf("LENGTH");
                break;
            case t_plus:
                printf("PLUS");
                break;
            case t_CONS:
                printf("CONS");
                break;
            case t_CDR:
                printf("CDR");
                break;
            case t_CAR:
                printf("CAR");
                break;
            case t_l_parenthesis:
                printf("(");
                break;
            case t_r_parenthesis:
                printf(")");
                break;
            case t_invalid:
                printf("INVALID");
                break;
        }
    }
}

void add_token(Token_list* tokens, Token* token) {
    if (token) {
        Token_node* new_token_node = (Token_node*)allocate_space(1, sizeof(Token_node));
        new_token_node->value = token;
        if (!(tokens->start)) {
            tokens->start = tokens->end = new_token_node;
        } else {
            Token_node* temp = tokens->end;
            tokens->end = temp->next = new_token_node;
            tokens->end->previous = temp;
        }
    }
}

void free_token_list(Token_list* list) {
    if (list) {
        free_token_node(list->start);
    }
    free(list);
}

void free_token_node(Token_node* node) {
    if (node) {
        if (node->next) {
            free_token_node(node->next);
        }
        if (node->value->lexeme) {
            free(node->value->lexeme);
        }
        free(node->value);
        free(node);
    }
}

void lexical_analysis_test(void) {
    // char* get_file_name(int argc, char* argv[]) 
    int test_argc = 3;
    char* test_argv[3];
    test_argv[0] = "./nuclei";
    test_argv[1] = "folder/file.ncl";
    test_argv[2] = "folder/other.ncl";
    char* test_file_name = get_file_name(test_argc, test_argv);
    assert(strcmp(test_file_name, test_argv[1]) == 0);
    
    test_argv[1] = "not_a_file.";
    test_file_name = get_file_name(test_argc, test_argv);
    assert(strcmp(test_file_name, test_argv[2]) == 0);
    
    test_argv[2] = "also_not_a_file_name";
    test_file_name = get_file_name(test_argc, test_argv);
    assert(!test_file_name);
    
    // void update_tokens(Token_list* tokens, Automata* automata, char c);
    Token_list* test_tokens = (Token_list*)allocate_space(1, sizeof(Token_list));
    Automata* test_automata = (Automata*)allocate_space(1, sizeof(Automata));
    
    // States to NIL
    char test_c = 'I';
    test_automata->state = s_N;
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_NI);
    test_c = 'L';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_NIL);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_nil);
    
    // States to WHILE
    test_c = 'W';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_W);
    test_c = 'H';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_WH);
    test_c = 'I';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_WHI);
    test_c = 'L';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_WHIL);
    test_c = 'E';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_WHILE);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_while);
    
    // states to IF
    test_c = 'I';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_I);
    test_c = 'F';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_IF);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_if);
    
    // states to PRINT
    test_c = 'P';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_P);
    test_c = 'R';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_PR);
    test_c = 'I';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_PRI);
    test_c = 'N';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_PRIN);
    test_c = 'T';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_PRINT);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_print);
    
    // states to PLUS
    test_c = 'P';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_P);
    test_c = 'L';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_PL);
    test_c = 'U';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_PLU);
    test_c = 'S';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_PLUS);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_plus);
    
    // states to SET
    test_c = 'S';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_S);
    test_c = 'E';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_SE);
    test_c = 'T';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_SET);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_set);
    
    // states to LENGTH
    test_c = 'L';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_L);
    test_c = 'E';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_LE);
    test_c = 'N';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_LEN);
    test_c = 'G';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_LENG);
    test_c = 'T';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_LENGT);
    test_c = 'H';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_LENGTH);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_length);
    
    // states to LESS
    test_c = 'L';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_L);
    test_c = 'E';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_LE);
    test_c = 'S';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_LES);
    test_c = 'S';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_LESS);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_less);
    
    // states to GREATER
    test_c = 'G';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_G);
    test_c = 'R';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_GR);
    test_c = 'E';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_GRE);
    test_c = 'A';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_GREA);
    test_c = 'T';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_GREAT);
    test_c = 'E';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_GREATE);
    test_c = 'R';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_GREATER);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_greater);
    
    // states to EQUAL
    test_c = 'E';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_E);
    test_c = 'Q';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_EQ);
    test_c = 'U';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_EQU);
    test_c = 'A';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_EQUA);
    test_c = 'L';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_EQUAL);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_equal);
    
    // states to CAR
    test_c = 'C';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_C);
    test_c = 'A';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_CA);
    test_c = 'R';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_CAR);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_CAR);
    
    // states to CDR
    test_c = 'C';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_C);
    test_c = 'D';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_CD);
    test_c = 'R';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_CDR);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_CDR);
    
    // states to CONS
    test_c = 'C';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_C);
    test_c = 'O';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_CO);
    test_c = 'N';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_CON);
    test_c = 'S';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_CONS);
    test_c = ' ';
    update_tokens(test_tokens, test_automata, test_c);
    assert(test_automata->state == s_start);
    assert(test_tokens->end->value->type == t_CONS);
    
    // void add_previous_chars(int n, Token_list* tokens, Automata* automata, char var, ...);
    
    
    // void make_and_add_simple_token(Token_list* tokens, Automata* automata, // token_type type);
    // void handle_start_state(Token_list* tokens, Automata* automata, char c);
    // void add_variable_token(Token_list* tokens, Automata* automata, char name);
    // void handle_in_state(Token_list* tokens, Automata* automata, char c);
    // void add_token(Token_list* tokens, Token* token);
    
    free_token_list(test_tokens);
    free(test_automata);
}
