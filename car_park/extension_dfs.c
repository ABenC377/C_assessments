#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#define GAP '.'
#define BOLLARD '#'
#define FAILS -1
#define MAXROW 20
#define MAXSTR 21
#define MAXCARS 26
#define SMALLARRAY 100
#define LARGEARRAY 100000
#define LARGENUM 40000000

typedef enum {INVALID, NORMAL, SHOW, ANIMATE, Dfs} Flags;

typedef struct {
  int width;
  int height;
  int moves;
  char board[MAXSTR][MAXSTR];
  int parents[SMALLARRAY];
} Cp;

typedef struct {
  int width;
  int height;
  int moves;
  char** board;
} DFScp;

typedef struct {
  int nextCp;
  int endArray;
  int endTried;
} State;

typedef struct {
  DFScp** tried;
  int endTried;
} DFSstate;

typedef struct {
  char name;
  int startRow;
  int startCol;
  int size;
  bool vertical;
} Car;

typedef struct {
  int row;
  int col;
} Location;


typedef struct DFS {
  DFScp* carpark;
  struct DFS* next;
} DFS;

typedef struct {
  DFS* start;
} Stack;

Flags checkInputs(int argc, char* argv[]);
Flags findFlags(int argc, char* argv[]);
char* getFilename(int argc, char* argv[]);
void solveCarpark(char* fileName, Flags flag);
Cp populateCarpark(FILE* fp); 
bool isValidTile(char tile);
bool isValidCp(Cp start);
int findSolutionBFS(Cp cpArray[], Cp tried[], State* state, Flags flag);
bool isComplete(Cp cp);
void makeNextStates(Cp* current, Cp cpArray[], Cp tried[], State* state);
int getCars(Cp* current, Car cars[]);
int updateCars(int row, int col, char tile, Car cars[], int numCars);
void updateCar(Car* car, int row, int col);
void addCar(int row, int col, char tile, Car cars[], int numCars);
bool carsMisnamed(Car cars[]);
void moveCar(Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state);
void getMoves(Car car, Location* moveBack, Location* moveForwards);
void tryMove(Location move, Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state);
bool movePossible(Location move, Cp* carpark);
bool atEdge(Location move, Cp* carpark);
void removeCar(Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state);
void makeMove(Location move, Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state);
Location getNewGap(Location move, Car car);
void addCarpark(Cp carpark, Cp cpArray[], Cp tried[], State* state);
bool carparksAreSame(Cp carpark1, Cp carpark2);
void handleResult(int moves);
void throwError(char* errorMessage);
void printCarpark(Cp carpark);
void printPath(Cp carpark, Cp cpArray[]);
void copyCarpark(Cp* copy, Cp* original);
void strToCp(Cp* carpark, char* string);
void animateState(Cp* carpark, bool first);
void printTile(char value);
void inefficientPause(void);
int findSolutionDFS(Cp start, DFSstate* dfsState, Flags flag);
void animateStateDFS(DFScp* carpark, bool first);
void makeNextStatesDFS(DFScp* carpark, Stack* stack, DFSstate* dfsState);
void moveCarDFS(Car car, DFScp* carpark, Stack* stack, DFSstate* dfsState);
void tryMoveDFS(Location move, Car car, DFScp* carpark, Stack* stack, DFSstate* dfsState);
void removeCarDFS(Car car, DFScp* carpark, Stack* stack, DFSstate* dfsState);
void makeMoveDFS(Location move, Car car, DFScp* carpark, Stack* stack, DFSstate* dfsState);
void addCarparkDFS(DFScp* carpark, Stack* stack, DFSstate* dfsState);
void addStack(Stack* stack, DFScp* carpark);
DFS* removeStack(Stack* stack);
void* allocateSpace(int num, int size);
void freeDFS(DFS* node);
void freeDFSRecursive(DFS* node);
void freeStack(Stack* stack);
bool isCompleteDFS(DFScp* cp);
int getCarsDFS(DFScp* current, Car cars[]);
bool movePossibleDFS(Location move, DFScp* carpark);
bool atEdgeDFS(Location move, DFScp* carpark);
void copyCarparkDFS(DFScp* copy, DFScp* original);
bool carparksAreSameDFS(DFScp* carpark1, DFScp* carpark2);
void test(void);


int main(int argc, char* argv[]) {
  test();
  Flags flag = checkInputs(argc, argv);
  char* fileName = getFilename(argc, argv);
  
  solveCarpark(fileName, flag);

  return EXIT_SUCCESS;
}


Flags checkInputs(int argc, char* argv[]) {
  if (argc < 2) {
    throwError("ERROR: correct usage = './carpark <carpark file> <flags (optional)>'\n");
  }

  Flags flag = findFlags(argc, argv);
  if (flag == INVALID) {
    throwError("ERROR: correct usage = './carpark <carpark file> <flags (optional)>'\n");
  }
  return flag;
}


