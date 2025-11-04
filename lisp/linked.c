#include "specific.h"

lisp* lisp_atom(const atomtype a) {
    lisp* newAtom = (lisp*)allocateSpace(1, sizeof(lisp));
    newAtom->value = a;
    return newAtom;
}


lisp* lisp_cons(const lisp* l1,  const lisp* l2) {
    lisp* newList = (lisp*)allocateSpace(1, sizeof(lisp));
    newList->car = (lisp*)l1;
    newList->cdr = (lisp*)l2; 
    return newList;
}


lisp* lisp_car(const lisp* l) {
    return (l) ? l->car : NULL;
}


lisp* lisp_cdr(const lisp* l) {
    return (l) ? l->cdr : NULL;
}


atomtype lisp_getval(const lisp* l) {
    return ((l) && (lisp_isatomic(l))) ? l->value : EMPTY;
}


bool lisp_isatomic(const lisp* l) {
    return (!(!l || l->car || l->cdr));
}


lisp* lisp_copy(const lisp* l) {
    if (!l) {
        return NULL; 
    }
    
    lisp* newList = (lisp*)allocateSpace(1, sizeof(lisp));
    newList->car = lisp_copy(l->car);
    newList->cdr = lisp_copy(l->cdr);
    newList->value = l->value;
    
    return newList;
}


int lisp_length(const lisp* l) {
    int num = EMPTY;
    if (!l || lisp_isatomic(l)) {
        return num;
    } else {
        num++;
    }
    
    lisp* current = (lisp*)l;
    while (current->cdr) {
        current = current->cdr;
        num++;
    }
    
    return num;
}


void lisp_tostring(const lisp* l, char* str) {
    if (!l) {
        setStringToEmpty(str);
    } else { 
        int index = EMPTY;
        if (lisp_isatomic(l)) {
            addNodeToString(l, str, &index);    
        } else {
            str[index++] = '(';
            addNodeToString(l, str, &index);
            str[index++] = ')'; 
        }
        str[index] = STREND;
    }
}


void lisp_free(lisp** l) {
    if (*l) {
        lisp_free(&((*l)->car)); 
        lisp_free(&((*l)->cdr));
        free(*l);
        *l = NULL;
    }
}

/* ------------- Tougher Ones : Extensions ---------------*/


lisp* lisp_fromstring(const char* str) {
    if ((strcmp(str, "()") == 0) || (isInvalid(str))) { 
        return NULL; 
    }

    lisp* newList = (lisp*)allocateSpace(1, sizeof(lisp));
    int index = 0;

    if (str[index] != '(') {
        newList->value = getValueFromString(str, &index);
    } else {
        index++;
        addNewSubLisp(str, &index, newList);
    }
    
    return newList;
}


lisp* lisp_list(const int n, ...) {
    if (n <= EMPTY) { 
        return NULL; 
    }

    va_list arguments; 
    va_start(arguments, n);
    
    lisp* newList = (lisp*)allocateSpace(1, sizeof(lisp));
    lisp* current = newList;
    current->car = va_arg(arguments, lisp*);
    
    for (int i = 1; i < n; i++) {
        current = current->cdr = (lisp*)allocateSpace(1, sizeof(lisp));
        current->car = va_arg(arguments, lisp*);
    }

    va_end(arguments);
    return newList;
}


void lisp_reduce(void (*func)(lisp* l, atomtype* n), lisp* l, atomtype* acc) {
    if ((l) && (lisp_isatomic(l))) {
        func(l, acc);
    } else if (l) {
        lisp_reduce(func, l->car, acc);
        if (l->cdr) {
            lisp_reduce(func, l->cdr, acc);
        }
    }
}


/* ------------- Helper functions ---------------*/

void* allocateSpace(unsigned int num, unsigned int size) {
    void* pointer = calloc(num, size);
    if (!pointer) {
        throwError("Error: unable to allocate space\n");
    }
    return pointer;
}


void throwError(const char* errorMessage) {
    fputs(errorMessage, stderr);
    exit(EXIT_FAILURE);
}


void setStringToEmpty(char* str) {
    str[0] = '(';
    str[1] = ')';
    str[2] = STREND;
}


void addNodeToString(const lisp* l, char* str, int* index) {
    if ((l->cdr) && !(l->car)) {
        throwError("ERROR: cannot turn invalid lisp into string\n"); 
    } 
    if (lisp_isatomic(l)) {
        addIntegerToString(lisp_getval(l), str, index);
    } else {
        considerAdjacentNodes(l, str, index); 
    }
}


