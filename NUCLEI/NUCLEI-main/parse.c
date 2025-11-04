#include "nuclei.h"

void parse_list(Token_list* list) {
    Syntax_tree* tree = (Syntax_tree*)allocate_space(1, sizeof(Syntax_tree));
    Token_node* current = list->start;
    // printf("First token being considered is VVV\n"); // debugging
    // print_token(current); // debugging
    // printf("\n"); // debugging
    bool parses_correctly = true;
    tree->program = descend_recursively(&current, &parses_correctly);
    print_tree(tree); // debugging
    if (parses_correctly) {
        printf("Parsed OK\n");
        free_tree(tree);
    } else {
        fprintf(stderr, "PARSING ERROR - syntactically invalid NUCLEI program\n");
        free_tree(tree);
        exit(EXIT_FAILURE);
    }
}

Tree_node* descend_recursively(Token_node** current, bool* parses_correctly) {
    Tree_node* program = make_node(PROG);
    if (!next_token_is(current, 1, t_l_parenthesis)) {
        return parser_fails(parses_correctly);
    } else {
        program->child1 = handle_INSTRCTS(current, parses_correctly);
    }
    return program;
}

Tree_node* handle_INSTRCTS(Token_node** current, bool* parses_correctly) {
    if ((*current) == NULL) {
        return parser_fails(parses_correctly);
    }
    // printf("In INSTRCTS.  Considering token: "); // debugging
    // print_token((*current)); // debugging
    // printf("\n"); // debugging
    if (next_token_is(current, 1, t_r_parenthesis)) {
        return NULL;
    } else {
        Tree_node* instructions = make_node(INSTRCTS);
        instructions->child1 = handle_INSTRCT(current, parses_correctly);
        instructions->child2 = handle_INSTRCTS(current, parses_correctly);
        return instructions;
    }
}

Tree_node* handle_INSTRCT(Token_node** current, bool* parses_correctly) {
    if (!next_token_is(current, 1, t_l_parenthesis)) {
        return parser_fails(parses_correctly);
    }
    Tree_node* instruction = make_node(INSTRCT);
    instruction->child1 = handle_FUNC(current, parses_correctly);
    if (!next_token_is(current, 1, t_r_parenthesis)) {
        return parser_fails(parses_correctly);
    }
    return instruction;
}

Tree_node* handle_FUNC(Token_node** current, bool* parses_correctly) {
    Tree_node* function = make_node(FUNC);
    if (is_RETFUNC(*current)) {
        function->child1 = handle_RETFUNC(current, parses_correctly);
    } else if (is_IOFUNC(*current)) {
        function->child1 = handle_IOFUNC(current, parses_correctly);
    } else if (is_IF(*current)) {
        function->child1 = handle_IF(current, parses_correctly);
    } else if (is_LOOP(*current)) {
        function->child1 = handle_LOOP(current, parses_correctly);
    } else {
        return parser_fails(parses_correctly);
    }
    return function;
}

bool is_RETFUNC(Token_node* current) {
    return (is_LISTFUNC(current) || is_INTFUNC(current) || is_BOOLFUNC(current));
}

Tree_node* handle_RETFUNC(Token_node** current, bool* parses_correctly) {
    Tree_node* ret_function = make_node(RETFUNC);
    if (is_LISTFUNC(*current)) {
        ret_function->child1 = handle_LISTFUNC(current, parses_correctly);
    } else if (is_INTFUNC(*current)) {
        ret_function->child1 = handle_INTFUNC(current, parses_correctly);
    } else if (is_BOOLFUNC(*current)) {
        ret_function->child1 = handle_BOOLFUNC(current, parses_correctly);
    } else {
        return parser_fails(parses_correctly);
    }
    return ret_function;
}

bool is_IOFUNC(Token_node* current) {
    return (current->value->type == t_set || current->value->type == t_print);
}

Tree_node* handle_IOFUNC(Token_node** current, bool* parses_correctly) {
    Tree_node* io_function = make_node(IOFUNC);
    if ((*current)->value->type == t_set) {
        io_function->child1 = handle_SET(current, parses_correctly);
    } else if ((*current)->value->type == t_print) {
        io_function->child1 = handle_PRINT(current, parses_correctly);
    } else {
        return parser_fails(parses_correctly);
    }
    return io_function;
}

Tree_node* handle_SET(Token_node** current, bool* parses_correctly) {
    Tree_node* set = make_node(SET);
    if (!next_token_is(current, 1, t_set)) {
        return parser_fails(parses_correctly);
    } else {
        set->child1 = handle_VAR(current, parses_correctly);
        set->child2 = handle_LIST(current, parses_correctly);
        return set;
    }
}

