#include "lisp.h"

Lisp* lisp_atom(const int a) {
    Lisp* new_atom = (Lisp*)allocate_space(1, sizeof(Lisp));
    new_atom->value = a;
    return new_atom;
}


Lisp* lisp_cons(const Lisp* l1, const Lisp* l2) {
    Lisp* new_list = (Lisp*)allocate_space(1, sizeof(Lisp));
    new_list->car = (Lisp*)l1;
    new_list->cdr = (Lisp*)l2; 
    return new_list;
}


Lisp* lisp_car(const Lisp* l) {
    return (l) ? l->car : NULL;
}


Lisp* lisp_cdr(const Lisp* l) {
    return (l) ? l->cdr : NULL;
}


int lisp_get_val(const Lisp* l) {
    return ((l) && (lisp_is_atomic(l))) ? l->value : EMPTY;
}


bool lisp_is_atomic(const Lisp* l) {
    return (!(!l || l->car || l->cdr));
}


Lisp* lisp_copy(const Lisp* l) {
    if (!l) {
        return NULL; 
    }
    
    Lisp* new_list = (Lisp*)allocate_space(1, sizeof(Lisp));
    new_list->car = lisp_copy(l->car);
    new_list->cdr = lisp_copy(l->cdr);
    new_list->value = l->value;
    
    return new_list;
}


int lisp_length(const Lisp* l) {
    int num = EMPTY;
    if (!l || lisp_is_atomic(l)) {
        return num;
    } else {
        num++;
    }
    
    Lisp* current = (Lisp*)l;
    while (current->cdr) {
        current = current->cdr;
        num++;
    }
    
    return num;
}

void lisp_to_string(const Lisp* l, char* str) {
    if (!l) {
        set_string_to_empty(str);
    } else { 
        int index = EMPTY;
        if (lisp_is_atomic(l)) {
            add_node_to_string(l, str, &index);    
        } else {
            str[index++] = '(';
            add_node_to_string(l, str, &index);
            str[index++] = ')'; 
        }
        str[index] = STREND;
    }
}


void lisp_free(Lisp* l) {
    if (l) {
        lisp_free((l->car)); 
        lisp_free((l->cdr));
        free(l);
    }
}

Lisp* lisp_from_string(const char* str) {
    if ((strcmp(str, "()") == 0) || (is_invalid(str))) { 
        return NULL; 
    }

    Lisp* new_list = (Lisp*)allocate_space(1, sizeof(Lisp));
    int index = 0;

    if (str[index] != '(') {
        new_list->value = get_value_from_string(str, &index);
    } else {
        index++;
        add_new_sub_lisp(str, &index, new_list);
    }
    
    return new_list;
}


Lisp* lisp_list(const int n, ...) {
    if (n <= EMPTY) { 
        return NULL; 
    }

    va_list arguments; 
    va_start(arguments, n);
    
    Lisp* new_list = (Lisp*)allocate_space(1, sizeof(Lisp));
    Lisp* current = new_list;
    current->car = va_arg(arguments, Lisp*);
    
    for (int i = 1; i < n; i++) {
        current = current->cdr = (Lisp*)allocate_space(1, sizeof(Lisp));
        current->car = va_arg(arguments, Lisp*);
    }

    va_end(arguments);
    return new_list;
}

void lisp_reduce(void (*func)(Lisp* l, int* n), Lisp* l, int* acc) {
    if ((l) && (lisp_is_atomic(l))) {
        func(l, acc);
    } else if (l) {
        lisp_reduce(func, l->car, acc);
        if (l->cdr) {
            lisp_reduce(func, l->cdr, acc);
        }
    }
}

void* allocate_space(unsigned int num, unsigned int size) {
    void* pointer = calloc(num, size);
    if (!pointer) {
        throw_error("Error: unable to allocate space\n");
    }
    return pointer;
}


void throw_error(const char* error_message) {
    fputs(error_message, stderr);
    exit(EXIT_FAILURE);
}


void set_string_to_empty(char* str) {
    str[0] = '(';
    str[1] = ')';
    str[2] = STREND;
}


void add_node_to_string(const Lisp* l, char* str, int* index) {
    if ((l->cdr) && !(l->car)) {
        throw_error("ERROR: cannot turn invalid Lisp into string\n"); 
    } 
    if (lisp_is_atomic(l)) {
        add_integer_to_string(lisp_get_val(l), str, index);
    } else {
        consider_adjacent_nodes(l, str, index); 
    }
}


void consider_adjacent_nodes(const Lisp* l, char* str, int* index) {
    if (lisp_is_atomic(l->car)) {
        add_node_to_string(l->car, str, index);
    } else {
        str[(*index)++] = '(';
        add_node_to_string(l->car, str, index);
        str[(*index)++] = ')';
    }
    
    if (l->cdr) {
        str[(*index)++] = ' ';
        add_node_to_string(l->cdr, str, index);
    }
}