void considerAdjacentNodes(const lisp* l, char* str, int* index) {
    if (lisp_isatomic(l->car)) {
        addNodeToString(l->car, str, index);
    } else {
        str[(*index)++] = '(';
        addNodeToString(l->car, str, index);
        str[(*index)++] = ')';
    }
    
    if (l->cdr) {
        str[(*index)++] = ' ';
        addNodeToString(l->cdr, str, index);
    }
}


void addIntegerToString(long integer, char* str, int* index) {
    long remainder = getAbsolute(integer, str, index);
    
    while (remainder > MAXDIGIT) {
        int power = getLogTen(remainder);
        long divisor = getPowerOfTen(power);
        long toAdd = remainder / divisor;
        str[(*index)++] = ('0' + (char)toAdd);
        remainder = remainder % divisor;
    }
    
    str[(*index)++] = ('0' + (char)remainder);
}


long getAbsolute(long integer, char* str, int* index) {
    if (integer < 0) {
        str[(*index)++] = '-'; 
        return (0 - integer);
    } else {
        return integer;
    }
}


long getPowerOfTen(int power) {
    long output = 1;
    for (int i = 0; i < power; i++) {
        output *= 10;
    }
    return output;
}


int getLogTen(long n) {
    int output = 0;
    while (n >= 10) {
        n /= 10;
        output++;
    }
    return output;
}


atomtype getValueFromString(const char* str, int* index) {
    bool positive = isPositive(str, index);

    int value = 0;
    while (isdigit(str[*index])) {
        value *= 10;
        value += (str[*index] - '0');
        (*index)++;
    }

    return (positive) ? value : (0 - value);
}


bool isPositive(const char* str, int* index) {
    if (str[*index] == '-') {
        (*index)++;
        return false;
    } else {
        return true;
    }
}


void addNewSubLisp(const char* str, int* index, lisp* list) {
    while (str[*index] != ')') {
        if (str[*index] == ' ') {
            (*index)++;
        } else if (str[*index] == '(') {
            list = moveToCDR(str, index, list);
            (*index)++;
            list->car = (lisp*)allocateSpace(1, sizeof(lisp));
            addNewSubLisp(str, index, list->car);
        } else {
            list = moveToCDR(str, index, list);
            atomtype value = getValueFromString(str, index);
            list->car = (lisp*)allocateSpace(1, sizeof(lisp));
            list->car->value = value;
        }
    }
    (*index)++;
}


lisp* moveToCDR(const char* str, int* index, lisp* list) {
    if (shouldMoveToCDR(str, index)) {
        list->cdr = (lisp*)allocateSpace(1, sizeof(lisp));
        list = list->cdr;
    }
    return list;
}