Flags findFlags(int argc, char* argv[]) {
  int numNonFlags = 0;
  bool show = false;
  bool animate = false;
  bool dfs = false;
  for (int i = 1; i < argc; i++) {
    char* arg = argv[i];
    if (arg[0] != '-') {
      numNonFlags++;
    } else if (strcmp(arg, "-animate") == 0) {
      animate = true;
    } else if (strcmp(arg, "-show") == 0) {
      show = true;
    } else if (strcmp(arg, "-DFS") == 0) {
      dfs = true;
    }
  }
  if (numNonFlags != 1) {
    return INVALID;
  } else if (dfs) {
    return Dfs;
  } else if (animate) {
    return ANIMATE;
  } else if (show) {
    return SHOW;
  } else {
    return NORMAL;
  } 
}


char* getFilename(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    char* arg = argv[i];
    if (arg[0] != '-') {
      return arg;
    }
  }
  throwError("ERROR: correct usage = './carpark <carpark file> <flags (optional)>'\n");
  return "";
}


void solveCarpark(char* fileName, Flags flag) {
  FILE* fp = fopen(fileName, "r");
  if (!fp) {
    throwError("ERROR: unable to open file\n");
  }
  static Cp cpArray[LARGEARRAY];
  static Cp tried[LARGEARRAY];
  static State state = {0, 1, 1};
  cpArray[0] = tried[0] = populateCarpark(fp);
  fclose(fp);
  
  int moves;
  if (flag == Dfs) {
    DFSstate* dfsState = (DFSstate*)allocateSpace(1, sizeof(DFSstate));
    dfsState->tried = (DFScp**)allocateSpace(LARGEARRAY, sizeof(DFScp*));
    moves = findSolutionDFS(cpArray[0], dfsState, flag);
  } else {
    moves = findSolutionBFS(cpArray, tried, &state, flag);
  }

  handleResult(moves);
}


Cp populateCarpark(FILE* fp) {
  Cp start;
  start.moves = 0;
  char line[MAXSTR];
  fgets(line, MAXSTR, fp);
  sscanf(line, "%ix%i", &(start.height), &(start.width));

  for (int row = 0; row < start.height; row++) {
    fgets(line, MAXSTR, fp);
    for (int col = 0; col < start.width; col++) {
      char tile = line[col];
      if (isValidTile(tile)) {
        start.board[row][col] = tile;
      } else {
        throwError("ERROR: invalid starting carpark\n");
      }
    }
  }
  
  if (!isValidCp(start)) {
    throwError("ERROR: invalid starting carpark\n");
  }
  
  return start;
}


bool isValidTile(char tile) {
  bool isGap = (tile == GAP);
  bool isBol = (tile == BOLLARD);
  bool isCar = ((tile >= 'A') && (tile <= 'Z'));
  return ((isGap) || (isBol) || (isCar));
}


bool isValidCp(Cp start) {
  int numCars = 0;
  Car cars[MAXCARS];
  for (int i = 0; i < MAXCARS; i++) {
    cars[i].name = '\0'; 
  }
  // getCars() throws errors if there are non-contiguous/straight cars
  numCars = getCars(&start, cars); 
  
  // Therefore, just need to check cars are of size > 1
  for (int i = 0; i < numCars; i++) {
    Car ithCar = cars[i];
    if (ithCar.size < 2) {
      return false;
    }
  }
  
  // And that the cars are named in order
  if (carsMisnamed(cars)) {
    return false;
  }
  
  return true;
}


int findSolutionBFS(Cp cpArray[], Cp tried[], State* state, Flags flag) {
  if (flag == ANIMATE) {
    animateState(&(cpArray[state->nextCp]), true);
  }
  while ((state->nextCp < state->endArray) && (state->endArray < LARGEARRAY)) {
    if (flag == ANIMATE) {
      animateState(&(cpArray[state->nextCp]), false);
    }
    
    if (isComplete(cpArray[state->nextCp])) {
      if (flag == SHOW) {
        printPath(cpArray[state->nextCp], cpArray);
      }
      return cpArray[state->nextCp].moves;
    }
    
    makeNextStates(&(cpArray[state->nextCp]), cpArray, tried, state);
    (state->nextCp)++;
  }
  return FAILS; 
}


bool isComplete(Cp cp) {
  for (int row = 0; row < cp.height; row++) {
    for (int col = 0; col < cp.width; col++) {
      char tile = cp.board[row][col]; 
      if ((tile >= 'A') && (tile <= 'Z')) {
        return false;
      }
    }
  }
  return true;
}


void makeNextStates(Cp* current, Cp cpArray[], Cp tried[], State* state) {
  int numCars = 0;
  Car cars[MAXCARS];
  for (int i = 0; i < MAXCARS; i++) {
    cars[i].name = '\0'; 
  }
  numCars = getCars(current, cars);
  for (int i = 0; i < numCars; i++) {
    moveCar(cars[i], current, cpArray, tried, state);
  }
}


