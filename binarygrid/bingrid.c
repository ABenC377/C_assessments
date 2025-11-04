#include "bingrid.h"

#define BOARDSTR (MAX*MAX+1)
#define NUMTOTALS 4

typedef enum {up, right, down, left, upDown, rightLeft} direction;
typedef enum {row1s, row0s, col1s, col0s} tots;
typedef struct {
  board* brd;
  int row;
  int col;
} location;

bool setSize(board* brd, char* str);
bool fillGrid(board* brd, char* str);
bool updateBoard(board* brd);
bool tileCompleted(location* tile);
bool solvePairsOxo(location* tile);
bool isPair(location* tile, direction dir);
char getValue(location* tile);
void getPairCoordinates(direction dir, location* tile1, location* tile2);
bool isOutOfBounds(location* tile);
bool updateTile(location* tile, char newValue);
bool isValidPlacement(location* tile);
bool failsCounting(location* tile);
int* getRowColTotals(location* tile);
bool formsThree(location* tile);
bool isThree(location* tile, direction dir);
bool solveCounting(location* tile);
bool boardIsComplete(board* brd);
//void printBoard(board* brd);


bool str2board(board* brd, char* str) {
  if ((!brd) || (!str)) {
    return false;
  }

  return ((setSize(brd, str)) && (fillGrid(brd, str)));
}


bool setSize(board* brd, char* str) {
  int num_tiles = strlen(str);
  
  // Can't sensibly have a 0x0 board
  if (num_tiles == 0) {
    return false;
  }
  
  // Check that num_tiles is a square number
  int board_size = (int)sqrt((float)num_tiles);
  if (board_size * board_size != num_tiles) {
    return false;
  }

  // Check that the board size is even
  if ((board_size & 1) != 0) {
    return false;
  }

  brd->sz = board_size;
  return true;
}


bool fillGrid(board* brd, char* str) {
  int str_index = 0;
  for (int row = 0; row < brd->sz; row++) {
    for (int col = 0; col < brd->sz; col++) {
      if (str[str_index] == '\0') {
        return false;
      }
      brd->b2d[row][col] = str[str_index];
      str_index++;
    }
  }

  // Check that we have reached the end of the string
  return (str[str_index] == '\0');
}


void board2str(char* str, board* brd) {
  if ((!brd) || (!str)) {
    fprintf(stderr, "Error: board2str() passed a NULL pointer\n");
    exit(EXIT_FAILURE);
  }

  int str_index = 0;
  for (int row = 0; row < brd->sz; row++) {
    for (int col = 0; col < brd->sz; col++) {
      str[str_index] = brd->b2d[row][col];
      str_index++;
    }
  }
  str[str_index] = '\0';
}


bool solve_board(board* brd) {
  if (!brd) {
    return false;
  }

  while (updateBoard(brd)) {
  }
  return boardIsComplete(brd);
}


bool updateBoard(board* brd) {
  for (int row = 0; row < brd->sz; row++) {
    for (int col = 0; col < brd->sz; col++) {
      location tile = {brd, row, col};
      if ((brd->b2d[row][col] == UNK) && (tileCompleted(&tile))) {
        return true;
      }
    }
  }
  return false;
}


bool tileCompleted(location* tile) {
  return ((solvePairsOxo(tile)) || (solveCounting(tile)));
}


bool solvePairsOxo(location* tile) {
  for (direction dir = up; dir <= rightLeft; dir++) { // iterate through directions
    if (isPair(tile, dir)) {
      return true;
    }
  }
  return false;
}


bool isPair(location* tile, direction dir) {
  // Get the tiles that we need to check
  location tile1, tile2;
  tile1 = tile2 = *tile;
  getPairCoordinates(dir, &tile1, &tile2);
  
  char tile1Val = getValue(&tile1);
  char tile2Val = getValue(&tile2);

  // We are not interested in pairs of UNK
  if (tile1Val == UNK) {
    return false;
  }
  
  if (tile1Val == tile2Val) {
    return (tile1Val == ONE) ? updateTile(tile, ZERO) : updateTile(tile, ONE);
  }
  return false;
}


