#include "../lisp.h"
#include <ctype.h>

#define TESTSTRLEN 1000
#define LISPIMPL "linked"
#define EMPTY 0
#define STREND '\0'
#define ISEVEN(val) ((val & 1) == 0)
#define MAXDIGIT 9

struct lisp {
    struct lisp* car; // points to this element of the list
    struct lisp* cdr; // points to the remainder of the list
    atomtype value;
};

void* allocateSpace(unsigned int num, unsigned int size);
void throwError(const char* errorMessage);
void setStringToEmpty(char* str);
void addNodeToString(const lisp* l, char* str, int* index);
void considerAdjacentNodes(const lisp* l, char* str, int* index);
void addIntegerToString(long integer, char* str, int* index);
long getAbsolute(long integer, char* str, int* index);
long getPowerOfTen(int power);
int getLogTen(long n);
atomtype getValueFromString(const char* str, int* index);
bool isPositive(const char* str, int* index);
void addNewSubLisp(const char* str, int* index, lisp* list);
lisp* moveToCDR(const char* str, int* index, lisp* list);
bool shouldMoveToCDR(const char* str, int* index);
bool isInvalid(const char* str);
void reduceTestCountAtoms(lisp* atom, atomtype* accum);
void reduceTestCountEvenAtoms(lisp* atom, atomtype* accum);
void test(void); 