int getCars(Cp* current, Car cars[]) {
  int numCars = 0;
  for (int row = 0; row < current->height; row++) {
    for (int col = 0; col < current->width; col++) {
      char tile = current->board[row][col];
      if ((tile >= 'A') && (tile <= 'Z')) {
        numCars = updateCars(row, col, tile, cars, numCars);
      }
    }
  }
  
  return numCars;
}


int updateCars(int row, int col, char tile, Car cars[], int numCars) {
  int carIndex = -1;
  for (int i = 0; i < numCars; i++) {
    if (cars[i].name == tile) {
      carIndex = i;
    }
  }
  
  if (carIndex >= 0) {
    updateCar(&(cars[carIndex]), row, col);
    return numCars;
  } else {
    addCar(row, col, tile, cars, numCars);
    return (numCars + 1);
  } 
}


void updateCar(Car* car, int row, int col) {  
  // carSize = 1 is tricky, because we don't yet know if it is vertical or not
  if (car->size == 1) {
    if ((row == car->startRow + 1) && (col == car->startCol)) {
      car->vertical = true;
    } else if ((row == car->startRow) && (col == car->startCol + 1)) {
      car->vertical = false;
    } else {
      throwError("ERROR: invalid car position (size = 1)\n");
    }
    
  // Otherwise, it's just a check of whether this tile is where it should be
  } else {
    bool validVert = ((car->vertical) && (row == car->startRow + car->size) && (col == car->startCol));
    bool validHorz = (!(car->vertical) && (col == car->startCol + car->size) && (row == car->startRow));
    if (!(validVert) && !(validHorz)) {
      throwError("ERROR: invalid car position (size > 1)\n");
    }    
  }
  (car->size)++;
}


void addCar(int row, int col, char tile, Car cars[], int numCars) {
  cars[numCars].name = tile;
  cars[numCars].startRow = row;
  cars[numCars].startCol = col;
  cars[numCars].size = 1;
}


bool carsMisnamed(Car cars[]) {
  bool previousUsed = true;
  for (char letter = 'A'; letter <= 'Z'; letter++) {
    bool letterUsed = false;
    for (int i = 0; i < MAXCARS; i++) {
      if (cars[i].name == letter) {
        letterUsed = true;
      }
    }
    if ((letterUsed) && (!previousUsed)) {
      return true;
    }
    previousUsed = letterUsed;
  }
  return false; 
}


void moveCar(Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state) {
  Location moveBack;
  Location moveForwards;
  getMoves(car, &moveBack, &moveForwards); 
  
  
  tryMove(moveBack, car, carpark, cpArray, tried, state);
  tryMove(moveForwards, car, carpark, cpArray, tried, state);
}


void getMoves(Car car, Location* moveBack, Location* moveForwards) {
  moveBack->row = (car.vertical) ? (car.startRow - 1) : car.startRow;
  moveBack->col = (car.vertical) ? car.startCol : (car.startCol - 1);
  moveForwards->row = (car.vertical) ? (car.startRow + car.size) : car.startRow;
  moveForwards->col = (car.vertical) ? car.startCol : (car.startCol + car.size);
}


void tryMove(Location move, Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state) {
  if (movePossible(move, carpark)) {
    if (atEdge(move, carpark)) {
      removeCar(car, carpark, cpArray, tried, state);
    } else {
      makeMove(move, car, carpark, cpArray, tried, state);
    }
  }
}


bool movePossible(Location move, Cp* carpark) {
  return (carpark->board[move.row][move.col] == GAP);
}


bool atEdge(Location move, Cp* carpark) {
  bool atTop = (move.row == 0);
  bool atLeft = (move.col == 0);
  bool atRight = (move.col == carpark->width - 1);
  bool atBottom = (move.row == carpark->height - 1);
  return ((atTop) || (atLeft) || (atRight) || (atBottom));
}


void removeCar(Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state) {
  Cp new;
  copyCarpark(&new, carpark);
  for (int row = 0; row < new.height; row++) {
    for (int col = 0; col < new.width; col++) {
      if (new.board[row][col] == car.name) {
        new.board[row][col] = GAP;
      }
    }
  }
  new.parents[new.moves] = state->nextCp;
  (new.moves)++;
  
  // Insert new carpark
  addCarpark(new, cpArray, tried, state);
}


void makeMove(Location move, Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state) {
  Cp new;
  copyCarpark(&new, carpark);
  
  // Get tiles to update
  Location newGap = getNewGap(move, car);
  
  // updateTiles
  new.board[move.row][move.col] = car.name;
  new.board[newGap.row][newGap.col] = GAP; 
  new.parents[new.moves] = state->nextCp;
  (new.moves)++;
  
  // Insert new carpark
  addCarpark(new, cpArray, tried, state);
}