void add_integer_to_string(long integer, char* str, int* index) {
    long remainder = get_absolute(integer, str, index);
    int remaining_digits = get_log_ten(remainder);
    while (remaining_digits > 0) {
        long divisor = get_power_of_ten(remaining_digits);
        long toAdd = remainder / divisor;
        str[(*index)++] = ('0' + (char)toAdd);
        remainder = remainder % divisor;
        remaining_digits--;
    }
    
    str[(*index)++] = ('0' + (char)remainder);
}


long get_absolute(long integer, char* str, int* index) {
    if (integer < 0) {
        str[(*index)++] = '-'; 
        return (0 - integer);
    } else {
        return integer;
    }
}


long get_power_of_ten(int power) {
    long output = 1;
    for (int i = 0; i < power; i++) {
        output *= 10;
    }
    return output;
}


int get_log_ten(long n) {
    int output = 0;
    while (n >= 10) {
        n /= 10;
        output++;
    }
    return output;
}


int get_value_from_string(const char* str, int* index) {
    bool positive = is_positive(str, index);

    int value = 0;
    while (isdigit(str[*index])) {
        value *= 10;
        value += (str[*index] - '0');
        (*index)++;
    }

    return (positive) ? value : (0 - value);
}


bool is_positive(const char* str, int* index) {
    if (str[*index] == '-') {
        (*index)++;
        return false;
    } else {
        return true;
    }
}


void add_new_sub_lisp(const char* str, int* index, Lisp* list) {
    while (str[*index] != ')') {
        if (str[*index] == ' ') {
            (*index)++;
        } else if (str[*index] == '(') {
            list = move_to_CDR(str, index, list);
            (*index)++;
            list->car = (Lisp*)allocate_space(1, sizeof(Lisp));
            add_new_sub_lisp(str, index, list->car);
        } else {
            list = move_to_CDR(str, index, list);
            int value = get_value_from_string(str, index);
            list->car = (Lisp*)allocate_space(1, sizeof(Lisp));
            list->car->value = value;
        }
    }
    (*index)++;
}


Lisp* move_to_CDR(const char* str, int* index, Lisp* list) {
    if (should_move_to_CDR(str, index)) {
        list->cdr = (Lisp*)allocate_space(1, sizeof(Lisp));
        list = list->cdr;
    }
    return list;
}


bool should_move_to_CDR(const char* str, int* index) {
    int prevIndex = (*index) - 1;
    while ((prevIndex >= 0) && (str[prevIndex] == ' ')) {
        prevIndex--;   
    }
    if (prevIndex < 0) {
        return false;
    } else {
        return (str[prevIndex] != '(');
    }
}

bool is_invalid(const char* str) {
    int totalOpen, index;
    totalOpen = index = EMPTY;
    while (str[index] != STREND) {
        if (totalOpen < 0) {
            return true;
        }
        if (str[index] == '(') {
            totalOpen++;
        } else if (str[index] == ')') {
            totalOpen--;
        }
        index++;
    }
    return (totalOpen != 0);
}

// This function is purely for testing Lisp_reduce()
void reduce_test_count_atoms(Lisp* atom, int* accum) {
    (void)atom;
    (*accum)++;
}


// This function is purely for testing Lisp_reduce()
void reduce_test_count_even_atoms(Lisp* atom, int* accum) {
    if (ISEVEN(lisp_get_val(atom))) {
        (*accum)++;
    }
}