bool is_IF(Token_node* current) {
    // print_token(current); // debugging
    // printf(" <- this is the current token\n"); // debugging
    return (current->value->type == t_if);
}

Tree_node* handle_IF(Token_node** current, bool* parses_correctly) {
    Tree_node* if_node = make_node(IF);
    if (!next_token_is(current, 1, t_if)) {
        return parser_fails(parses_correctly);
    }
    if (!next_token_is(current, 1, t_l_parenthesis)) {
        return parser_fails(parses_correctly);
    }
    if_node->child1 = handle_BOOLFUNC(current, parses_correctly);
    if (!next_token_is(current, 1, t_r_parenthesis)) {
        return parser_fails(parses_correctly);
    }
    if (!next_token_is(current, 1, t_l_parenthesis)) {
        return parser_fails(parses_correctly);
    }
    if_node->child2 = handle_INSTRCTS(current, parses_correctly);
    if (!next_token_is(current, 1, t_l_parenthesis)) {
        return parser_fails(parses_correctly);
    }
    if_node->child3 = handle_INSTRCTS(current, parses_correctly);
    return if_node;
}

bool is_LOOP(Token_node* current) {
    return (current->value->type == t_while);
}

Tree_node* handle_LOOP(Token_node** current, bool* parses_correctly) {
    Tree_node* loop = make_node(LOOP);
    if (!next_token_is(current, 1, t_while)) {
        return parser_fails(parses_correctly);
    }
    if (!next_token_is(current, 1, t_l_parenthesis)) {
        return parser_fails(parses_correctly);
    }
    loop->child1 = handle_BOOLFUNC(current, parses_correctly);
    if (!next_token_is(current, 1, t_r_parenthesis)) {
        return parser_fails(parses_correctly);
    }
    if (!next_token_is(current, 1, t_l_parenthesis)) {
        return parser_fails(parses_correctly);
    }
    loop->child2 = handle_INSTRCTS(current, parses_correctly);
    return loop;
}

bool is_LISTFUNC(Token_node* current) {
    token_type type = current->value->type;
    return (type == t_CAR || type == t_CDR || type == t_CONS);
}

Tree_node* handle_LISTFUNC(Token_node** current, bool* parses_correctly) {
    Tree_node* list_function = make_node(LISTFUNC);
    token_type type = (*current)->value->type;
    if (!next_token_is(current, 3, t_CAR, t_CDR, t_CONS)) {
        return parser_fails(parses_correctly);
    } else {
        list_function->func_type = type;
    }
    list_function->child1 = handle_LIST(current, parses_correctly);
    if (type == t_CONS) {
        list_function->child2 = handle_LIST(current, parses_correctly);
    }
    return list_function;
}
    

bool is_INTFUNC(Token_node* current) {
    token_type type = current->value->type;
    return (type == t_plus || type == t_length);
}

Tree_node* handle_INTFUNC(Token_node** current, bool* parses_correctly) {
    Tree_node* int_function = make_node(INTFUNC);
    token_type type = (*current)->value->type;
    if (!next_token_is(current, 2, t_plus, t_length)) {
        return parser_fails(parses_correctly);
    } else {
        int_function->func_type = type;
        int_function->child1 = handle_LIST(current, parses_correctly);
    }
    if (type == t_plus) {
        int_function->child2 = handle_LIST(current, parses_correctly);
    }
    return int_function;
}

bool is_BOOLFUNC(Token_node* current) {
    token_type type = current->value->type;
    return (type == t_less || type == t_greater || type == t_equal);
}

Tree_node* handle_BOOLFUNC(Token_node** current, bool* parses_correctly) {
    Tree_node* bool_function = make_node(BOOLFUNC);
    token_type type = (*current)->value->type;
    if (!next_token_is(current, 3, t_less, t_greater, t_equal)) {
        return parser_fails(parses_correctly);
    } else {
        bool_function->func_type = type;
        bool_function->child1 = handle_LIST(current, parses_correctly);
        bool_function->child2 = handle_LIST(current, parses_correctly);
    }
    return bool_function;
}

Tree_node* handle_LIST(Token_node** current, bool* parses_correctly) {
    Tree_node* list = make_node(LIST);
    token_type type = (*current)->value->type;
    if (type == t_variable) {
        list->child1 = handle_VAR(current, parses_correctly);
    } else if (type == t_literal) {
        list->child1 = handle_LITERAL(current, parses_correctly);
    } else if (type == t_nil) {
        list->child1 = handle_NIL(current, parses_correctly);
    } else if (next_token_is(current, 1, t_l_parenthesis)) {
        list->child1 = handle_RETFUNC(current, parses_correctly);
        if (!next_token_is(current, 1, t_r_parenthesis)) {
            return parser_fails(parses_correctly);
        }
    } else {
        return parser_fails(parses_correctly);
    }
    return list;
}