Location getNewGap(Location move, Car car) {
  Location newGap;
  if (car.vertical) {
    newGap.row = (move.row == car.startRow - 1) ? car.startRow + (car.size - 1) : car.startRow;
    newGap.col = car.startCol;
  } else {
    newGap.row = car.startRow;
    newGap.col = (move.col == car.startCol - 1) ? car.startCol + (car.size - 1) : car.startCol;
  }
  return newGap;
}


void addCarpark(Cp carpark, Cp cpArray[], Cp tried[], State* state) {
  bool alreadyTried = false;
  for (int i = 0; i < state->endTried; i++) {
    if (carparksAreSame(carpark, tried[i])) {
      alreadyTried = true;
    }
  }
  
  if (!alreadyTried) {
    copyCarpark(&(cpArray[state->endArray]), &carpark);
    copyCarpark(&(tried[state->endTried]), &carpark);
    (state->endArray)++;
    (state->endTried)++; 
  }
}


bool carparksAreSame(Cp carpark1, Cp carpark2) {
  for (int row = 0; row < carpark1.height; row++) {
    for (int col = 0; col < carpark1.width; col++) {
      if (carpark1.board[row][col] != carpark2.board[row][col]) {
        return false;
      }
    }
  }
  return true;
}


void handleResult(int moves) {
  if (moves == FAILS) {
    printf("No Solution?\n");
  } else {
    printf("%i moves\n", moves);
  }
}


void throwError(char* errorMessage) {
  fputs(errorMessage, stderr);
  exit(EXIT_FAILURE);
}


void printCarpark(Cp carpark) {
  for (int row = 0; row < carpark.height; row++) {
    for (int col = 0; col < carpark.width; col++) {
      printf("%c", carpark.board[row][col]);
    }
    printf("\n");
  }
} 


void printPath(Cp carpark, Cp cpArray[]) {
  int moves = carpark.moves;
  for (int i = 0; i < moves; i++) {
    printCarpark(cpArray[carpark.parents[i]]);
    for (int i = 0; (i < (carpark.width - 1)) >> 1 ; i++) {
      printf(" ");
    }
    printf("\u2193\n");
  }
  printCarpark(carpark);
  printf("\n");
}


void copyCarpark(Cp* copy, Cp* original) {
  copy->width = original->width;
  copy->height = original->height;
  copy->moves = original->moves;
  for (int row = 0; row < copy->height; row++) {
    for (int col = 0; col < copy->width; col++) {
      copy->board[row][col] = original->board[row][col];
    }
  }
  for (int move = 0; move < copy->moves; move++) {
    copy->parents[move] = original->parents[move];
  } 
}



void animateState(Cp* carpark, bool first) {
  inefficientPause();
  if (!first) {
    printf("\x1b[%iD\x1b[%iA", carpark->width, carpark->height);
  }
  for (int row = 0; row < carpark->height; row++) {
    for (int col = 0; col < carpark->width; col++) {
      printTile(carpark->board[row][col]);
    }
    printf("\n");
  }
  printf("\x1b[0m"); // return to normal formatting etc.
}


void printTile(char value) {
  int colour = 31 + ((value - 'A') % 15); 
  switch (value) {
    case GAP:
      printf("\x1b[37m\u2591");
      break;
    case BOLLARD:
      printf("\x1b[30m\u2591");
      break;
    case 'A' ... 'Z':
      printf("\x1b[%im\u2588", colour);
      break;
    default:
      throwError("ERROR: carpark has an invalid character");
      break;
  }
}


// the below is used in the assert testing only
void strToCp(Cp* carpark, char* string) {
  int strIndex = 0;
  for (int row = 0; row < carpark->height; row++) {
    for (int col = 0; col < carpark->width; col++) {
      carpark->board[row][col] = string[strIndex];
      strIndex++;
    }
  }
}


void inefficientPause(void) {
  int i = 0;
  int b = 0;
  while (i++ < LARGENUM) {
    b++;
  }
}


int findSolutionDFS(Cp start, DFSstate* dfsState, Flags flag) {
  Stack* stack = (Stack*)allocateSpace(1, sizeof(Stack));
  
  DFScp* DFSstart = (DFScp*)allocateSpace(1, sizeof(DFScp));
  DFSstart->width = start.width;
  DFSstart->height = start.height;
  DFSstart->moves = start.moves;
  DFSstart->board = (char**)allocateSpace(DFSstart->height, sizeof(char*));
  for (int i = 0; i < DFSstart->height; i++) {
    DFSstart->board[i] = (char*)allocateSpace(DFSstart->width, sizeof(char));
    for (int j = 0; j < DFSstart->width; j++) {
      DFSstart->board[i][j] = start.board[i][j];
    }
  }
  
  stack->start = (DFS*)allocateSpace(1, sizeof(DFS));
  stack->start->carpark = DFSstart;
  
  if (flag == ANIMATE) {
    animateStateDFS(stack->start->carpark, true);
  }
  DFS* current = removeStack(stack);
  while (current) {
    if (flag == ANIMATE) {
      animateStateDFS(current->carpark, false);
    }
    
    if (isCompleteDFS(current->carpark)) {
      freeDFS(current);
      freeStack(stack);
      return current->carpark->moves;
    }
    
    makeNextStatesDFS(current->carpark, stack, dfsState);
    freeDFS(current);
    current = removeStack(stack);
  }
  freeStack(stack);
  return FAILS;
}


