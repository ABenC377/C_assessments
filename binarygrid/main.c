#include <stdbool.h>
#include <stdlib.h>

#define MAX 16
#define ONE '1'
#define ZERO '0'
#define UNK '.'
#define NUMDIREC 6
#define NUM3DIREC 4
#define TOTALS 4

typedef struct {
    int sz;
    char b2d[MAX][MAX];
} board;

typedef enum {up, right, down, left, upDown, rightLeft} direction;
typedef enum {row1s, row0s, col1s, col0s} tots;

bool solvePuzzle(board* brd);
bool canAddInformation(board* brd);
bool canCompleteTile(board* brd, int row, int col);
bool solvePairsOxo(board *brd, int row, int col);
bool isPair(board* brd, int row, int col, direction dir);
void getCoordinates(int row, int col, direction dir, int* row1, int* col1, int* row2, int* col2);
bool isInbounds(board* brd, int row, int col);
bool isValidPlacement(board* brd, int row, int col);
bool failsCounting(board* brd, int row, int count);
int* getTotals(board* brd, int row, int col);
bool formsThree(board* brd, int row, int col);
bool isThree(board* brd, int row, int col, direction dir);
bool solveCounting(board* brd, int row, int col);
bool boardIsComplete(board* brd);


bool solvePuzzle(board* brd) {
    while(canAddInformation(brd)) {
    }
    return boardIsComplete(brd);
}

bool canAddInformation(board* brd) {
    for (int row = 0; row < brd->sz; row++) {
        for (int col = 0; col < brd->sz; col++) {
            if ((brd->b2d[row][col] == UNK) && (canCompleteTile(brd, row, col))) {
                return true;
            }
        }
    }
    return false;
}

bool canCompleteTile(board* brd, int row, int col) {
    return (solvePairsOxo(brd, row, col), solveCounting(brd, row, col));
}

bool solvePairsOxo(board *brd, int row, int col) {
    for (direction dir = up; dir < NUMDIREC; dir++) {
        if (isPair(brd, row, col, dir)) {
            return true;
        }
    }
    return false;
}

bool isPair(board* brd, int row, int col, direction dir) {
    int row1, col1, row2, col2;
    getCoordinates(row, col, dir, &row1, &col1, &row2, &col2);
    if ((!isInbounds(brd, row1, col1)) || (!isInbounds(brd, row2, col2))) {
        return false;
    }
    if (brd->b2d[row1][col1] == brd->b2d[row2][col2]) {
        brd->b2d[row][col] = (brd->b2d[row1][col1] == ONE) ? ZERO : ONE;
        if (isValidPlacement(brd, row, col)) {
            return true;
        } else {
            brd->b2d[row][col] = UNK;
        }
    }
    return false;
}

void getCoordinates(int row, int col, direction dir, int* row1, int* col1, int* row2, int* col2) {
    switch (dir) {
        case up:
            *row1 = row - 1;
            *row2 = row - 2;
            *col1 = *col2 = col;
            break;
        case right:
            *row1 = *row2 = row;
            *col1 = col + 1;
            *col2 = col + 2;
            break;
        case down:
            *row1 = row + 1;
            *row2 = row + 2;
            *col1 = *col2 = col;
            break;
        case left:
            *row1 = *row2 = row;
            *col1 = col - 1;
            *col2 = col - 2;
            break;
        case upDown:
            *row1 = row - 1;
            *row2 = row + 1;
            *col1 = *col2 = col;
            break;
        case rightLeft:
            *row1 = *row2 = row;
            *col1 = col - 1;
            *col2 = col + 1;
            break;
        default:
            break;
    }
}

bool isInbounds(board* brd, int row, int col) {
    return ((row > 0) && (row < brd->sz) && (col > 0) & (col < brd->sz));
}

bool isValidPlacement(board* brd, int row, int col) {
    if ((failsCounting(brd, row, col)) || (formsThree(brd, row, col))) {
        return false;
    } else {
        return true;
    }
}

bool failsCounting(board* brd, int row, int col) {
    int* totals = getTotals(brd, row, col);
    int rowMax = (brd->sz >> 1);
    for (int tot = 0; tot < TOTALS; tot++) {
        if (totals[tot] > rowMax) {
            free(totals);
            return true;
        }
    }
    free(totals);
    return false;
}

int* getTotals(board* brd, int row, int col) {
    int* totals = malloc(sizeof(int) * TOTALS);
    for (int tot = 0; tot < TOTALS; tot++) {
        totals[tot] = 0;
    }
    for (int index = 0; index < brd->sz; index++) {
        if (brd->b2d[row][index] == ONE) {
            totals[row1s]++;
        } else if (brd->b2d[row][index] == ZERO) {
            totals[row0s]++;
        }
        if (brd->b2d[index][col] == ONE) {
            totals[col1s]++;
        } else if (brd->b2d[index][col] == ZERO) {
            totals[col0s]++;
        }
    }
    return totals;
}

bool formsThree(board* brd, int row, int col) {
    for (direction dir = up; dir < NUM3DIREC; dir++) {
        if (isThree(brd, row, col, dir)) {
            return true;
        }
    }
    return false;
}

bool isThree(board* brd, int row, int col, direction dir) {
    int row1, col1, row2, col2;
    getCoordinates(row, col, dir, &row1, &col1, &row2, &col2);
    if ((!isInbounds(brd, row1, col1)) || (!isInbounds(brd, row2, col2))) {
        return false;
    }
    if ((brd->b2d[row1][col1] == brd->b2d[row2][col2]) && (brd->b2d[row][col] == brd->b2d[row2][col2])) {
        return true;
    } else {
        return false;
    }
}

bool solveCounting(board* brd, int row, int col) {
    int* totals = getTotals(brd, row, col);
    int rowMax = (brd->sz >> 1);
    if ((totals[row1s] == rowMax) || (totals[col1s] == rowMax)) {
        brd->b2d[row][col] = ZERO;
    } else if ((totals[row0s] == rowMax) || (totals[col0s] == rowMax)) {
        brd->b2d[row][col] = ONE;
    }
    free(totals);
    if ((brd->b2d[row][col] != UNK) && (isValidPlacement(brd, row, col))) {
        return true;
    } else {
        brd->b2d[row][col] = UNK;
        return false;
    }
}

bool boardIsComplete(board* brd) {
    for (int row = 0; row < brd->sz; row++) {
        for (int col = 0; col < brd->sz; col++) {
            if (brd->b2d[row][col] == UNK) {
                return false;
            }
        }
    }
    return true;
}