Tree_node* handle_VAR(Token_node** current, bool* parses_correctly) {
    Tree_node* var = make_node(VAR);
    var->var_name = (*current)->value->var_name;
    *current = (*current)->next;
    return var;
}

Tree_node* handle_LITERAL(Token_node** current, bool* parses_correctly)  {
    if ((*current)->value->type == t_literal) {
        Tree_node* literal = make_node(LITERAL);
        literal->string_value = (*current)->value->lexeme;
        *current = (*current)->next;
        return literal;
    } else {
        return parser_fails(parses_correctly);
    }
    
}

Tree_node* handle_NIL(Token_node** current, bool* parses_correctly)  {
    Tree_node* nil = make_node(NIL);
    *current = (*current)->next;
    return nil;
}

Tree_node* handle_PRINT(Token_node** current, bool* parses_correctly) {
    Tree_node* print = make_node(PRINT);
    if (!next_token_is(current, 1, t_print)) {
        return parser_fails(parses_correctly);
    } 
    if ((*current)->value->type == t_string) {
        print->child1 = handle_STRING(current, parses_correctly);
    } else if (is_LIST(*current)) {
        print->child1 = handle_LIST(current, parses_correctly);
    }
    return print;
}

bool is_LIST(Token_node* current) {
    token_type type = current->value->type;
    return (type == t_variable || type == t_literal || type == t_nil || type == t_l_parenthesis);
}

Tree_node* handle_STRING(Token_node** current, bool* parses_correctly) {
    if ((*current)->value->type == t_string) {
        Tree_node* string = make_node(STRING);
        string->string_value = (*current)->value->lexeme;
        *current = (*current)->next;
        return string;
    } else {
        return parser_fails(parses_correctly);
    }
}

Tree_node* make_node(grammar_type type) {
    Tree_node* new_node = (Tree_node*)allocate_space(1, sizeof(Tree_node));
    new_node->type = type;
    return new_node;
}

Tree_node* parser_fails(bool* parses_correctly) {
    *parses_correctly = false;
    return NULL;
}

bool next_token_is(Token_node** current, int num_possible_tokens, ...) {
    if ((num_possible_tokens < 1) || !(*current)) {
        return false;
    }
    va_list token_list;
    va_start(token_list, num_possible_tokens);
    for (int i = 0; i < num_possible_tokens; i++) {
        token_type expected = va_arg(token_list, token_type);
        if ((*current)->value->type == expected) {
            *current = (*current)->next;
            return true;
        }
    }
    return false;
}

void print_tree(Syntax_tree* tree) {
    if (tree->program) {
        print_tree_node(tree->program);
    } else {
        printf("EMPTY TREE");
    }
    printf("\n");
}

void print_tree_node(Tree_node* node) {
    char* node_type = get_node_type(node);
    printf("(%s", node_type);
    if (node->child1) {
        print_tree_node(node->child1);
    } else {
        printf("()"); 
    }
    if (node->child2) {
        print_tree_node(node->child2);
    }
    if (node->child3) {
        print_tree_node(node->child3);
    }
    printf(")");
}

char* get_node_type(Tree_node* node) {
    switch (node->type) {
        case PROG:
            return "PROG";
        case INSTRCTS:
            return "INSTRCTS";
        case INSTRCT:
            return "INSTRCT";
        case FUNC:
            return "FUNC";
        case RETFUNC:
            return "RETFUNC";
        case LISTFUNC:
            return "LISTFUNC";
        case INTFUNC:
            return "INTFUNC";
        case BOOLFUNC:
            return "BOOLFUNC";
        case IOFUNC:
            return "IOFUNC";
        case SET:
            return "SET";
        case PRINT:
            return "PRINT";
        case IF:
            return "IF";
        case LOOP:
            return "LOOP";
        case LIST:
            return "LIST";
        case VAR:
            return "VAR";
        case STRING:
            return "STRING";
        case LITERAL:
            return "LITERAL";
        case NIL:
            return "NIL";
        default:
            return "null";
    }
}

void free_tree(Syntax_tree* tree) {
    if (tree->program) {
        free_node(tree->program);
    }
    free(tree);
}

void free_node(Tree_node* node) {
    if (node->child1) {
        free_node(node->child1);
        node->child1 = NULL;
    }
    if (node->child2) {
        free_node(node->child2);
        node->child2 = NULL;
    }
    if (node->child3) {
        free_node(node->child3);
        node->child3 = NULL;
    }
    free(node);
}