bool shouldMoveToCDR(const char* str, int* index) {
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

bool isInvalid(const char* str) {
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



// This function is purely for testing lisp_reduce()
void reduceTestCountAtoms(lisp* atom, atomtype* accum) {
    (void)atom;
    (*accum)++;
}


// This function is purely for testing lisp_reduce()
void reduceTestCountEvenAtoms(lisp* atom, atomtype* accum) {
    if (ISEVEN(lisp_getval(atom))) {
        (*accum)++;
    }
}


void test(void) {
    // testing lisp_atom() and lisp_getval() and lisp_isatomic()
    lisp* testAtom1 = lisp_atom(13);
    assert(!(testAtom1->car));
    assert(!(testAtom1->cdr));
    assert(lisp_isatomic(testAtom1));
    assert(testAtom1->value == 13);
    assert(lisp_getval(testAtom1) == 13);
    free(testAtom1);
    
    testAtom1 = lisp_atom(-13);
    assert(!(testAtom1->car));
    assert(!(testAtom1->cdr));
    assert(lisp_isatomic(testAtom1));
    assert(testAtom1->value == -13);
    assert(lisp_getval(testAtom1) == -13);
    free(testAtom1);
    
    testAtom1 = lisp_atom(500);
    assert(!(testAtom1->car));
    assert(!(testAtom1->cdr));
    assert(lisp_isatomic(testAtom1));
    assert(testAtom1->value == 500);
    assert(lisp_getval(testAtom1) == 500);
    free(testAtom1);
    
    testAtom1 = lisp_atom(0);
    assert(!(testAtom1->car));
    assert(!(testAtom1->cdr));

    assert(testAtom1->value == 0);
    assert(lisp_getval(testAtom1) == 0);
    free(testAtom1);
    
    
    /*
    The following test lisps include the edge cases that the functions may 
    encounter, and are used to assert test the following functions:
        - lisp_fromstring()
        - lisp_getval()
        - lisp_length()
        - lisp_cons()
        - lisp_tostring()
        - lisp_isatomic()
    */
    
    //------------------------------TEST LISP 1--------------------------------
    
    const char* testConstStr1 = "(3 (2 3) 2)";
    lisp* testLisp1 = lisp_fromstring(testConstStr1);
    /*
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
    assert(testLisp1);
    assert(testLisp1->cdr);
    assert(testLisp1->cdr->car);
    assert(testLisp1->cdr->car->cdr);
    assert(!(testLisp1->cdr->car->cdr->cdr)); // list end
    assert(testLisp1->cdr->cdr);
    assert(!(testLisp1->cdr->cdr->cdr)); // list end
    
    // Assert that the correct nodes are atoms
    assert(lisp_isatomic(testLisp1->car));
    assert(lisp_isatomic(testLisp1->cdr->car->car));
    assert(lisp_isatomic(testLisp1->cdr->car->cdr->car));
    assert(lisp_isatomic(testLisp1->cdr->cdr->car));
    assert(!lisp_isatomic(testLisp1));
    assert(!lisp_isatomic(testLisp1->cdr));
    assert(!lisp_isatomic(testLisp1->cdr->car));
    assert(!lisp_isatomic(testLisp1->cdr->cdr));
    assert(!lisp_isatomic(testLisp1->cdr->car->cdr));
    
    // Assert that the atom nodes have the correct values
    assert(lisp_getval(testLisp1->car) == 3);
    assert(lisp_getval(testLisp1->cdr->car->car) == 2);
    assert(lisp_getval(testLisp1->cdr->car->cdr->car) == 3);
    assert(lisp_getval(testLisp1->cdr->cdr->car) == 2);
    
    // Assert that the lists have the correct length
    assert(lisp_length(testLisp1) == 3);
    assert(lisp_length(testLisp1->cdr->car) == 2);
    
    // Assert that tostring gets back to the input string
    char testString1[TESTSTRLEN] = {'\0'};
    lisp_tostring(testLisp1, testString1);
    assert(strcmp(testString1, testConstStr1) == 0);
    
    
    
    //------------------------------TEST LISP 2--------------------------------
    
    const char* testConstStr2 = "((1 2 (3 4)) 1 (89 -214))";
    lisp* testLisp2 = lisp_fromstring(testConstStr2);
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
    assert(testLisp2);
    assert(testLisp2->car);
    assert(testLisp2->car->cdr);
    assert(testLisp2->car->cdr->cdr);
    assert(!(testLisp2->car->cdr->cdr->cdr)); // list end
    assert(testLisp2->car->cdr->cdr->car);
    assert(testLisp2->car->cdr->cdr->car->cdr);
    assert(!(testLisp2->car->cdr->cdr->car->cdr->cdr)); // list end
    assert(testLisp2->cdr);
    assert(testLisp2->cdr->cdr);
    assert(testLisp2->cdr->cdr->car);
    assert(!(testLisp2->cdr->cdr->cdr)); // list end
    assert(testLisp2->cdr->cdr->car->cdr);
    assert(!(testLisp2->cdr->cdr->car->cdr->cdr)); // list end
    
    // Assert that the correct nodes are atoms
    assert(lisp_isatomic(testLisp2->car->car));
    assert(lisp_isatomic(testLisp2->car->cdr->car));
    assert(lisp_isatomic(testLisp2->car->cdr->cdr->car->car));
    assert(lisp_isatomic(testLisp2->car->cdr->cdr->car->cdr->car));
    assert(lisp_isatomic(testLisp2->cdr->car));
    assert(lisp_isatomic(testLisp2->cdr->cdr->car->car));
    assert(lisp_isatomic(testLisp2->cdr->cdr->car->cdr->car));
    assert(!lisp_isatomic(testLisp2));
    assert(!lisp_isatomic(testLisp2->car));
    assert(!lisp_isatomic(testLisp2->car->cdr));
    assert(!lisp_isatomic(testLisp2->car->cdr->cdr));
    assert(!lisp_isatomic(testLisp2->car->cdr->cdr->car));
    assert(!lisp_isatomic(testLisp2->car->cdr->cdr->car->cdr));
    assert(!lisp_isatomic(testLisp2->cdr));
    assert(!lisp_isatomic(testLisp2->cdr->cdr));
    assert(!lisp_isatomic(testLisp2->cdr->cdr->car));
    assert(!lisp_isatomic(testLisp2->cdr->cdr->car->cdr));
    
    // Assert that the atom nodes have the correct values
    assert(lisp_getval(testLisp2->car->car) == 1);
    assert(lisp_getval(testLisp2->car->cdr->car) == 2);
    assert(lisp_getval(testLisp2->car->cdr->cdr->car->car) == 3);
    assert(lisp_getval(testLisp2->car->cdr->cdr->car->cdr->car) == 4);
    assert(lisp_getval(testLisp2->cdr->car) == 1);
    assert(lisp_getval(testLisp2->cdr->cdr->car->car) == 89);
    assert(lisp_getval(testLisp2->cdr->cdr->car->cdr->car) == -214);
    
    // Assert that the lists have the correct length
    assert(lisp_length(testLisp2) == 3);
    assert(lisp_length(testLisp2->car) == 3);
    assert(lisp_length(testLisp2->car->cdr->cdr->car) == 2);
    assert(lisp_length(testLisp2->cdr->cdr->car) == 2);
    
    // Assert that tostring gets back to the input string
    lisp_tostring(testLisp2, testString1);
    assert(strcmp(testString1, testConstStr2) == 0);
    
    
    //------------------------------TEST LISP 3--------------------------------
    
    lisp* testLisp3 = lisp_cons(testLisp1, testLisp2);
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
    assert(testLisp3);
    assert(testLisp3->car);
    assert(testLisp3->car->cdr);
    assert(testLisp3->car->cdr->car);
    assert(testLisp3->car->cdr->car->cdr);
    assert(!(testLisp3->car->cdr->car->cdr->cdr)); // list end
    assert(testLisp3->car->cdr->cdr);
    assert(!(testLisp3->car->cdr->cdr->cdr)); // list end
    assert(testLisp3->cdr);
    assert(testLisp3->cdr->car);
    assert(testLisp3->cdr->car->cdr);
    assert(testLisp3->cdr->car->cdr->cdr);
    assert(!(testLisp3->cdr->car->cdr->cdr->cdr)); // list end
    assert(testLisp3->cdr->car->cdr->cdr->car);
    assert(lisp_isatomic(testLisp3->cdr->car->cdr->cdr->car->car));
    assert(testLisp3->cdr->car->cdr->cdr->car->cdr);
    assert(!(testLisp3->cdr->car->cdr->cdr->car->cdr->cdr)); // list end
    assert(testLisp3->cdr->cdr);
    assert(testLisp3->cdr->cdr->cdr);
    assert(testLisp3->cdr->cdr->cdr->car);
    assert(!(testLisp3->cdr->cdr->cdr->cdr)); // list end
    assert(testLisp3->cdr->cdr->cdr->car->cdr);
    assert(!(testLisp3->cdr->cdr->cdr->car->cdr->cdr)); // list end
    
    // Assert that the correct nodes are atoms
    assert(lisp_isatomic(testLisp3->car->car));
    assert(lisp_isatomic(testLisp3->car->cdr->car->car));
    assert(lisp_isatomic(testLisp3->car->cdr->car->cdr->car));
    assert(lisp_isatomic(testLisp3->car->cdr->cdr->car));
    assert(lisp_isatomic(testLisp3->cdr->car->car));
    assert(lisp_isatomic(testLisp3->cdr->car->cdr->car));
    assert(lisp_isatomic(testLisp3->cdr->car->cdr->cdr->car->cdr->car));
    assert(lisp_isatomic(testLisp3->cdr->cdr->car));
    assert(lisp_isatomic(testLisp3->cdr->cdr->cdr->car->car));
    assert(lisp_isatomic(testLisp3->cdr->cdr->cdr->car->cdr->car));
    
    // Assert that the atom nodes have the correct values
    assert(lisp_getval(testLisp3->car->car) == 3);
    assert(lisp_getval(testLisp3->car->cdr->car->car) == 2);
    assert(lisp_getval(testLisp3->car->cdr->car->cdr->car) == 3);
    assert(lisp_getval(testLisp3->car->cdr->cdr->car) == 2);
    assert(lisp_getval(testLisp3->cdr->car->car) == 1);
    assert(lisp_getval(testLisp3->cdr->car->cdr->car) == 2);
    assert(lisp_getval(testLisp3->cdr->car->cdr->cdr->car->car) == 3);
    assert(lisp_getval(testLisp3->cdr->car->cdr->cdr->car->cdr->car) == 4);
    assert(lisp_getval(testLisp3->cdr->cdr->car) == 1);
    assert(lisp_getval(testLisp3->cdr->cdr->cdr->car->car) == 89);
    assert(lisp_getval(testLisp3->cdr->cdr->cdr->car->cdr->car) == -214);
    
    // Assert that the lists have the correct length
    assert(lisp_length(testLisp3) == 4);
    assert(lisp_length(testLisp3->car) == 3);
    assert(lisp_length(testLisp3->car->cdr->car) == 2);
    assert(lisp_length(testLisp3->cdr->car) == 3);
    assert(lisp_length(testLisp3->cdr->car->cdr->cdr->car) == 2);
    assert(lisp_length(testLisp3->cdr->cdr->cdr->car) == 2);
    
    // Assert that tostring gets the desired output
    const char* testConstStr3 = "((3 (2 3) 2) (1 2 (3 4)) 1 (89 -214))";
    lisp_tostring(testLisp3, testString1);
    assert(strcmp(testString1, testConstStr3) == 0);


    //------------------------------TEST LISP 4--------------------------------
    
    
    lisp* testLisp4 = lisp_cons(testLisp1, NULL);
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
    assert(testLisp4);
    assert(testLisp4->car);
    assert(testLisp4->car->cdr);
    assert(testLisp4->car->cdr->car);
    assert(testLisp4->car->cdr->car->cdr);
    assert(!(testLisp4->car->cdr->car->cdr->cdr)); // list end
    assert(testLisp4->car->cdr->cdr);
    assert(!(testLisp4->car->cdr->cdr->cdr)); // list end
    assert(!(testLisp4->cdr));
    
    // Assert that the correct nodes are atoms
    assert(lisp_isatomic(testLisp4->car->car));
    assert(lisp_isatomic(testLisp4->car->cdr->car->car));
    assert(lisp_isatomic(testLisp4->car->cdr->car->cdr->car));
    assert(lisp_isatomic(testLisp4->car->cdr->cdr->car));
    
    // Assert that the atom nodes have the correct values
    assert(lisp_getval(testLisp4->car->car) == 3);
    assert(lisp_getval(testLisp4->car->cdr->car->car) == 2);
    assert(lisp_getval(testLisp4->car->cdr->car->cdr->car) == 3);
    assert(lisp_getval(testLisp4->car->cdr->cdr->car) == 2);
    
    // Assert that the lists have the correct length
    assert(lisp_length(testLisp4) == 1);
    assert(lisp_length(testLisp4->car) == 3);
    assert(lisp_length(testLisp4->car->cdr->car) == 2);
    
    // Assert that tostring gets the desired output
    const char* testConstStr4 = "((3 (2 3) 2))";
    lisp_tostring(testLisp4, testString1);
    assert(strcmp(testString1, testConstStr4) == 0);
    
    
    //------------------------------TEST LISP 5--------------------------------
    
    lisp* testLisp5 = lisp_cons(NULL, testLisp1);
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
    assert(testLisp5);
    assert(!(testLisp5->car));
    assert(testLisp5->cdr);
    assert(testLisp5->cdr->cdr);
    assert(testLisp5->cdr->cdr->car);
    assert(testLisp5->cdr->cdr->car->cdr);
    assert(!(testLisp5->cdr->cdr->car->cdr->cdr)); // list end
    assert(testLisp5->cdr->cdr->cdr);
    assert(!(testLisp5->cdr->cdr->cdr->cdr)); // list end
    
    // Assert that the correct nodes are atoms
    assert(lisp_isatomic(testLisp5->cdr->car));
    assert(lisp_isatomic(testLisp5->cdr->cdr->car->car));
    assert(lisp_isatomic(testLisp5->cdr->cdr->car->cdr->car));
    assert(lisp_isatomic(testLisp5->cdr->cdr->cdr->car));
    
    // Assert that the atom nodes have the correct values
    assert(lisp_getval(testLisp5->cdr->car) == 3);
    assert(lisp_getval(testLisp5->cdr->cdr->car->car) == 2);
    assert(lisp_getval(testLisp5->cdr->cdr->car->cdr->car) == 3);
    assert(lisp_getval(testLisp5->cdr->cdr->cdr->car) == 2);
    
    // Assert that the lists have the correct length
    assert(lisp_length(testLisp5) == 4);
    assert(lisp_length(testLisp5->cdr->cdr->car) == 2);
    
    // This is an invalid lisp, and so tostring() will exit if run with it
    // therefore, no assert testing of tostring() on this test lisp
    
    //------------------------------TEST LISP 6--------------------------------
    
    lisp* testLisp6 = lisp_cons(NULL, NULL);
    /*
    CONS
    */
    // assert that there are nodes where there should be, 
    // and not where they shouldn't
    assert(testLisp6);
    assert(!(testLisp6->car));
    assert(!(testLisp6->cdr));
    
    // Assert that the lists have the correct length
    assert(lisp_length(testLisp6) == 0);
    assert(lisp_length(testLisp6->cdr) == 0);
    
    free(testLisp4);
    free(testLisp5);
    
    // Assert that tostring gets the desired output
    // this is not a sensible lisp, but the atom should have a value of 0
    const char* testConstStr6 = "0"; 
    lisp_tostring(testLisp6, testString1);
    assert(strcmp(testString1, testConstStr6) == 0);
    
    //----------------------DISCRETE ASSERT TESTING----------------------------
    
    // assert testing lisp_free()
    lisp_free(&testLisp3);
    assert(!testLisp3);
    
    lisp_free(&testLisp6);
    assert(!testLisp6);
    
    // assert testing lisp_car() and lisp_cdr()
    testAtom1 = lisp_atom(0);
    lisp* testAtom2 = lisp_atom(-234);
    testLisp1 = lisp_cons(testAtom1, testAtom2);
    assert(lisp_getval(testLisp1->car) == 0);
    assert(lisp_getval(testLisp1->cdr) == -234);
    
    testLisp2 = lisp_car(testLisp1);
    assert(lisp_getval(testLisp2) == 0);
    assert(!(testLisp2->car));
    assert(!(testLisp2->cdr));
    
    testLisp3 = lisp_cdr(testLisp1);
    assert(lisp_getval(testLisp3) == -234);
    assert(!(testLisp3->car));
    assert(!(testLisp3->cdr));
    
    
    // assert testing lisp_isatomic()
    assert(lisp_isatomic(testAtom1)); // Is atomic
    assert(lisp_isatomic(testAtom2)); // Is atomic
    assert(!lisp_isatomic(testLisp1)); // Should fail - has CAR and CDR
    testLisp1->cdr = NULL;
    assert(!lisp_isatomic(testLisp1)); // Should fail - has CAR but not CDR
    
    
    // assert testing lisp_copy();
    testLisp1->cdr = testAtom2;
    testLisp2 = lisp_copy(testLisp1);
    /*
    CONS --> -234
     |
     V
     0
    */
    assert(lisp_getval(testLisp1->car) == lisp_getval(testLisp2->car));
    assert(lisp_getval(testLisp1->cdr) == lisp_getval(testLisp2->cdr));
    
    lisp_free(&testLisp1);
    lisp_free(&testLisp2);

    // assert testing lisp_list()
    testLisp1 = lisp_atom(1);
    testLisp2 = lisp_atom(2);
    testLisp3 = lisp_atom(3);
    testLisp4 = lisp_atom(4);
    
    testLisp5 = lisp_list(4, testLisp1, testLisp2, testLisp3, testLisp4);
    assert(lisp_length(testLisp5) == 4);
    assert(lisp_getval(lisp_car(testLisp5)) == 1);
    assert(lisp_getval(lisp_car(testLisp5->cdr)) == 2);
    assert(lisp_getval(lisp_car(testLisp5->cdr->cdr)) == 3);
    assert(lisp_getval(lisp_car(testLisp5->cdr->cdr->cdr)) == 4);
    lisp_free(&testLisp5);
    
    testLisp1 = lisp_atom(1);
    testLisp2 = lisp_atom(2);
    testLisp3 = lisp_atom(3);
    testLisp4 = lisp_atom(4);
    testLisp6 = lisp_list(2, testLisp1, testLisp2, testLisp3, testLisp4);
    assert(lisp_length(testLisp6) == 2);
    assert(lisp_getval(lisp_car(testLisp6)) == 1);
    assert(lisp_getval(lisp_car(testLisp6->cdr)) == 2);
    lisp_free(&testLisp6);
    free(testLisp3);
    free(testLisp4);
    
    testLisp1 = lisp_atom(1);
    testLisp2 = lisp_atom(2);
    testLisp3 = lisp_atom(3);
    testLisp4 = lisp_atom(4);
    lisp* testLisp7 = lisp_list(2, testLisp3, testLisp4, testLisp1, testLisp2);
    assert(lisp_length(testLisp7) == 2);
    assert(lisp_getval(lisp_car(testLisp7)) == 3);
    assert(lisp_getval(lisp_car(testLisp7->cdr)) == 4);
    lisp_free(&testLisp7);
    free(testLisp1);
    free(testLisp2);

    // assert testing lisp_reduce()
    testLisp1 = lisp_fromstring("(1 ((2 3) 45) 12 (12 (44 (35 43) 45) (13)))");
    atomtype total = EMPTY;
    lisp_reduce(reduceTestCountAtoms, testLisp1, &total);
    assert(total == 11);
    
    total = EMPTY;
    lisp_reduce(reduceTestCountEvenAtoms, testLisp1, &total);
    assert(total == 4);
    
    lisp_free(&testLisp1);
    
    testLisp1 = lisp_fromstring("()");
    total = EMPTY;
    lisp_reduce(reduceTestCountAtoms, testLisp1, &total);
    assert(total == 0);
    
    total = EMPTY;
    lisp_reduce(reduceTestCountEvenAtoms, testLisp1, &total);
    assert(total == 0);
    
    lisp_free(&testLisp1);
    
    testLisp1 = lisp_fromstring("1");
    total = EMPTY;
    lisp_reduce(reduceTestCountAtoms, testLisp1, &total);
    assert(total == 1);
    
    total = EMPTY;
    lisp_reduce(reduceTestCountEvenAtoms, testLisp1, &total);
    assert(total == 0);
    
    lisp_free(&testLisp1);
    
    // assert testing setStringToEmpty() - 
    setStringToEmpty(testString1);
    assert(strcmp(testString1, "()") == 0);
    assert(testString1[0] == '(');
    assert(testString1[1] == ')');
    assert(testString1[2] == '\0');
    
    // assert testing addNodeToString()
    testLisp3 = (lisp*)allocateSpace(1, sizeof(lisp));
    testLisp3->cdr = (lisp*)allocateSpace(1, sizeof(lisp));
    testLisp3->car = (lisp*)allocateSpace(1, sizeof(lisp));
    testLisp3->cdr->car = (lisp*)allocateSpace(1, sizeof(lisp));
    testLisp3->car->value = -20;
    testLisp3->cdr->car->value = 50;
    char testStr1[TESTSTRLEN] = {'\0'};
    int index = 0;
    addNodeToString(testLisp3->cdr->car, testStr1, &index);
    assert(index == 2);
    testStr1[index] = '\0';
    assert(strcmp(testStr1, "50") == 0);
    
    addNodeToString(testLisp3->car, testStr1, &index);
    assert(index == 5);
    testStr1[index] = '\0';
    assert(strcmp(testStr1, "50-20") == 0);
    
    index = 0;
    addNodeToString(testLisp3, testStr1, &index);
    assert(index == 6);
    testStr1[index] = '\0';
    assert(strcmp(testStr1, "-20 50") == 0);    
    
    // assert testing considerAdjacentNodes()
    index = 0;
    considerAdjacentNodes(testLisp3, testStr1, &index);
    assert(index == 6);
    testStr1[index] = '\0';
    assert(strcmp(testStr1, "-20 50") == 0);
    
    index = 0;
    considerAdjacentNodes(testLisp3->cdr, testStr1, &index);
    assert(index == 2);
    testStr1[index] = '\0';
    assert(strcmp(testStr1, "50") == 0);
    
    lisp_free(&testLisp3);
    
    // assert testing getAbsolute()
    index = 0;
    long testAbs = getAbsolute(0, testStr1, &index);
    assert(testAbs == 0);
    assert(index == 0);
    
    testAbs = getAbsolute(500, testStr1, &index);
    assert(testAbs == 500);
    assert(index == 0);
    
    testAbs = getAbsolute(-500, testStr1, &index);
    assert(testAbs == 500);
    assert(index == 1);
    assert(testStr1[0] == '-');
    
    testAbs = getAbsolute(-1, testStr1, &index);
    assert(testAbs == 1);
    assert(index == 2);
    assert(testStr1[1] == '-');

    // assert testing isPositive()
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
    assert(!isPositive(testStr1, &index));
    assert(index == 1);
    assert(isPositive(testStr1, &index));
    assert(index == 1);
    index = 2;
    assert(isPositive(testStr1, &index));
    assert(index == 2);
    index = 3;
    assert(isPositive(testStr1, &index));
    assert(index == 3);
    index = 4;
    assert(!isPositive(testStr1, &index));
    assert(index == 5);
    assert(isPositive(testStr1, &index));
    assert(index == 5);
    index = 6;
    assert(isPositive(testStr1, &index));
    assert(index == 6);
    index = 7;
    assert(isPositive(testStr1, &index));
    assert(index == 7);
    index = 8;
    assert(!isPositive(testStr1, &index));
    assert(index == 9);
    assert(isPositive(testStr1, &index));
    assert(index == 9);
    index = 10;
    assert(isPositive(testStr1, &index));
    assert(index == 10);
    index = 11;
   
    // assert testing addIntegerToString()
    index = 0;
    addIntegerToString(5234, testStr1, &index);
    assert(testStr1[0] == '5');
    assert(testStr1[1] == '2');
    assert(testStr1[2] == '3');
    assert(testStr1[3] == '4');
    assert(index == 4);
    
    addIntegerToString(-1, testStr1, &index);
    assert(testStr1[4] == '-');
    assert(testStr1[5] == '1');
    assert(index == 6);
    
    addIntegerToString(0, testStr1, &index);
    assert(testStr1[6] == '0');
    assert(index == 7);
    
    addIntegerToString(-2463, testStr1, &index);
    assert(testStr1[7] == '-');
    assert(testStr1[8] == '2');
    assert(testStr1[9] == '4');
    assert(testStr1[10] == '6');
    assert(testStr1[11] == '3');
    assert(index == 12);

    // assert testing getPowerOfTen()
	assert(getPowerOfTen(-154) == 1); // negative inputs should output 1
	assert(getPowerOfTen(0) == 1);
	assert(getPowerOfTen(1) == 10);
	assert(getPowerOfTen(4) == 10000);
	assert(getPowerOfTen(10) == 10000000000);

    
    // assert testing getLogTen()
    assert(getLogTen(10) == 1);
    assert(getLogTen(-500) == 0); // negative inputs should output 1
    assert(getLogTen(999) == 2);
    assert(getLogTen(3) == 0);
    assert(getLogTen(36875368) == 7);
    assert(getLogTen(36875368519753) == 13);
    
    
    // assert testing getValueFromString()
    char testStr[TESTSTRLEN];
    testStr[0] = '0';
    testStr[1] = '\0';
    index = 0;
    assert(getValueFromString(testStr, &index) == 0);
    assert(index == 1);

    testStr[0] = '7';
    testStr[1] = '7';
    testStr[2] = '7';
    testStr[3] = '\0';
    index = 0;
    assert(getValueFromString(testStr, &index) == 777);
    assert(index == 3);
    
    testStr[0] = '-';
    testStr[1] = '7';
    testStr[2] = '7';
    testStr[3] = '\0';
    index = 0;
    assert(getValueFromString(testStr, &index) == -77);
    assert(index == 3);

    testStr[0] = '-';
    testStr[1] = '7';
    testStr[2] = '7';
    testStr[3] = ' ';
    testStr[4] = '3';
    testStr[5] = ')';
    testStr[6] = '\0';
    index = 0;
    assert(getValueFromString(testStr, &index) == -77);
    assert(index == 3);
    
    // bool shouldMoveToCDR(const char* str, int* index)
    // assert testing moveToCDR() and shouldMoveToCDR()
    testLisp1 = (lisp*)allocateSpace(1, sizeof(lisp));
    lisp* current = testLisp1;
    index = 0;    
    testStr1[0] = '(';
    assert(!shouldMoveToCDR(testStr1, &index));
    lisp* next = moveToCDR(testStr1, &index, current);
    assert(current->cdr == NULL);
    assert(current == next);
    

    testStr1[1] = '4';
    index = 1;
    current = next;
    assert(!shouldMoveToCDR(testStr1, &index));
    next = moveToCDR(testStr1, &index, current);
    assert(current->cdr == NULL);
    assert(current == next);

    testStr1[2] = ' ';
    index = 2;
    current = next;
    assert(shouldMoveToCDR(testStr1, &index));
    next = moveToCDR(testStr1, &index, current);
    assert(current->cdr != NULL);
    assert(current->cdr == next);
    
    testStr1[3] = '(';
    index = 3;
    current = next;
    assert(shouldMoveToCDR(testStr1, &index));
    next = moveToCDR(testStr1, &index, current);
    assert(current->cdr != NULL);
    assert(current->cdr == next);
    
    testStr1[4] = ' ';
    index = 4;
    current = next;
    assert(!shouldMoveToCDR(testStr1, &index));
    next = moveToCDR(testStr1, &index, current);
    assert(current->cdr == NULL);
    assert(current == next);
    
    testStr1[5] = '7';
    index = 5;
    current = next;
    assert(!shouldMoveToCDR(testStr1, &index));
    next = moveToCDR(testStr1, &index, current);
    assert(current->cdr == NULL);
    assert(current == next);
    
    testStr1[6] = '-';
    index = 6;
    current = next;
    assert(shouldMoveToCDR(testStr1, &index));
    next = moveToCDR(testStr1, &index, current);
    assert(current->cdr != NULL);
    assert(current->cdr == next);
    
    testStr1[7] = ')';
    index = 7;
    current = next;
    assert(shouldMoveToCDR(testStr1, &index));
    next = moveToCDR(testStr1, &index, current);
    assert(current->cdr != NULL);
    assert(current->cdr == next);
      
    lisp_free(&testLisp1);
    
    // assert testing isInvalid()
    const char* testStr10 = "()";
    assert(!isInvalid(testStr10));
    
    const char* testStr11 = "(4 0 2 (2)";
    assert(isInvalid(testStr11));
    
    const char* testStr12 = ")(";
    assert(isInvalid(testStr12));
    
    const char* testStr13 = "(3 (2 3) 2)";
    assert(!isInvalid(testStr13));
}