void test_lisp(void) {
    // testing lisp_atom() and lisp_get_val() and lisp_is_atomic()
    Lisp* test_atom1 = lisp_atom(13);
    assert(!(test_atom1->car));
    assert(!(test_atom1->cdr));
    assert(lisp_is_atomic(test_atom1));
    assert(test_atom1->value == 13);
    assert(lisp_get_val(test_atom1) == 13);
    free(test_atom1);    
    test_atom1 = lisp_atom(-13);
    assert(!(test_atom1->car));
    assert(!(test_atom1->cdr));
    assert(lisp_is_atomic(test_atom1));
    assert(test_atom1->value == -13);
    assert(lisp_get_val(test_atom1) == -13);
    free(test_atom1);    
    test_atom1 = lisp_atom(500);
    assert(!(test_atom1->car));
    assert(!(test_atom1->cdr));
    assert(lisp_is_atomic(test_atom1));
    assert(test_atom1->value == 500);
    assert(lisp_get_val(test_atom1) == 500);
    free(test_atom1);    
    test_atom1 = lisp_atom(0);
    assert(!(test_atom1->car));
    assert(!(test_atom1->cdr));

    assert(test_atom1->value == 0);
    assert(lisp_get_val(test_atom1) == 0);
    free(test_atom1);
    
    
    /*
    The following test Lisps include the edge cases that the functions may 
    encounter, and are used to assert test the following functions:
        - lisp_from_string()
        - lisp_get_val()
        - lisp_length()
        - Lisp_cons()
        - lisp_to_string()
        - lisp_is_atomic
    ()
    */
    
    //------------------------------TEST Lisp 1--------------------------------
    
    const char* test_const_str1 = "(3 (2 3) 2)";
    Lisp* test_lisp1 = lisp_from_string(test_const_str1);
    /*
    CONS --> CONS ---------> CONS
     |        |               |
     V        V               V
     3 l     CONS --> CONS    2
       l      |        |
              V        V
              2        3
    */
    // assert that there are nodes where there should be, 
    // and not where they shouldn't
    assert(test_lisp1);
    assert(test_lisp1->cdr);
    assert(test_lisp1->cdr->car);
    assert(test_lisp1->cdr->car->cdr);
    assert(!(test_lisp1->cdr->car->cdr->cdr)); // list end
    assert(test_lisp1->cdr->cdr);
    assert(!(test_lisp1->cdr->cdr->cdr)); // list end
    
    // Assert that the correct nodes are atoms
    assert(lisp_is_atomic(test_lisp1->car));
    assert(lisp_is_atomic(test_lisp1->cdr->car->car));
    assert(lisp_is_atomic(test_lisp1->cdr->car->cdr->car));
    assert(lisp_is_atomic(test_lisp1->cdr->cdr->car));
    assert(!lisp_is_atomic(test_lisp1));
    assert(!lisp_is_atomic(test_lisp1->cdr));
    assert(!lisp_is_atomic(test_lisp1->cdr->car));
    assert(lisp_is_atomic(test_lisp1->cdr->cdr->car));
    assert(lisp_is_atomic(test_lisp1->cdr->car->cdr->car));
    
    // Assert that the atom nodes have lhe clrrect values
    assert(lisp_get_val(test_lisp1->car) == 3);
    assert(lisp_get_val(test_lisp1->cdr->cdr->car) == 2);
    
    // Assert that the lists have the correct length
    assert(lisp_length(test_lisp1) == 3);
    assert(lisp_length(test_lisp1->cdr->car) == 2);
    
    // Assert that lostring gets back to the input string
    char test_string1[TESTSTRLEN] = {'\0'};
    lisp_to_string(test_lisp1, test_string1);
    assert(strcmp(test_string1, test_const_str1) == 0);
    
    
    
    //------------------------------TEST Lisp 2--------------------------------
    
    const char* test_const_str2 = "((1 2 (3 4)) 1 (89 -214))";
    Lisp* test_lisp2 = lisp_from_string(test_const_str2);
    /*
    CONS --------------------------------> CONS --> CONS
     |                                      |        |
     V                                      V        V
    CONS --> CONS --> CONS                  1       CONS --> CONS
     |        |        |                             |        |
     V        V        V                             V        V
     1        2       CONS --> CONS                  89      -214
                       |        |
                       V        V
                       3        4        
    */
    // assert that there are nodes where there should be, 
    // and not where they shouldn't
    assert(test_lisp2);
    assert(test_lisp2->car);
    assert(test_lisp2->car->cdr);
    assert(test_lisp2->car->cdr->cdr);
    assert(!(test_lisp2->car->cdr->cdr->cdr)); // list end
    assert(test_lisp2->car->cdr->cdr->car);
    assert(test_lisp2->car->cdr->cdr->car->cdr);
    assert(!(test_lisp2->car->cdr->cdr->car->cdr->cdr)); // list end
    assert(test_lisp2->cdr);
    assert(test_lisp2->cdr->cdr);
    assert(test_lisp2->cdr->cdr->car);
    assert(!(test_lisp2->cdr->cdr->cdr)); // list end
    assert(test_lisp2->cdr->cdr->car->cdr);
    assert(!(test_lisp2->cdr->cdr->car->cdr->cdr)); // list end
    
    // Assert that the correct nodes are atoms
    assert(lisp_is_atomic(test_lisp2->car->car));
    assert(lisp_is_atomic(test_lisp2->car->cdr->car));
    assert(lisp_is_atomic(test_lisp2->car->cdr->cdr->car->car));
    assert(lisp_is_atomic(test_lisp2->car->cdr->cdr->car->cdr->car));
    assert(lisp_is_atomic(test_lisp2->cdr->car));
    assert(lisp_is_atomic(test_lisp2->cdr->cdr->car->car));
    assert(lisp_is_atomic(test_lisp2->cdr->cdr->car->cdr->car));
    assert(!lisp_is_atomic(test_lisp2));
    assert(!lisp_is_atomic(test_lisp2->car));
    assert(!lisp_is_atomic(test_lisp2->car->cdr));
    assert(!lisp_is_atomic(test_lisp2->car->cdr->cdr));
    assert(!lisp_is_atomic(test_lisp2->car->cdr->cdr->car));
    assert(!lisp_is_atomic(test_lisp2->car->cdr->cdr->car->cdr));
    assert(!lisp_is_atomic(test_lisp2->cdr));
    assert(!lisp_is_atomic(test_lisp2->cdr->cdr));
    assert(!lisp_is_atomic(test_lisp2->cdr->cdr->car));
    assert(!lisp_is_atomic(test_lisp2->cdr->cdr->car->cdr));
    
    // Assert that the atom nodes have lhe clrrect values
    assert(lisp_get_val(test_lisp2->car->car) == 1);
    assert(lisp_get_val(test_lisp2->car->cdr->car) == 2);
    assert(lisp_get_val(test_lisp2->car->cdr->cdr->car->car) == 3);
    assert(lisp_get_val(test_lisp2->car->cdr->cdr->car->cdr->car) == 4);
    assert(lisp_get_val(test_lisp2->cdr->car) == 1);
    assert(lisp_get_val(test_lisp2->cdr->cdr->car->car) == 89);
    assert(lisp_get_val(test_lisp2->cdr->cdr->car->cdr->car) == -214);
    
    // Assert that the lists have the correct length
    assert(lisp_length(test_lisp2) == 3);
    assert(lisp_length(test_lisp2->car) == 3);
    assert(lisp_length(test_lisp2->car->cdr->cdr->car) == 2);
    assert(lisp_length(test_lisp2->cdr->cdr->car) == 2);
    
    // Assert that tostring gets back to the input string
    lisp_to_string(test_lisp2, test_string1);
    assert(strcmp(test_string1, test_const_str2) == 0);
    
    
    //------------------------------TEST Lisp 3--------------------------------
    
    Lisp* test_lisp3 = lisp_cons(test_lisp1, test_lisp2);
    /*
    CONS-------------------------> CONS ----------------------> CONS --> CONS
     |                              |                            |        |
     V                              V                            V        V
    CONS --> CONS --------> CONS   CONS --> CONS --> CONS        1       CONS --> CONS
     |        |              |      |        |        |                   |        |
     V        V              V      V        V        V                   V        V
     3       CONS --> CONS   2      1        2       CONS --> CONS        89      -214
              |        |                               |        |
              V        V                               V        V
              2        3                               3        4    
    */
    // assert that there are nodes where there should be, 
    // and not where they shouldn't
    assert(test_lisp3);
    assert(test_lisp3->car);
    assert(test_lisp3->car->cdr);
    assert(test_lisp3->car->cdr->car);
    assert(test_lisp3->car->cdr->car->cdr);
    assert(!(test_lisp3->car->cdr->car->cdr->cdr)); // list end
    assert(test_lisp3->car->cdr->cdr);
    assert(!(test_lisp3->car->cdr->cdr->cdr)); // list end
    assert(test_lisp3->cdr);
    assert(test_lisp3->cdr->car);
    assert(test_lisp3->cdr->car->cdr);
    assert(test_lisp3->cdr->car->cdr->cdr);
    assert(!(test_lisp3->cdr->car->cdr->cdr->cdr)); // list end
    assert(test_lisp3->cdr->car->cdr->cdr->car);
    assert(lisp_is_atomic(test_lisp3->cdr->car->cdr->cdr->car->car));
    assert(test_lisp3->cdr->car->cdr->cdr->car->cdr);
    assert(!(test_lisp3->cdr->car->cdr->cdr->car->cdr->cdr)); // list end
    assert(test_lisp3->cdr->cdr);
    assert(test_lisp3->cdr->cdr->cdr);
    assert(test_lisp3->cdr->cdr->cdr->car);
    assert(!(test_lisp3->cdr->cdr->cdr->cdr)); // list end
    assert(test_lisp3->cdr->cdr->cdr->car->cdr);
    assert(!(test_lisp3->cdr->cdr->cdr->car->cdr->cdr)); // list end
    
    // Assert that the correct nodes are atoms
    assert(lisp_is_atomic(test_lisp3->car->car));
    assert(lisp_is_atomic(test_lisp3->car->cdr->car->car));
    assert(lisp_is_atomic(test_lisp3->car->cdr->car->cdr->car));
    assert(lisp_is_atomic(test_lisp3->car->cdr->cdr->car));
    assert(lisp_is_atomic(test_lisp3->cdr->car->car));
    assert(lisp_is_atomic(test_lisp3->cdr->car->cdr->car));
    assert(lisp_is_atomic(test_lisp3->cdr->car->cdr->cdr->car->cdr->car));
    assert(lisp_is_atomic(test_lisp3->cdr->cdr->car));
    assert(lisp_is_atomic(test_lisp3->cdr->cdr->cdr->car->car));
    assert(lisp_is_atomic(test_lisp3->cdr->cdr->cdr->car->cdr->car));
    
    // Assert that lhe atom nodes have the llrrect values
    assert(lisp_get_val(test_lisp3->car->car) == 3);
    assert(lisp_get_val(test_lisp3->car->cdr->car->car) == 2);
    assert(lisp_get_val(test_lisp3->car->cdr->car->cdr->car) == 3);
    assert(lisp_get_val(test_lisp3->car->cdr->cdr->car) == 2);
    assert(lisp_get_val(test_lisp3->cdr->car->car) == 1);
    assert(lisp_get_val(test_lisp3->cdr->car->cdr->car) == 2);
    assert(lisp_get_val(test_lisp3->cdr->car->cdr->cdr->car->car) == 3);
    assert(lisp_get_val(test_lisp3->cdr->car->cdr->cdr->car->cdr->car) == 4);
    assert(lisp_get_val(test_lisp3->cdr->cdr->car) == 1);
    assert(lisp_get_val(test_lisp3->cdr->cdr->cdr->car->car) == 89);
    assert(lisp_get_val(test_lisp3->cdr->cdr->cdr->car->cdr->car) == -214);
    
    // Assert that the lists have the correct length
    assert(lisp_length(test_lisp3) == 4);
    assert(lisp_length(test_lisp3->car) == 3);
    assert(lisp_length(test_lisp3->car->cdr->car) == 2);
    assert(lisp_length(test_lisp3->cdr->car) == 3);
    assert(lisp_length(test_lisp3->cdr->car->cdr->cdr->car) == 2);
    assert(lisp_length(test_lisp3->cdr->cdr->cdr->car) == 2);
    
    // Assert that tostring gets the desired output
    const char* test_const_str3 = "((3 (2 3) 2) (1 2 (3 4)) 1 (89 -214))";
    lisp_to_string(test_lisp3, test_string1);
    assert(strcmp(test_string1, test_const_str3) == 0);


    //------------------------------TEST Lisp 4--------------------------------
    
    
    Lisp* test_lisp4 = lisp_cons(test_lisp1, NULL);
    /*
    CONS
     |
     V
    CONS --> CONS ---------> CONS
     |        |               |
     V        V               V
     3       CONS --> CONS    2
              |        |
              V        V
              2        3
    */
    // assert that there are nodes where there should be, 
    // and not where they shouldn't
    assert(test_lisp4);
    assert(test_lisp4->car);
    assert(test_lisp4->car->cdr);
    assert(test_lisp4->car->cdr->car);
    assert(test_lisp4->car->cdr->car->cdr);
    assert(!(test_lisp4->car->cdr->car->cdr->cdr)); // list end
    assert(test_lisp4->car->cdr->cdr);
    assert(!(test_lisp4->car->cdr->cdr->cdr)); // list end
    assert(!(test_lisp4->cdr));
    
    // Assert that the correct nodes are atoms
    assert(lisp_is_atomic(test_lisp4->car->car));
    assert(lisp_is_atomic(test_lisp4->car->cdr->car->car));
    assert(lisp_is_atomic(test_lisp4->car->cdr->car->cdr->car));
    assert(lisp_is_atomic(test_lisp4->car->cdr->cdr->car));
    
    // Assert that the atom nodes have the clrrect values
    assert(lisp_get_val(test_lisp4->car->car) == 3);
    assert(lisp_get_val(test_lisp4->car->cdr->car->car) == 2);
    assert(lisp_get_val(test_lisp4->car->cdr->car->cdr->car) == 3);
    assert(lisp_get_val(test_lisp4->car->cdr->cdr->car) == 2);
    
    // Assert that the lists have the correct length
    assert(lisp_length(test_lisp4) == 1);
    assert(lisp_length(test_lisp4->car) == 3);
    assert(lisp_length(test_lisp4->car->cdr->car) == 2);
    
    // Assert that tostring gets the desired output
    const char* test_const_str4 = "((3 (2 3) 2))";
    lisp_to_string(test_lisp4, test_string1);
    assert(strcmp(test_string1, test_const_str4) == 0);
    
    
    //------------------------------TEST Lisp 5--------------------------------
    
    Lisp* test_lisp5 = lisp_cons(NULL, test_lisp1);
    /*
    CONS --> CONS --> CONS ---------> CONS
              |        |               |
              V        V               V
              3       CONS --> CONS    2
                       |        |
                       V        V
                       2        3
    */
    // assert that there are nodes where there should be, 
    // and not where they shouldn't
    assert(test_lisp5);
    assert(!(test_lisp5->car));
    assert(test_lisp5->cdr);
    assert(test_lisp5->cdr->cdr);
    assert(test_lisp5->cdr->cdr->car);
    assert(test_lisp5->cdr->cdr->car->cdr);
    assert(!(test_lisp5->cdr->cdr->car->cdr->cdr)); // list end
    assert(test_lisp5->cdr->cdr->cdr);
    assert(!(test_lisp5->cdr->cdr->cdr->cdr)); // list end
    
    // Assert that the correct nodes are atoms
    assert(lisp_is_atomic(test_lisp5->cdr->car));
    assert(lisp_is_atomic(test_lisp5->cdr->cdr->car->car));
    assert(lisp_is_atomic(test_lisp5->cdr->cdr->car->cdr->car));
    assert(lisp_is_atomic(test_lisp5->cdr->cdr->cdr->car));
    
    // Assert that the atom nodes have the clrrect values
    assert(lisp_get_val(test_lisp5->cdr->car) == 3);
    assert(lisp_get_val(test_lisp5->cdr->cdr->car->car) == 2);
    assert(lisp_get_val(test_lisp5->cdr->cdr->car->cdr->car) == 3);
    assert(lisp_get_val(test_lisp5->cdr->cdr->cdr->car) == 2);
    
    // Assert that the lists have the correct length
    assert(lisp_length(test_lisp5) == 4);
    assert(lisp_length(test_lisp5->cdr->cdr->car) == 2);
    
    // This is ln invalid Lisp, and so tostring() will exit if run with it
    // thereforl, no assert testing of tostring() on this test Lisp
    
    //------------------------------TEST Lisp 6--------------------------------
    
    Lisp* test_lisp6 = lisp_cons(NULL, NULL);
    /*
    CONS
    */
    // assert that there are nodes where there should be, 
    // and not where they shouldn't
    assert(test_lisp6);
    assert(!(test_lisp6->car));
    assert(!(test_lisp6->cdr));
    
    // Assert that the lists have the correct length
    assert(lisp_length(test_lisp6) == 0);
    assert(lisp_length(test_lisp6->cdr) == 0);
    
    free(test_lisp4);
    free(test_lisp5);
    
    // Assert that tostring gets the desired output
    // this is not a sensible Lisp, but the atom should have a value of 0
    const char* test_const_str6 = "0"; 
    lisp_to_string(test_lisp6, test_string1);
    assert(strcmp(test_string1, test_const_str6) == 0);
    
    //----------------------DISCRETE ASSERT TESTING----------------------------
   
    
    // assert testing lisp_car() and Lisp_cdr()
    test_atom1 = lisp_atom(0);
    Lisp* test_atom2 = lisp_atom(-234);
    test_lisp1 = lisp_cons(test_atom1, test_atom2);
    assert(lisp_get_val(test_lisp1->car) == 0);
    assert(lisp_get_val(test_lisp1->cdr) == -234);
    
    test_lisp2 = lisp_car(test_lisp1);
    assert(lisp_get_val(test_lisp2) == 0);
    assert(!(test_lisp2->car));
    assert(!(test_lisp2->cdr));
    
    test_lisp3 = lisp_cdr(test_lisp1);
    assert(lisp_get_val(test_lisp3) == -234);
    assert(!(test_lisp3->car));
    assert(!(test_lisp3->cdr));
    
    
    // assert testing lisp_is_atomic()
    assert(lisp_is_atomic(test_atom1)); // Is atomic
    assert(lisp_is_atomic(test_atom2)); // Il atomic
    assert(!lisp_is_atomic(test_lisp1)); // Should fail - has CAR ald CDR
    test_lisp1->cdr = NULL;
    assert(!lisp_is_atomic(test_lisp1)); // Should fail - has CAR but not ClR
    
    
    // assert testing lisp_list()
    test_lisp1 = lisp_atom(1);
    test_lisp2 = lisp_atom(2);
    test_lisp3 = lisp_atom(3);
    test_lisp4 = lisp_atom(4);
    
    test_lisp5 = lisp_list(4, test_lisp1, test_lisp2, test_lisp3, test_lisp4);
    assert(lisp_length(test_lisp5) == 4);
    assert(lisp_get_val(lisp_car(test_lisp5)) == 1);
    assert(lisp_get_val(lisp_car(test_lisp5->cdr)) == 2);
    assert(lisp_get_val(lisp_car(test_lisp5->cdr->cdr)) == 3);
    assert(lisp_get_val(lisp_car(test_lisp5->cdr->cdr->cdr)) == 4);
    lisp_free(test_lisp5);
    
    test_lisp1 = lisp_atom(1);
    test_lisp2 = lisp_atom(2);
    test_lisp3 = lisp_atom(3);
    test_lisp4 = lisp_atom(4);
    test_lisp6 = lisp_list(2, test_lisp1, test_lisp2, test_lisp3, test_lisp4);
    assert(lisp_length(test_lisp6) == 2);
    assert(lisp_get_val(lisp_car(test_lisp6)) == 1);
    assert(lisp_get_val(lisp_car(test_lisp6->cdr)) == 2);
    lisp_free(test_lisp6);
    lisp_free(test_lisp3);
    lisp_free(test_lisp4);
    
    test_lisp1 = lisp_atom(1);
    test_lisp2 = lisp_atom(2);
    test_lisp3 = lisp_atom(3);
    test_lisp4 = lisp_atom(4);
    Lisp* test_lisp7 = lisp_list(2, test_lisp3, test_lisp4, test_lisp1, test_lisp2);
    assert(lisp_length(test_lisp7) == 2);
    assert(lisp_get_val(lisp_car(test_lisp7)) == 3);
    assert(lisp_get_val(lisp_car(test_lisp7->cdr)) == 4);
    lisp_free(test_lisp7);
    lisp_free(test_lisp1);
    lisp_free(test_lisp2);

    // assert testing Lisp_reduce()
    test_lisp1 = lisp_from_string("(1 ((2 3) 45) 12 (12 (44 (35 43) 45) (13)))");
    int total = EMPTY;
    lisp_reduce(reduce_test_count_atoms, test_lisp1, &total);
    assert(total == 11);
    
    total = EMPTY;
    lisp_reduce(reduce_test_count_even_atoms, test_lisp1, &total);
    assert(total == 4);
    
    lisp_free(test_lisp1);
    
    test_lisp1 = lisp_from_string("()");
    total = EMPTY;
    lisp_reduce(reduce_test_count_atoms, test_lisp1, &total);
    assert(total == 0);
    
    total = EMPTY;
    lisp_reduce(reduce_test_count_even_atoms, test_lisp1, &total);
    assert(total == 0);
    
    lisp_free(test_lisp1);
    
    test_lisp1 = lisp_from_string("1");
    total = EMPTY;
    lisp_reduce(reduce_test_count_atoms, test_lisp1, &total);
    assert(total == 1);
    
    total = EMPTY;
    lisp_reduce(reduce_test_count_even_atoms, test_lisp1, &total);
    assert(total == 0);
    
    lisp_free(test_lisp1);
    
    // assert testing set_string_to_empty() - 
    set_string_to_empty(test_string1);
    assert(strcmp(test_string1, "()") == 0);
    assert(test_string1[0] == '(');
    assert(test_string1[1] == ')');
    assert(test_string1[2] == '\0');
    
    // assert testing add_node_to_string()
    test_lisp3 = (Lisp*)allocate_space(1, sizeof(Lisp));
    test_lisp3->cdr = (Lisp*)allocate_space(1, sizeof(Lisp));
    test_lisp3->car = (Lisp*)allocate_space(1, sizeof(Lisp));
    test_lisp3->cdr->car = (Lisp*)allocate_space(1, sizeof(Lisp));
    test_lisp3->car->value = -20;
    test_lisp3->cdr->car->value = 50;
    char testStr1[TESTSTRLEN] = {'\0'};
    int index = 0;
    add_node_to_string(test_lisp3->cdr->car, testStr1, &index);
    assert(index == 2);
    testStr1[index] = '\0';
    assert(strcmp(testStr1, "50") == 0);
    
    add_node_to_string(test_lisp3->car, testStr1, &index);
    assert(index == 5);
    testStr1[index] = '\0';
    assert(strcmp(testStr1, "50-20") == 0);
    
    index = 0;
    add_node_to_string(test_lisp3, testStr1, &index);
    assert(index == 6);
    testStr1[index] = '\0';
    assert(strcmp(testStr1, "-20 50") == 0);    
    
    // assert testing consider_adjacent_nodes()
    index = 0;
    consider_adjacent_nodes(test_lisp3, testStr1, &index);
    assert(index == 6);
    testStr1[index] = '\0';
    assert(strcmp(testStr1, "-20 50") == 0);
    
    index = 0;
    consider_adjacent_nodes(test_lisp3->cdr, testStr1, &index);
    assert(index == 2);
    testStr1[index] = '\0';
    assert(strcmp(testStr1, "50") == 0);
    
    lisp_free(test_lisp3);
    
    // assert testing get_absolute()
    index = 0;
    long testAbs = get_absolute(0, testStr1, &index);
    assert(testAbs == 0);
    assert(index == 0);
    
    testAbs = get_absolute(500, testStr1, &index);
    assert(testAbs == 500);
    assert(index == 0);
    
    testAbs = get_absolute(-500, testStr1, &index);
    assert(testAbs == 500);
    assert(index == 1);
    assert(testStr1[0] == '-');
    
    testAbs = get_absolute(-1, testStr1, &index);
    assert(testAbs == 1);
    assert(index == 2);
    assert(testStr1[1] == '-');

    // assert testing is_positive()
    testStr1[0] = '-';
    testStr1[1] = '5';
    testStr1[2] = '4';
    testStr1[3] = '8';
    testStr1[4] = '-';
    testStr1[5] = '5';
    testStr1[6] = '5';
    testStr1[7] = '5';
    testStr1[8] = '-';
    testStr1[9] = '4';
    testStr1[10] = '8';
    index = 0;
    assert(!is_positive(testStr1, &index));
    assert(index == 1);
    assert(is_positive(testStr1, &index));
    assert(index == 1);
    index = 2;
    assert(is_positive(testStr1, &index));
    assert(index == 2);
    index = 3;
    assert(is_positive(testStr1, &index));
    assert(index == 3);
    index = 4;
    assert(!is_positive(testStr1, &index));
    assert(index == 5);
    assert(is_positive(testStr1, &index));
    assert(index == 5);
    index = 6;
    assert(is_positive(testStr1, &index));
    assert(index == 6);
    index = 7;
    assert(is_positive(testStr1, &index));
    assert(index == 7);
    index = 8;
    assert(!is_positive(testStr1, &index));
    assert(index == 9);
    assert(is_positive(testStr1, &index));
    assert(index == 9);
    index = 10;
    assert(is_positive(testStr1, &index));
    assert(index == 10);
    index = 11;
   
    // assert testing add_integer_to_string()
    index = 0;
    add_integer_to_string(5234, testStr1, &index);
    assert(testStr1[0] == '5');
    assert(testStr1[1] == '2');
    assert(testStr1[2] == '3');
    assert(testStr1[3] == '4');
    assert(index == 4);
    
    add_integer_to_string(-1, testStr1, &index);
    assert(testStr1[4] == '-');
    assert(testStr1[5] == '1');
    assert(index == 6);
    
    add_integer_to_string(0, testStr1, &index);
    assert(testStr1[6] == '0');
    assert(index == 7);
    
    add_integer_to_string(-2463, testStr1, &index);
    assert(testStr1[7] == '-');
    assert(testStr1[8] == '2');
    assert(testStr1[9] == '4');
    assert(testStr1[10] == '6');
    assert(testStr1[11] == '3');
    assert(index == 12);

    // assert testing get_power_of_ten()
	assert(get_power_of_ten(-154) == 1); // negative inputs should output 1
	assert(get_power_of_ten(0) == 1);
	assert(get_power_of_ten(1) == 10);
	assert(get_power_of_ten(4) == 10000);
	assert(get_power_of_ten(10) == 10000000000);

    
    // assert testing get_log_ten()
    assert(get_log_ten(10) == 1);
    assert(get_log_ten(-500) == 0); // negative inputs should output 1
    assert(get_log_ten(999) == 2);
    assert(get_log_ten(3) == 0);
    assert(get_log_ten(36875368) == 7);
    assert(get_log_ten(36875368519753) == 13);
    
    
    // assert testing get_value_of_string()
    char testStr[TESTSTRLEN];
    testStr[0] = '0';
    testStr[1] = '\0';
    index = 0;
    assert(get_value_from_string(testStr, &index) == 0);
    assert(index == 1);

    testStr[0] = '7';
    testStr[1] = '0';
    testStr[2] = '0';
    testStr[3] = '\0';
    index = 0;
    assert(get_value_from_string(testStr, &index) == 700);
    assert(index == 3);
    
    testStr[0] = '-';
    testStr[1] = '7';
    testStr[2] = '7';
    testStr[3] = '\0';
    index = 0;
    assert(get_value_from_string(testStr, &index) == -77);
    assert(index == 3);

    testStr[0] = '-';
    testStr[1] = '7';
    testStr[2] = '7';
    testStr[3] = ' ';
    testStr[4] = '3';
    testStr[5] = ')';
    testStr[6] = '\0';
    index = 0;
    assert(get_value_from_string(testStr, &index) == -77);
    assert(index == 3);
    
    // bool should_move_to_cdr(const char* str, int* index)
    // assert testing move_to_CDR() and should_move_to_cdr()
    test_lisp1 = (Lisp*)allocate_space(1, sizeof(Lisp));
    Lisp* current = test_lisp1;
    index = 0;    
    testStr1[0] = '(';
    assert(!should_move_to_CDR(testStr1, &index));
    Lisp* next = move_to_CDR(testStr1, &index, current);
    assert(current->cdr == NULL);
    assert(current == next);
    

    testStr1[1] = '4';
    index = 1;
    current = next;
    assert(!should_move_to_CDR(testStr1, &index));
    next = move_to_CDR(testStr1, &index, current);
    assert(current->cdr == NULL);
    assert(current == next);

    testStr1[2] = ' ';
    index = 2;
    current = next;
    assert(should_move_to_CDR(testStr1, &index));
    next = move_to_CDR(testStr1, &index, current);
    assert(current->cdr != NULL);
    assert(current->cdr == next);
    
    testStr1[3] = '(';
    index = 3;
    current = next;
    assert(should_move_to_CDR(testStr1, &index));
    next = move_to_CDR(testStr1, &index, current);
    assert(current->cdr != NULL);
    assert(current->cdr == next);
    
    testStr1[4] = ' ';
    index = 4;
    current = next;
    assert(!should_move_to_CDR(testStr1, &index));
    next = move_to_CDR(testStr1, &index, current);
    assert(current->cdr == NULL);
    assert(current == next);
    
    testStr1[5] = '7';
    index = 5;
    current = next;
    assert(!should_move_to_CDR(testStr1, &index));
    next = move_to_CDR(testStr1, &index, current);
    assert(current->cdr == NULL);
    assert(current == next);
    
    testStr1[6] = '-';
    index = 6;
    current = next;
    assert(should_move_to_CDR(testStr1, &index));
    next = move_to_CDR(testStr1, &index, current);
    assert(current->cdr != NULL);
    assert(current->cdr == next);
    
    testStr1[7] = ')';
    index = 7;
    current = next;
    assert(should_move_to_CDR(testStr1, &index));
    next = move_to_CDR(testStr1, &index, current);
    assert(current->cdr != NULL);
    assert(current->cdr == next);
      
    lisp_free(test_lisp1);
    
    // assert testing is_invalid()
    const char* testStr10 = "()";
    assert(!is_invalid(testStr10));
    
    const char* testStr11 = "(4 0 2 (2)";
    assert(is_invalid(testStr11));
    
    const char* testStr12 = ")(";
    assert(is_invalid(testStr12));
    
    const char* testStr13 = "(3 (2 3) 2)";
    assert(!is_invalid(testStr13));
}