bool isCompleteDFS(DFScp* cp) {
  for (int row = 0; row < cp->height; row++) {
    for (int col = 0; col < cp->width; col++) {
      char tile = cp->board[row][col]; 
      if ((tile >= 'A') && (tile <= 'Z')) {
        return false;
      }
    }
  }
  return true;
}


void animateStateDFS(DFScp* carpark, bool first) {
  inefficientPause();
  if (!first) {
    printf("\x1b[%iD\x1b[%iA", carpark->width, carpark->height);
  }
  for (int row = 0; row < carpark->height; row++) {
    for (int col = 0; col < carpark->width; col++) {
      printTile(carpark->board[row][col]);
    }
    printf("\n");
  }
  printf("\x1b[0m"); // return to normal formatting etc.
}


void makeNextStatesDFS(DFScp* carpark, Stack* stack, DFSstate* dfsState) {
  int numCars = 0;
  Car cars[MAXCARS];
  for (int i = 0; i < MAXCARS; i++) {
    cars[i].name = '\0'; 
  }
  numCars = getCarsDFS(carpark, cars);
  for (int i = 0; i < numCars; i++) {
    moveCarDFS(cars[i], carpark, stack, dfsState);
  }
}


int getCarsDFS(DFScp* current, Car cars[]) {
  int numCars = 0;
  for (int row = 0; row < current->height; row++) {
    for (int col = 0; col < current->width; col++) {
      char tile = current->board[row][col];
      if ((tile >= 'A') && (tile <= 'Z')) {
        numCars = updateCars(row, col, tile, cars, numCars);
      }
    }
  }
  
  return numCars;
}


void moveCarDFS(Car car, DFScp* carpark, Stack* stack, DFSstate* dfsState) {
  Location moveBack;
  Location moveForwards;
  getMoves(car, &moveBack, &moveForwards); 
  
  
  tryMoveDFS(moveBack, car, carpark, stack, dfsState);
  tryMoveDFS(moveForwards, car, carpark, stack, dfsState);
}


void tryMoveDFS(Location move, Car car, DFScp* carpark, Stack* stack, DFSstate* dfsState) {
  if (movePossibleDFS(move, carpark)) {
    if (atEdgeDFS(move, carpark)) {
      removeCarDFS(car, carpark, stack, dfsState);
    } else {
      makeMoveDFS(move, car, carpark, stack, dfsState);
    }
  }
}


bool movePossibleDFS(Location move, DFScp* carpark) {
  return (carpark->board[move.row][move.col] == GAP);
}


bool atEdgeDFS(Location move, DFScp* carpark) {
  bool atTop = (move.row == 0);
  bool atLeft = (move.col == 0);
  bool atRight = (move.col == carpark->width - 1);
  bool atBottom = (move.row == carpark->height - 1);
  return ((atTop) || (atLeft) || (atRight) || (atBottom));
}


void removeCarDFS(Car car, DFScp* carpark, Stack* stack, DFSstate* dfsState) {
  DFScp* new = (DFScp*)allocateSpace(1, sizeof(Cp));
  copyCarparkDFS(new, carpark);
  for (int row = 0; row < new->height; row++) {
    for (int col = 0; col < new->width; col++) {
      if (new->board[row][col] == car.name) {
        new->board[row][col] = GAP;
      }
    }
  }
  (new->moves)++;
  
  // Insert new carpark
  addCarparkDFS(carpark, stack, dfsState);
}


void copyCarparkDFS(DFScp* copy, DFScp* original) {
  copy->width = original->width;
  copy->height = original->height;
  copy->moves = original->moves;
  copy->board = (char**)allocateSpace(copy->height, sizeof(char*));
  for (int row = 0; row < copy->height; row++) {
    copy->board[row] = (char*)allocateSpace(copy->width, sizeof(char));
    for (int col = 0; col < copy->width; col++) {
      copy->board[row][col] = original->board[row][col];
    }
  }
}


void makeMoveDFS(Location move, Car car, DFScp* carpark, Stack* stack, DFSstate* dfsState) {
  DFScp* new = (DFScp*)allocateSpace(1, sizeof(Cp));
  copyCarparkDFS(new, carpark);
  
  // Get tiles to update
  Location newGap = getNewGap(move, car);
  
  // updateTiles
  new->board[move.row][move.col] = car.name;
  new->board[newGap.row][newGap.col] = GAP; 
  (new->moves)++;
  
  // Insert new carpark
  addCarparkDFS(carpark, stack, dfsState);
}