char getValue(location* tile) {
  return (isOutOfBounds(tile)) ? UNK : tile->brd->b2d[tile->row][tile->col];
}


bool isOutOfBounds(location* tile) {
  bool rowOut = ((tile->row < 0) || (tile->row >= tile->brd->sz));
  bool colOut = ((tile->col < 0) || (tile->col >= tile->brd->sz)); 
  return ((rowOut) || (colOut));
}


void getPairCoordinates(direction dir, location* tile1, location* tile2) {
  switch (dir) {
    case up:
      (tile1->row)--;
      (tile2->row) -= 2;
      break;
    case right:
      (tile1->col)++;
      (tile2->col) += 2;
      break;
    case down:
      (tile1->row)++;
      (tile2->row) += 2;
      break;
    case left:
      (tile1->col)--;
      (tile2->col) -= 2;
      break;
    case upDown:
      (tile1->row)--;
      (tile2->row)++;
      break;
    case rightLeft:
      (tile1->col)--;
      (tile2->col)++;
      break;
    default:
      break;
  }
}


bool updateTile(location* tile, char newValue) { 
  tile->brd->b2d[tile->row][tile->col] = newValue;
  if (isValidPlacement(tile)) {
    return true;
  } else {
    tile->brd->b2d[tile->row][tile->col] = UNK;
    return false;
  }
}


bool isValidPlacement(location* tile) {
  return ((!failsCounting(tile)) && (!formsThree(tile)));
}


bool failsCounting(location* tile) {
  int* totals = getRowColTotals(tile);
  int rowMax = (tile->brd->sz >> 1);

  for (int tot = 0; tot < NUMTOTALS; tot++) {
    if (totals[tot] > rowMax) {
      free(totals);
      return true;
    }
  }

  free(totals);
  return false;
}


int* getRowColTotals(location* tile) {
  int* totals = (int*)calloc(NUMTOTALS, sizeof(int));
  
  for (int index = 0; index < tile->brd->sz; index++) {
    char rowVal = tile->brd->b2d[tile->row][index]; 
    char colVal = tile->brd->b2d[index][tile->col]; 

    if (rowVal == ONE) {
      totals[row1s]++;
    } else if (rowVal == ZERO) {
      totals[row0s]++;
    }
    if (colVal == ONE) {
      totals[col1s]++;
    } else if (colVal == ZERO) {
      totals[col0s]++;
    }
  }
  return totals;
}


bool formsThree(location* tile) {
  for (direction dir = up; dir <= rightLeft; dir++) { // Iterate through the directions
    if (isThree(tile, dir)) {
      return true;
    }
  }
  return false;
}


bool isThree(location* tile, direction dir) {
  // Get tiles we are checking
  location tile1, tile2;
  tile1 = tile2 = *tile;
  getPairCoordinates(dir, &tile1, &tile2);
  
  char tileVal = getValue(tile);
  char tile1Val = getValue(&tile1);
  char tile2Val = getValue(&tile2);

  // Not interested in UNK triples
  if (tileVal == UNK) {
    return false;
  }

  bool tileSameAsAdjacent = (tileVal == tile2Val);
  bool adjacentTilesSame = (tile1Val == tile2Val);
  return ((tileSameAsAdjacent) && (adjacentTilesSame));
}


