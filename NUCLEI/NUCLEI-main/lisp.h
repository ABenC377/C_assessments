#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#define TESTSTRLEN 1000
#define LISPIMPL "linked"
#define EMPTY 0
#define STREND '\0'
#define ISEVEN(val) ((val & 1) == 0)

typedef struct Lisp {
    struct Lisp* car; // points to this element of the list
    struct Lisp* cdr; // points to the remainder of the list
    int value;
} Lisp; 

void* allocate_space(unsigned int num, unsigned int size);
void throw_error(const char* error_message);
void set_string_to_empty(char* str);
void add_node_to_string(const Lisp* l, char* str, int* index);
void consider_adjacent_nodes(const Lisp* l, char* str, int* index);
void add_integer_to_string(long integer, char* str, int* index);
long get_absolute(long integer, char* str, int* index);
long get_power_of_ten(int power);
int get_log_ten(long n);
int get_value_from_string(const char* str, int* index);
bool is_positive(const char* str, int* index);
void add_new_sub_lisp(const char* str, int* index, Lisp* list);
Lisp* move_to_CDR(const char* str, int* index, Lisp* list);
bool should_move_to_CDR(const char* str, int* index);
bool is_invalid(const char* str);
void reduce_test_count_atoms(Lisp* atom, int* accum);
void reduce_test_count_even_atoms(Lisp* atom, int* accum);
void test_lisp(void); 
Lisp* lisp_atom(const int a);
Lisp* lisp_cons(const Lisp* l1, const Lisp* l2);
Lisp* lisp_car(const Lisp* l);
Lisp* lisp_cdr(const Lisp* l);
int lisp_get_val(const Lisp* l);
bool lisp_is_atomic(const Lisp* l);
Lisp* lisp_copy(const Lisp* l);
int lisp_length(const Lisp* l);
void lisp_to_string(const Lisp* l, char* str);
void lisp_free(Lisp* l);
Lisp* lisp_from_string(const char* str);
Lisp* lisp_list(const int n, ...);
void lisp_reduce(void (*func)(Lisp* l, int* n), Lisp* l, int* acc);