void addCarparkDFS(DFScp* carpark, Stack* stack, DFSstate* dfsState) {
  bool alreadyTried = false;
  for (int i = 0; i < dfsState->endTried; i++) {
    if (carparksAreSameDFS(carpark, dfsState->tried[i])) {
      alreadyTried = true;
    }
  }
  
  if (!alreadyTried) {
    addStack(stack, carpark);
    copyCarparkDFS(dfsState->tried[dfsState->endTried], carpark);
    (dfsState->endTried)++; 
  }
}


bool carparksAreSameDFS(DFScp* carpark1, DFScp* carpark2) {
  for (int row = 0; row < carpark1->height; row++) {
    for (int col = 0; col < carpark1->width; col++) {
      if (carpark1->board[row][col] != carpark2->board[row][col]) {
        return false;
      }
    }
  }
  return true;
}




void addStack(Stack* stack, DFScp* carpark) {
  DFS* newNode = (DFS*)allocateSpace(1, sizeof(DFS));
  newNode->carpark = carpark;
  if (!(stack->start)) {
    stack->start = newNode;
  } else {
    DFS* current = stack->start;
    while (current->next) {
      current = current->next;
    }
    current->next = newNode;
  }
}


DFS* removeStack(Stack* stack) {
  if (stack->start) {
    DFS* toRemove = stack->start;
    stack->start = toRemove->next;
    return toRemove;
  }
  return NULL;
}


void* allocateSpace(int num, int size) {
  void* pointer = calloc(num, size);
  if (!pointer) {
    throwError("ERROR: unable to allocate space\n");
  }
  return pointer;
}


void freeDFS(DFS* node) {
  if (node) {
    for (int i = 0; i < node->carpark->height; i++) {
      free(node->carpark->board[i]);
    }
    free(node->carpark->board);
    free(node->carpark);
    free(node);
  }
}


void freeDFSRecursive(DFS* node) {
  if (node->next) {
    freeDFS(node->next);
    node->next = NULL;
  }
  if (node) {
    for (int i = 0; i < node->carpark->height; i++) {
      free(node->carpark->board[i]);
    }
    free(node->carpark->board);
    free(node->carpark);
    free(node);
  }
}


void freeStack(Stack* stack) {
  if (stack->start) {
    freeDFSRecursive(stack->start);
  }
  stack->start = NULL;
  free(stack);  
}