bool solveCounting(location* tile) {
  int* totals = getRowColTotals(tile);
  int rowMax = (tile->brd->sz >> 1); // rowMax is half the row size

  if ((totals[row1s] == rowMax) || (totals[col1s] == rowMax)) {
    free(totals);
    return updateTile(tile, ZERO);
  } else if ((totals[row0s] == rowMax) || (totals[col0s] == rowMax)) {
    free(totals);
    return updateTile(tile, ONE);
  }

  free(totals);
  return false;
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


/*// This function isn't used, but is helpful for debugging.  Therefore, keeping it here in case further development is required.
void printBoard(board* brd) {
  printf("\n");
  for (int row = 0; row < brd->sz; row++) {
    for (int col = 0; col < brd->sz; col++) {
      printf("%c", brd->b2d[row][col]);
    }
    printf("\n");
  }
  printf("\n");
} */


void test(void) {
  board brd;
  char str[BOARDSTR];
  location tile;

  // setSize(board* brd, char* str)
  assert(setSize(&brd, "0110")); // Should work for 2x2
  assert(setSize(&brd, "0110100101101001")); // Should work for 4x4
  assert(setSize(&brd, "0110100101101001011010010110100101101001011010010110100101101001")); // Should work for 8x8
  assert(!setSize(&brd, "")); // Shouldn't make zero-size board
  assert(!setSize(&brd, "011")); // Shouldn't make non-square board
  assert(!setSize(&brd, "0110101101011010110101101")); // Shouldn't make odd board

  // fillGrid(board* brd, char* str)
  assert(setSize(&brd, "0110"));
  assert(fillGrid(&brd, "0110")); // Should work for 2x2 when size set
  assert(setSize(&brd, "0110100101101001")); 
  assert(fillGrid(&brd, "0110100101101001")); // Should work for 4x4 when size set
  assert(!fillGrid(&brd, "0110")); // Shouldn't work - short string 
  assert(!fillGrid(&brd, "011010010110100100000111110000011111")); // Shouldn't work - long string

  // str2board(board* brd, char* str)
  assert(str2board(&brd, "0110"));
  assert(str2board(&brd, "0110100101101001"));
  assert(str2board(&brd, "0110100101101001011010010110100101101001011010010110100101101001"));
  assert(!str2board(&brd, ""));
  assert(!str2board(&brd, "011"));
  assert(!str2board(&brd, "0110101101011010110101101"));


  // solve_board(board* brd) and board2str(&brd, &str)
  str2board(&brd, "011.");
  assert(solve_board(&brd));
  board2str(str, &brd);
  assert(strcmp(str, "0110") == 0);

  str2board(&brd, "011.....0.011001");
  assert(solve_board(&brd));
  board2str(str, &brd);
  assert(strcmp(str, "0110101001011001") == 0);

  str2board(&brd, "1..0....00.1.00..1......00.1...1..00");
  assert(solve_board(&brd));
  board2str(str, &brd);
  assert(strcmp(str, "101010010011100101011010001101110100") == 0);

  str2board(&brd, "0...");
  assert(solve_board(&brd));
  board2str(str, &brd);
  assert(strcmp(str, "0110") == 0);

  str2board(&brd, "................");
  assert(!solve_board(&brd));
  board2str(str, &brd);
  assert(strcmp(str, "................") == 0);

  str2board(&brd, "1..0........0..1");
  assert(!solve_board(&brd));
  board2str(str, &brd);
  assert(strcmp(str, "1..0........0..1") == 0);

  str2board(&brd, "..1...00.1..0..............1......0.");
  assert(!solve_board(&brd));
  board2str(str, &brd);
  assert(strcmp(str, "101...001101010...1........1......0.") == 0);

  // updateBoard(board* brd)
  str2board(&brd, "..1...00.1..0..............1......0.");
  assert(updateBoard(&brd));

  str2board(&brd, "1..0....00.1.00..1......00.1...1..00");
  assert(updateBoard(&brd));

  str2board(&brd, "011.");
  assert(updateBoard(&brd));

  str2board(&brd, "0110101001011001");
  assert(!updateBoard(&brd));

  str2board(&brd, "................");
  assert(!updateBoard(&brd));

  str2board(&brd, "1..0........0..1");
  assert(!updateBoard(&brd)); 

  // tileCompleted(location* tile)
  str2board(&brd, ".11.............");
  tile = (location){.brd = &brd, .row = 0, .col = 0};
  assert(tileCompleted(&tile)); // Should complete by pairs

  str2board(&brd, ".1.1............");
  tile = (location){.brd = &brd, .row = 0, .col = 2};
  assert(tileCompleted(&tile)); // Should complete by oxo

  str2board(&brd, "1..1............");
  tile = (location){.brd = &brd, .row = 0, .col = 1};
  assert(tileCompleted(&tile)); // Should complete by counting

  str2board(&brd, ".10.............");
  tile = (location){.brd = &brd, .row = 0, .col = 0};
  assert(!tileCompleted(&tile));

  str2board(&brd, ".1.0............");
  tile = (location){.brd = &brd, .row = 0, .col = 2};
  assert(!tileCompleted(&tile));

  str2board(&brd, "1..0............");
  tile = (location){.brd = &brd, .row = 0, .col = 1};
  assert(!tileCompleted(&tile));

  // solvePairsOxo(location* tile)
  str2board(&brd, "1..0....00.1.00..1......00.1...1..00");
  tile = (location){.brd = &brd, .row = 1, .col = 4};
  assert(solvePairsOxo(&tile)); // Should solve because pair left

  tile = (location){.brd = &brd, .row = 3, .col = 2};
  assert(solvePairsOxo(&tile)); // Should solve because pair up

  tile = (location){.brd = &brd, .row = 3, .col = 1};
  assert(solvePairsOxo(&tile)); // Should solve because oxo up/down

  tile = (location){.brd = &brd, .row = 3, .col = 0};
  assert(solvePairsOxo(&tile)); // Should solve because pair right

  tile = (location){.brd = &brd, .row = 0, .col = 5};
  assert(solvePairsOxo(&tile)); // Should solve because pair down

  tile = (location){.brd = &brd, .row = 0, .col = 4};
  assert(solvePairsOxo(&tile)); // Should solve because oxo right/left

  tile = (location){.brd = &brd, .row = 5, .col = 2};
  assert(!solvePairsOxo(&tile)); // Not enough info

  brd.b2d[1][0] = ONE;
  tile = (location){.brd = &brd, .row = 1, .col = 1};
  assert(!solvePairsOxo(&tile)); // Would lead to impossible row

  brd.b2d[2][3] = ZERO;
  tile = (location){.brd = &brd, .row = 3, .col = 3};
  assert(!solvePairsOxo(&tile)); // Would lead to three consecutive same values

  // isPair(location* tile, direction dir)
  str2board(&brd, "1..0....00.1.00..1......00.1...1..00");
  tile = (location){.brd = &brd, .row = 2, .col = 3};
  assert(isPair(&tile, up));

  tile = (location){.brd = &brd, .row = 2, .col = 0};
  assert(isPair(&tile, right));

  tile = (location){.brd = &brd, .row = 0, .col = 2};
  assert(isPair(&tile, down));

  tile = (location){.brd = &brd, .row = 4, .col = 2};
  assert(isPair(&tile, left));

  tile = (location){.brd = &brd, .row = 3, .col = 3};
  assert(isPair(&tile, upDown));

  tile = (location){.brd = &brd, .row = 2, .col = 4};
  assert(isPair(&tile, rightLeft));

  tile = (location){.brd = &brd, .row = 1, .col = 1};
  assert(!isPair(&tile, up)); // Pair in this direction is out of bounds

  tile = (location){.brd = &brd, .row = 4, .col = 5};
  assert(!isPair(&tile, left)); // Not enough info

  tile = (location){.brd = &brd, .row = 3, .col = 2};
  assert(!isPair(&tile, down)); // Pair is above, not down

  // getValue(location* tile)
  assert(getValue(&tile) == UNK);

  tile = (location){.brd = &brd, .row = -2, .col = 3};
  assert(getValue(&tile) == UNK); // Out of bounds so should return UNK

  tile = (location){.brd = &brd, .row = 4, .col = 3};
  assert(getValue(&tile) == ONE);

  tile = (location){.brd = &brd, .row = 2, .col = 1};
  assert(getValue(&tile) == ZERO);
 

  // getPairCoordinates(dirction dir, location* tile1, location* tile2)
  tile = (location){.brd = &brd, .row = 3, .col = 2};
  location tile1;
  location tile2;
  tile1 = tile2 = tile;
  getPairCoordinates(up, &tile1, &tile2);
  assert(tile1.row == 2);
  assert(tile1.col == 2);
  assert(tile2.row == 1);
  assert(tile2.col == 2);

  tile1 = tile2 = tile;
  getPairCoordinates(left, &tile1, &tile2);
  assert(tile1.row == 3);
  assert(tile1.col == 1);
  assert(tile2.row == 3);
  assert(tile2.col == 0);

  tile1 = tile2 = tile;
  getPairCoordinates(down, &tile1, &tile2);
  assert(tile1.row == 4);
  assert(tile1.col == 2);
  assert(tile2.row == 5);
  assert(tile2.col == 2);

  tile1 = tile2 = tile;
  getPairCoordinates(right, &tile1, &tile2);
  assert(tile1.row == 3);
  assert(tile1.col == 3);
  assert(tile2.row == 3);
  assert(tile2.col == 4);

  tile1 = tile2 = tile;
  getPairCoordinates(upDown, &tile1, &tile2);
  assert(tile1.row == 2);
  assert(tile1.col == 2);
  assert(tile2.row == 4);
  assert(tile2.col == 2);

  tile1 = tile2 = tile;
  getPairCoordinates(rightLeft, &tile1, &tile2);
  assert(tile1.row == 3);
  assert(tile1.col == 1);
  assert(tile2.row == 3);
  assert(tile2.col == 3);
  
  // isOutOfBounds(location* tile)
  tile = (location){.brd = &brd, .row = 0, .col = 0};
  assert(!isOutOfBounds(&tile));

  tile = (location){.brd = &brd, .row = 5, .col = 5};
  assert(!isOutOfBounds(&tile));

  tile = (location){.brd = &brd, .row = -1, .col = 5};
  assert(isOutOfBounds(&tile)); // row too low

  tile = (location){.brd = &brd, .row = 6, .col = 5};
  assert(isOutOfBounds(&tile)); // row too high

  tile = (location){.brd = &brd, .row = 3, .col = -2};
  assert(isOutOfBounds(&tile)); // col too low

  tile = (location){.brd = &brd, .row = 3, .col = 18};
  assert(isOutOfBounds(&tile)); // col too high

  // updateTile(location* tile, char newValue)
  tile = (location){.brd = &brd, .row = 1, .col = 1};
  assert(!updateTile(&tile, ZERO)); // Should fail because would make three in a row
  assert(brd.b2d[tile.row][tile.col] == UNK); // Should revert to UNK

  assert(updateTile(&tile, ONE));
  assert(brd.b2d[tile.row][tile.col] == ONE);

  brd.b2d[4][4] = ZERO;
  tile = (location){.brd = &brd, .row = 4, .col = 5};
  assert(!updateTile(&tile, ZERO)); // Should fail because would make illegal row
  assert(brd.b2d[tile.row][tile.col] == UNK);

  // isValidPlacement(location* tile)
  tile = (location){.brd = &brd, .row = 2, .col = 2};
  assert(isValidPlacement(&tile));

  tile = (location){.brd = &brd, .row = 3, .col = 3};
  assert(isValidPlacement(&tile));

  tile = (location){.brd = &brd, .row = 5, .col = 4};
  assert(isValidPlacement(&tile));

  brd.b2d[3][2] = ZERO;
  tile = (location){.brd = &brd, .row = 3, .col = 2};
  assert(!isValidPlacement(&tile)); // Three of a kind above

  brd.b2d[5][3] = ZERO;
  tile = (location){.brd = &brd, .row = 5, .col = 3};
  assert(!isValidPlacement(&tile)); // Four 0s in col
  
  // failsCounting(location* tile)
  assert(failsCounting(&tile)); // Four 0s in col

  brd.b2d[3][5] = ONE;
  brd.b2d[0][5] = ONE;
  tile = (location){.brd = &brd, .row = 0, .col = 5};
  assert(failsCounting(&tile)); // Four 1s in col

  brd.b2d[4][4] = ZERO;
  brd.b2d[4][5] = ZERO;
  tile = (location){.brd = &brd, .row = 4, .col = 4};
  assert(failsCounting(&tile)); // Four 0s in row

  brd.b2d[0][1] = ONE;
  tile = (location){.brd = &brd, .row = 0, .col = 2};
  assert(failsCounting(&tile)); // Four 1s in row

  tile = (location){.brd = &brd, .row = 2, .col = 2};
  assert(!failsCounting(&tile));


  tile = (location){.brd = &brd, .row = 5, .col = 4};
  assert(!failsCounting(&tile)); 

  // getTotals(location* tile)
  int* totals = getRowColTotals(&tile);
  assert(totals[row1s] == 1);
  assert(totals[row0s] == 3);
  assert(totals[col1s] == 0);
  assert(totals[col0s] == 3);
  free(totals);

  tile = (location){.brd = &brd, .row = 2, .col = 3};
  totals = getRowColTotals(&tile);
  assert(totals[row1s] == 3);
  assert(totals[row0s] == 3);
  assert(totals[col1s] == 2);
  assert(totals[col0s] == 4);
  free(totals);


  tile = (location){.brd = &brd, .row = 1, .col = 0};
  totals = getRowColTotals(&tile);
  assert(totals[row1s] == 2);
  assert(totals[row0s] == 2);
  assert(totals[col1s] == 2);
  assert(totals[col0s] == 1);
  free(totals);

  // formsThree(location* tile) and isThree(location* tile, direction dir)
  tile = (location){.brd = &brd, .row = 0, .col = 0};
  assert(formsThree(&tile)); // Three right
  assert(isThree(&tile, right));

  tile = (location){.brd = &brd, .row = 0, .col = 1};
  assert(formsThree(&tile)); // Three rightLeft
  assert(isThree(&tile, rightLeft));

  tile = (location){.brd = &brd, .row = 0, .col = 2};
  assert(formsThree(&tile)); // Three left
  assert(isThree(&tile, left));

  tile = (location){.brd = &brd, .row = 1, .col = 5};
  assert(formsThree(&tile)); // Three down
  assert(isThree(&tile, down));

  tile = (location){.brd = &brd, .row = 2, .col = 5};
  assert(formsThree(&tile)); // Three upDown
  assert(isThree(&tile, upDown));

  tile = (location){.brd = &brd, .row = 3, .col = 5};
  assert(formsThree(&tile)); // Three up
  assert(isThree(&tile, up));

  tile = (location){.brd = &brd, .row = 2, .col = 4};
  assert(!formsThree(&tile));
  assert(!isThree(&tile, up));
  assert(!isThree(&tile, right));

  tile = (location){.brd = &brd, .row = 4, .col = 0};
  assert(!formsThree(&tile));
  assert(!isThree(&tile, down));
  assert(!isThree(&tile, left));

  tile = (location){.brd = &brd, .row = 0, .col = 4};
  assert(!formsThree(&tile));
  assert(!isThree(&tile, upDown));
  assert(!isThree(&tile, rightLeft));

  // solveCounting(location* tile);
  str2board(&brd, "1..0..0.00.1.00..1.1....0011.1.1.010");
  tile = (location){.brd = &brd, .row = 3, .col = 3};
  assert(solveCounting(&tile)); // Three 0s in col

  tile = (location){.brd = &brd, .row = 4, .col = 4};
  assert(solveCounting(&tile)); // Three 1s in row

  tile = (location){.brd = &brd, .row = 1, .col = 1};
  assert(solveCounting(&tile)); // Three 0s in row

  tile = (location){.brd = &brd, .row = 0, .col = 1};
  assert(solveCounting(&tile)); // Three 1s in col

  tile = (location){.brd = &brd, .row = 2, .col = 3};
  assert(!solveCounting(&tile)); // Leads to triple

  tile = (location){.brd = &brd, .row = 3, .col = 0};
  assert(!solveCounting(&tile)); // Not enough info

  // boardIsComplete(board* brd);
  assert(!boardIsComplete(&brd));

  str2board(&brd, "1001");
  assert(boardIsComplete(&brd));

  str2board(&brd, "");
  assert(boardIsComplete(&brd)); // A 0x0 board is always complete

  str2board(&brd, "1001011010010110");
  assert(boardIsComplete(&brd));

  str2board(&brd, "1111");
  assert(boardIsComplete(&brd)); // Shouldn't matter that the board is invalid

  str2board(&brd, "10.1");
  assert(!boardIsComplete(&brd));

  str2board(&brd, "................");
  assert(!boardIsComplete(&brd));
}