void test(void) {
  // Flags findFlags(int argc, char* argv[]);
  int argc = 3;
  char* argv[3];
  argv[0] = "./carpark";
  argv[1] = "carpark.prk";
  argv[2] = "-show";
  assert(findFlags(argc, argv) == SHOW);
  argv[2] = "something untowards";
  assert(findFlags(argc, argv) == INVALID);
  argc = 2;
  assert(findFlags(argc, argv) == NORMAL);
  
  // char* getFilename(int argc, char* argv[]);
  argc = 3;
  argv[2] = "-show";
  char* testFilename = getFilename(argc, argv);
  assert(strcmp(testFilename, "carpark.prk") == 0); // Should work - anticipated case
  argv[2] = argv[1];
  argv[1] = "-show";
  assert(strcmp(testFilename, "carpark.prk") == 0); // Should work - regardless of position of filename
  
  // bool isValidTile(char tile);
  char tile = 'A';
  assert(isValidTile(tile)); // Should work - valid car name
  tile = 'X';
  assert(isValidTile(tile)); // Should work - valid car name
  tile = GAP;
  assert(isValidTile(tile)); // Should work - gap character
  tile = BOLLARD;
  assert(isValidTile(tile)); // Should work - bollard character
  tile = 'a';
  assert(!isValidTile(tile)); // Shouldn't work - lowercase letter
  tile = ' ';
  assert(!isValidTile(tile)); // Shouldn't work - invalid character
  tile = '@';
  assert(!isValidTile(tile)); // Shouldn't work - invalid character
  tile = '\0';
  assert(!isValidTile(tile)); // Shouldn't work - invalid character
  
  // bool isValidCp(Cp start);
  Cp testCp;
  testCp.width = 6;
  testCp.height = 6;
  testCp.moves = 0;
  char* testString = "#.####.BBB.##A...##A...##A...#######";
  strToCp(&testCp, testString);
  assert(isValidCp(testCp)); // Should work - example from 'exercises in C' pdf
  testCp.board[3][1] = GAP;
  testCp.board[4][1] = GAP;
  assert(!isValidCp(testCp)); // Shouldn't work - 'A' is only one unit long
  testCp.board[3][1] = 'A';
  assert(isValidCp(testCp)); // Should work - 'A' is size 2
  testCp.board[4][3] = 'D';
  testCp.board[4][4] = 'D';
  assert(!isValidCp(testCp)); // Shouldn't work - invalid naming order
  
  // bool isComplete(Cp cp);
  testCp.board[3][1] = GAP;
  testCp.board[4][3] = GAP;
  testCp.board[4][4] = GAP;
  testCp.board[2][1] = GAP;
  assert(!isComplete(testCp)); // Shouldn't work - has B car in it
  testCp.board[1][1] = GAP;
  testCp.board[1][2] = GAP;
  testCp.board[1][3] = GAP;
  assert(isComplete(testCp)); // Should work - no cars remaining
  
  // void makeNextStates(Cp* current, Cp cpArray[], Cp tried[], State* state);
  testCp.board[1][1] = 'A';
  testCp.board[1][2] = 'A';
  testCp.board[1][3] = 'A';
  static Cp testcpArray[SMALLARRAY];
  static Cp testtried[SMALLARRAY];
  static State teststate = {0, 1, 1};
  testcpArray[0] = testtried[0] = testCp;
  makeNextStates(&testCp, testcpArray, testtried, &teststate);
  assert(isComplete(testcpArray[1])); // Should work - this state should see A go off the side
  assert(testcpArray[2].board[1][1] == GAP);
  assert(testcpArray[2].board[1][4] == 'A'); 
  
   
  // int getCars(Cp* current, Car cars[]);
  Car cars[MAXCARS];
  for (int i = 0; i < MAXCARS; i++) {
    cars[i].name = '\0';
  } 
  assert(getCars(&(testcpArray[1]), cars) == 0); // This CP is complete
  assert(getCars(&testCp, cars) == 1); // This CP only has 'A' in it
  testCp.board[2][1] = 'B';
  testCp.board[3][1] = 'B';
  testCp.board[4][1] = 'B';
  assert(getCars(&testCp, cars) == 2); // We've now added 'B' to this CP
    
  // int updateCars(int row, int col, char tile, Car cars[], int numCars);
  int numCars = 2;
  assert(updateCars(1, 4, 'A', cars, numCars) == 2); // Making a car larger shouldn't increase numCars
  assert(updateCars(4, 3, 'C', cars, numCars) == 3); // Adding new car ('C') should increase numCars
  numCars = 3;
  assert(updateCars(4, 4, 'C', cars, numCars) == 3); // Making a car larger shouldn't increase numCars
  
  // void updateCar(Car* car, int row, int col);
  Car testCar = {'A', 1, 1, 3, false};
  updateCar(&testCar, 1, 4);
  assert(testCar.size == 4); // Should work - adds 1 to size of car
  Car testCar2 = {'C', 3, 3, 1, false};
  updateCar(&testCar2, 4, 3);
  assert(testCar2.size == 2); // Should work - adds 1 to size of car
  assert(testCar2.vertical); // Should work - sets verticality of car previously size 1  
  
  // void addCar(int row, int col, char tile, Car cars[], int numCars);
  addCar(5, 5, 'D', cars, numCars);
  assert(cars[numCars].name == 'D'); // Should work - newcar added to array
  assert(cars[numCars].startRow == 5); // Should start at input tile
  assert(cars[numCars].startCol == 5); // Should start at input tile
  assert(cars[numCars].size == 1); // Should have size 1;
  numCars++;
  addCar(6, 6, 'F', cars, numCars);
  assert(cars[numCars].name == 'F'); // Should work - newcar added to array
  assert(cars[numCars].startRow == 6); // Should start at input tile
  assert(cars[numCars].startCol == 6); // Should start at input tile
  assert(cars[numCars].size == 1); // Should have size 1;
  numCars++;
  
  // bool carsMisnamed(Car cars[]);
  assert(carsMisnamed(cars)); // Shouldn't work - currently named A-B-C-D-F
  cars[numCars - 1].name = 'E';
  assert(!carsMisnamed(cars)); // Should work - now named A-B-C-D-E
  
  // void moveCar(Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state);
  Car carB = {'B', 3, 1, 2, true};
  testcpArray[2].board[3][1] = 'B';
  testcpArray[2].board[4][1] = 'B';
  moveCar(carB, &(testcpArray[2]), testcpArray, testtried, &teststate);
  assert(testcpArray[3].board[2][1] == 'B'); // Should add one state to the array, where B moves up one
  assert(testcpArray[3].board[4][1] == GAP);
  
  // void getMoves(Car car, Location* moveBack, Location* moveForwards);
  Location testMoveBack;
  Location testMoveFore;
  getMoves(carB, &testMoveBack, &testMoveFore);
  assert(testMoveBack.row == 2); // Should work for vertical cars
  assert(testMoveBack.col == 1);
  assert(testMoveFore.row == 5);
  assert(testMoveFore.col == 1);
  Car carA = {'A', 1, 2, 3, false};
  getMoves(carA, &testMoveBack, &testMoveFore);
  assert(testMoveBack.row == 1); // Should work for horizontal cars
  assert(testMoveBack.col == 1);
  assert(testMoveFore.row == 1);
  assert(testMoveFore.col == 5);
    
  // void tryMove(Location move, Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state);
  tryMove(testMoveBack, carA, &(testcpArray[2]), testcpArray, testtried, &teststate);
  assert(testcpArray[4].board[1][1] == 'A');
  assert(testcpArray[4].board[1][4] == GAP);
  testcpArray[5].moves = 0;
  tryMove(testMoveFore, carA, &(testcpArray[2]), testcpArray, testtried, &teststate);
  // This move is impossible - therefore next CP in array should be unchanged
  assert(testcpArray[5].moves == 0); 
  
  // bool movePossible(Location move, Cp* carpark);
  assert(movePossible(testMoveBack, &(testcpArray[2]))); // Should work
  assert(!movePossible(testMoveFore, &(testcpArray[2]))); // Shouldn't work - crashes into bollard
  testcpArray[2].board[1][1] = 'C';
  assert(!movePossible(testMoveBack, &(testcpArray[2]))); // Shouldn't work - crashes into car
  testcpArray[2].board[1][1] = GAP;
  
  // bool atEdge(Location move, Cp* carpark);
  Location testMove = {0, 4};
  assert(atEdge(testMove, &(testcpArray[2]))); // At top edge
  testMove.row = 3;
  testMove.col = 0;
  assert(atEdge(testMove, &(testcpArray[2]))); // At left edge
  testMove.row = 5;
  testMove.col = 1;
  assert(atEdge(testMove, &(testcpArray[2]))); // at bottom edge
  testMove.row = 2;
  testMove.col = 5;
  assert(atEdge(testMove, &(testcpArray[2]))); // at right edge
  testMove.row = 2;
  testMove.col = 3;
  assert(!atEdge(testMove, &(testcpArray[2]))); // In middle
  testMove.row = 1;
  testMove.col = 1;
  assert(!atEdge(testMove, &(testcpArray[2]))); // In middle
  testMove.row = 4;
  testMove.col = 2;
  assert(!atEdge(testMove, &(testcpArray[2]))); // In middle
  
  // void removeCar(Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state);
  removeCar(carA, &(testcpArray[2]), testcpArray, testtried, &teststate);
  assert(testcpArray[5].board[1][2] == GAP);
  assert(testcpArray[5].board[1][3] == GAP);
  assert(testcpArray[5].board[1][4] == GAP);
  removeCar(carB, &(testcpArray[5]), testcpArray, testtried, &teststate);
  assert(isComplete(testcpArray[6]));

  // void makeMove(Location move, Car car, Cp* carpark, Cp cpArray[], Cp tried[], State* state);
  testMove.row = 2;
  testMove.col = 1;
  makeMove(testMove, carB, &(testcpArray[5]), testcpArray, testtried, &teststate);
  assert(testcpArray[6].board[2][1] == 'B');
  assert(testcpArray[6].board[4][1] == GAP);
  testMove.row = 1;
  testMove.col = 1;
  carB.startRow = 2;
  makeMove(testMove, carB, &(testcpArray[6]), testcpArray, testtried, &teststate);
  assert(testcpArray[7].board[1][1] == 'B');
  assert(testcpArray[7].board[3][1] == GAP);
  

  // Location getNewGap(Location move, Car car);
  Location gap = getNewGap(testMove, carA);
  assert(gap.row == 1);
  assert(gap.col == 4);
  
  Car carC = {'C', 4, 4, 2, true};
  testMove.row = 3;
  testMove.col = 4;
  gap = getNewGap(testMove, carC);
  assert(gap.row == 5);
  assert(gap.col == 4);

  // void addCarpark(Cp carpark, Cp cpArray[], Cp tried[], State* state);
  // bool carparksAreSame(Cp carpark1, Cp carpark2);
  addCarpark(testcpArray[5], testcpArray, testtried, &teststate);
  assert(!carparksAreSame(testcpArray[5], testcpArray[8])); // cp#5 has already been tried, so shouldn't have been added
  
  
  // Finally, a larger-scale example to run assert-testing on
  /*
  typedef struct {
  int width;
  int height;
  int moves;
  char board[MAXSTR][MAXSTR];
  int parents[SMALLARRAY];
} Cp;
  */
  Cp example = {7, 10, 0, {{'#', '#', '#', '.', '#', '#', '#'}, {'#', 'D', 'B', 'B', 'B', 'E', '.'}, {'#', 'D', '.', 'A', '.', 'E', '#'}, {'#', 'D', '.', 'A', '.', 'E', '#'}, {'#', 'D', '.', 'A', '.', 'E', '#'}, {'#', 'D', '.', 'A', '.', 'E', '#'}, {'.', 'D', 'C', 'C', 'C', 'E', '#'}, {'#', 'D', '.', 'G', '.', 'E', '#'}, {'#', 'F', 'F', 'G', 'H', 'H', '.'}, {'#', '.', '#', '.', '#', '.', '#'}}, {0}}; 

}
