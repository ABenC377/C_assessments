#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 20
#define SMALLARRAY 100
#define LARGEARRAY 100000
#define GAP '.'
#define BOLLARD '#'
#define NUMFLAGS 3

typedef enum {SHOW, PRETTY, ANIMATE, FAST} Flags;

typedef struct {
  int row;
  int col;
} Location;

typedef struct {
  int width;
  int height;
  Location** bollards;
  int numBollards;
} Carpark;

typedef struct {
  char name;
  Location* start;
  int size;
  bool vertical;
} Vehicle;

typedef struct State {
  int numCars;
  Vehicle** cars;
  int moves;
  struct State* previous;
} State;

typedef struct {
  int queueEnd;
  int nextState;
  State** states;
  int triedEnd;
  State** tried;
} Queue;


bool* checkInputs(int argc, char* argv[]);
int getFlags(int argc, char* argv[], bool* flagsArray);
char* getFileName(int argc, char* argv[]);
void solveCarPark(char* fileName, bool* flagsArray);
void populateCarpark(FILE* fp, State* start, Carpark* cp);
void addBollard(Carpark* cp, int row, int col);
void addCarTile(State* state, char name, int row, int col);
void updateCar(Vehicle* car, int row, int col);
void addCar(State* state, char name, int row, int col);
void checkStart(State* start);
int findSolution(State* start, Carpark* cp, bool* flagsArray);
bool isComplete(State* state);
void findNextStates(State* start, Carpark* cp, Queue* statesToTry);
void findMoves(Location* move1, Location* move2, Vehicle* car);
void tryMove(Location* move, Vehicle* car, State* start, Carpark* cp, Queue* statesToTry);
void addNewState(Location* move, Vehicle* car, State* start, Queue* statesToTry, Carpark* cp);
int findCarIndex(char name, State* state);
void addToQueue(State* newState, Queue* statesToTry);
bool statesAreSame(State* state1, State* state2);
bool vehiclesAreSame(Vehicle* car1, Vehicle* car2);
void updateCarStart(Location* move, Vehicle* car);
bool moveIsBackwards(Location* move, Vehicle* car);
bool atEdge(Vehicle* car, Carpark* cp);
bool isValid(Location* move, Vehicle* car, State* start, Carpark* cp);
bool bollardClash(Location* move, Carpark* cp);
bool sameLocation(Location* loc1, Location* loc2);
bool carClash(Location* move, Vehicle* car, State* start);
bool locationInCar(Location* loc, Vehicle* car);
void* allocateSpace(int num, int size);
Vehicle* allocateVehicleSpace(void);
void freeVehicle(Vehicle* car);
State* allocateStateSpace(void);
void freeState(State* state);
Carpark* allocateCarparkSpace(void);
void freeCarpark(Carpark* cp);
Queue* allocateQueueSpace(State* start);
void freeQueue(Queue* queue);
State* deepCopyState(State* original);
Vehicle* deepCopyVehicle(Vehicle* original);
void throwError(char* errorMessage);
void printState(State* state, Carpark* cp, bool pretty);
char* getAscii(char tile);
char* getPrintArray(State* state, Carpark* cp);
void addCarToArray(Vehicle* car, char* array, Carpark* cp);
void printStateCars(State* state);
void printQueue(Queue* q, Carpark* cp);
void printPath(State* end, Carpark* cp, bool pretty);
void test(void);


int main(int argc, char* argv[]) {
  test();
  
  bool* flagsArray = checkInputs(argc, argv);
  char* fileName = getFileName(argc, argv);
  
  solveCarPark(fileName, flagsArray);
  free(flagsArray);
  
  return 0;
}


bool* checkInputs(int argc, char* argv[]) {
  if (argc < 2) {
    throwError("ERROR: correct usage = './carpark <carpark file> <flags (optional)>'\n");
  }

  bool* flagsArray = (bool*)allocateSpace(NUMFLAGS, sizeof(bool));
  if (getFlags(argc, argv, flagsArray) != 1) {
    throwError("ERROR: correct usage = './carpark <carpark file> <flags (optional)>'\n");
  }
  
  return flagsArray;
}


int getFlags(int argc, char* argv[], bool* flagsArray) {
  int numNonFlags = 0;
  
  for (int i = 1; i < argc; i++) {
    char* arg = argv[i];
    if (strcmp(arg, "-show") == 0) {
      flagsArray[SHOW] = true;
    } else if (strcmp(arg, "-pretty") == 0) {
      flagsArray[PRETTY] = true;
    } else if (strcmp(arg, "-animate") == 0) {
      flagsArray[ANIMATE] = true;
    } else if (strcmp(arg, "-fast") == 0) {
      flagsArray[FAST] = true;
    } else {
      numNonFlags++;
    }
  }

  return numNonFlags;
}


char* getFileName(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    char* arg = argv[i];
    if (arg[0] != '-') {
      return arg;
    }
  }
  throwError("ERROR: correct usage = './carpark <carpark file> <flags (optional)>'\n");
  return "";
}


void solveCarPark(char* fileName, bool* flagsArray) {
  State* start = allocateStateSpace();
  Carpark* cp = allocateCarparkSpace();
  
  FILE* fp = fopen(fileName, "r");
  if (!fp) {
    throwError("ERROR: unable to open file\n");
  }
  populateCarpark(fp, start, cp);
  fclose(fp);
  
  int moves = findSolution(start, cp, flagsArray);
  if (moves == -1) {
    printf("No Solution?\n");
  } else {
    printf("%i moves\n", moves);
  }
  freeState(start);
  freeCarpark(cp);
}


void populateCarpark(FILE* fp, State* start, Carpark* cp) {
  char* line = (char*)allocateSpace(MAXLINE, sizeof(char));
  line = fgets(line, MAXLINE, fp);
  
  if (sscanf(line, "%ix%i", &(cp->height), &(cp->width)) != 2) {
    throwError("ERROR: unable to read carpark file\n");
  }
  
  for (int row = 0; row < cp->height; row++) {
    line = fgets(line, MAXLINE, fp);
    for (int col = 0; col < cp->width; col++) {
      if (line[col] == BOLLARD) {
        addBollard(cp, row, col);
      } else if ((isupper(line[col])) && (isupper(line[col]))) {
        addCarTile(start, line[col], row, col);
      } else if (line[col] != GAP) {
        throwError("ERROR: invalid starting carpark\n");
      }
    }
  }
  free(line);
  checkStart(start); // To make sure it is valid (i.e., cars are longer than 1)
}


void addBollard(Carpark* cp, int row, int col) {
  cp->bollards[cp->numBollards] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[cp->numBollards]->row = row;
  cp->bollards[cp->numBollards]->col = col;
  cp->numBollards++;
}


void addCarTile(State* state, char name, int row, int col) {
  bool alreadyPresent = false;
  Vehicle* car;
  for (int i = 0; i < state->numCars; i++) {
    car = state->cars[i];
    if (car->name == name) {
      updateCar(car, row, col);
      alreadyPresent = true;
    }
  }
  
  if (!alreadyPresent) {
    addCar(state, name, row, col);
  } 
}


void updateCar(Vehicle* car, int row, int col) { // streamline this function!!!
  if ((row == car->start->row + car->size) && (col == car->start->col)) {
    if (car->size == 1) {
      car->vertical = true;
      (car->size)++;
    } else {
      if (car->vertical) {
        (car->size)++;
      } else {
        throwError("ERROR: invalid starting carpark\n");
      }
    }
  } else if ((row == car->start->row) && (col == car->start->col + car->size)) {
    if (car->size == 1) {
      car->vertical = false;
      (car->size)++;
    } else {
      if (car->vertical) {
        throwError("ERROR: invalid starting carpark\n");
      } else {
        (car->size)++;
      }
    }
  } else {
    throwError("ERROR: invalid starting carpark\n");
  }
}


void addCar(State* state, char name, int row, int col) {
  state->cars[state->numCars] = allocateVehicleSpace();
  Vehicle* car = state->cars[state->numCars];
  car->size = 1;
  car->start->row = row;
  car->start->col = col;
  car->name = name;
  (state->numCars)++;
}


void checkStart(State* start) {
  for (int i = 0; i < start->numCars; i++) {
    if (start->cars[i]->size < 2) {
      throwError("ERROR: invalid starting carpark\n");
    }
  }
}


int findSolution(State* start, Carpark* cp, bool* flagsArray) {
  Queue* statesToTry = allocateQueueSpace(start);
  
  State* current;
  while (statesToTry->nextState < statesToTry->queueEnd) {
    current = statesToTry->states[statesToTry->nextState]; 
    (statesToTry->nextState)++;

    
    if (isComplete(current)) {
      if (((flagsArray[SHOW]) || (flagsArray[PRETTY])) && !(flagsArray[ANIMATE])) {
        printPath(current, cp, flagsArray[PRETTY]); // This is a -show/-pretty feature
      } 
      int moves = current->moves;
      freeQueue(statesToTry);
      return moves;
    }
    
    findNextStates(current, cp, statesToTry);
  }
  
  // If no completed state found, then carpark impossible
  freeQueue(statesToTry);
  return -1;
}


bool isComplete(State* state) {
  for (int i = 0; i < state->numCars; i++) {
    if (state->cars[i]) {
      return false;
    }
  }
  return true;
}


void findNextStates(State* current, Carpark* cp, Queue* statesToTry) {
  Vehicle* car;
  for (int i = 0; i < current->numCars; i++) {
    if (current->cars[i]) {
      car = deepCopyVehicle(current->cars[i]);
      Location* move1 = (Location*)allocateSpace(1, sizeof(Location));
      Location* move2 = (Location*)allocateSpace(1, sizeof(Location));
      findMoves(move1, move2, car);  

      tryMove(move1, car, current, cp, statesToTry);
      tryMove(move2, car, current, cp, statesToTry);
    
      freeVehicle(car); free(move1); free(move2);
    }
  }
}


void findMoves(Location* move1, Location* move2, Vehicle* car) {
  Location* carStart = car->start;
  move1->row = (car->vertical) ? (carStart->row - 1) : carStart->row;
  move1->col = (car->vertical) ? carStart->col : (carStart->col - 1);
  move2->row = (car->vertical) ? (carStart->row + car->size) : carStart->row;
  move2->col = (car->vertical) ? carStart->col : (carStart->col + car->size);
}


void tryMove(Location* move, Vehicle* car, State* start, Carpark* cp, Queue* statesToTry) {
  if (isValid(move, car, start, cp)) {
    addNewState(move, car, start, statesToTry, cp); 
  }
}


void addNewState(Location* move, Vehicle* car, State* start, Queue* statesToTry, Carpark* cp) {
  State* newState = deepCopyState(start);
  (newState->moves)++;
  newState->previous = start;
  int carIndex = findCarIndex(car->name, newState);
  updateCarStart(move, newState->cars[carIndex]);
   
  if (atEdge(newState->cars[carIndex], cp)) {
    freeVehicle(newState->cars[carIndex]);
    newState->cars[carIndex] = NULL;
  }
  
  addToQueue(newState, statesToTry); 
  freeState(newState); // addToQueue() adds copies, therefore can free this State* now
}


int findCarIndex(char name, State* state) {
  Vehicle* ithCar;
  for (int i = 0; i < state->numCars; i++) {
    ithCar = state->cars[i]; 
    if ((ithCar) && (ithCar->name == name)) {
      return i;
    }
  }
  throwError("ERROR: car not found\n");
  return -1;
}


void addToQueue(State* newState, Queue* statesToTry) {  
  bool tried = false;
  for (int i = 0; i < statesToTry->triedEnd; i++) {
    if (statesAreSame(newState, statesToTry->tried[i])) {
      tried = true;
    }
  }
  if (!tried) {
    statesToTry->states[statesToTry->queueEnd] = deepCopyState(newState);
    (statesToTry->queueEnd)++;
    
    statesToTry->tried[statesToTry->triedEnd] = deepCopyState(newState);
    (statesToTry->triedEnd)++;
  } else {
  }
}


bool statesAreSame(State* state1, State* state2) {  
  for (int i = 0; i < state1->numCars; i++) {
    if (!vehiclesAreSame(state1->cars[i], state2->cars[i])) {
      return false;
    }
  }
  return true;
}


bool vehiclesAreSame(Vehicle* car1, Vehicle* car2) {
  if (!car1) {
    return (!car2);
  } else if (!car2) {
    return false;
  } else {
    if (car1->name != car2->name) {
      return false;
    }
    return sameLocation(car1->start, car2->start);
  }
}


void updateCarStart(Location* move, Vehicle* car) {
  Location* start = car->start;
  if (moveIsBackwards(move, car)) {
    start->row = (car->vertical) ? (start->row - 1) : start->row;
    start->col = (car->vertical) ? start->col : (start->col - 1);
  } else {
    start->row = (car->vertical) ? (start->row + 1) : start->row;
    start->col = (car->vertical) ? start->col : (start->col + 1);
  }
}


bool moveIsBackwards(Location* move, Vehicle* car) {
  return (car->vertical) ? (move->row == (car->start->row - 1)) : (move->col == (car->start->col - 1));
}


bool atEdge(Vehicle* car, Carpark* cp) {
  int r = car->start->row;
  int c = car->start->col;
  int s = car->size;
  int h = cp->height;
  int w = cp->width;
  bool v = car->vertical;
  return ((r == 0) || (c == 0) || ((v) && (r == h - s)) || ((!v) && (c == w - s)));
}


bool isValid(Location* move, Vehicle* car, State* start, Carpark* cp) {
  return ((!bollardClash(move, cp)) && (!carClash(move, car, start)));
}


bool bollardClash(Location* move, Carpark* cp) {
  for (int i = 0; i < cp->numBollards; i++) {
    if (sameLocation(move, cp->bollards[i])) {
      return true;
    }
  }
  return false;
}

bool sameLocation(Location* loc1, Location* loc2) {
  bool sameRow = (loc1->row == loc2->row);
  bool sameCol = (loc1->col == loc2->col);
  return ((sameRow) && (sameCol));
} 


bool carClash(Location* move, Vehicle* car, State* start) {
  for (int i = 0; i < start->numCars; i++) {
    Vehicle* ithCar = start->cars[i];
    if ((ithCar) && (ithCar->name != car->name)) {
      if (locationInCar(move, ithCar)) {
        return true;
      }
    }
  }
  return false;
}


bool locationInCar(Location* loc, Vehicle* car) {
  Location* carTile = (Location*)allocateSpace(1, sizeof(Location));
  for (int i = 0; i < car->size; i++) {
    carTile->row = (car->vertical) ? (car->start->row + i) : car->start->row;
    carTile->col = (car->vertical) ? car->start->col : (car->start->col + i);
    if (sameLocation(loc, carTile)) {
      free(carTile);
      return true;
    }
  }
  free(carTile);
  return false; 
}


void* allocateSpace(int num, int size) {
  void* pointer = calloc(num, size);
  if (!pointer) {
    throwError("ERROR: unable to allocate space\n");
  }
  return pointer;
}


Vehicle* allocateVehicleSpace(void) {
  Vehicle* car = (Vehicle*)allocateSpace(1, sizeof(Vehicle));
  car->start = (Location*)allocateSpace(1, sizeof(Location));
  return car;
}


void freeVehicle(Vehicle* car) {
  if (car) {
    if (car->start) {
      free(car->start);
    }
    free(car);
  }
}


State* allocateStateSpace(void) {
  State* state = (State*)allocateSpace(1, sizeof(State));
  state->cars = (Vehicle**)allocateSpace(SMALLARRAY, sizeof(Vehicle*));
  return state;
}


void freeState(State* state) {
  if (state) {
    for (int i = 0; i < state->numCars; i++) {
      if (state->cars[i]) {
        freeVehicle(state->cars[i]);
      }
    }
    free(state->cars);
    free(state);
  }
}


Carpark* allocateCarparkSpace(void) {
  Carpark* cp = (Carpark*)allocateSpace(1, sizeof(Carpark));
  cp->bollards = (Location**)allocateSpace(SMALLARRAY, sizeof(Location*));
  return cp;
}


void freeCarpark(Carpark* cp) {
  for (int i = 0; i < cp->numBollards; i++) {
    if (cp->bollards[i]) {
      free(cp->bollards[i]);
    }
  }
  free(cp->bollards);
  free(cp);
}


Queue* allocateQueueSpace(State* start) {
  Queue* newQueue = (Queue*)allocateSpace(1, sizeof(Queue));
  newQueue->states = (State**)allocateSpace(LARGEARRAY, sizeof(State*));
  newQueue->tried = (State**)allocateSpace(LARGEARRAY, sizeof(State*));
  
  newQueue->states[0] = deepCopyState(start);
  newQueue->tried[0] = deepCopyState(start);
  newQueue->queueEnd = newQueue->triedEnd = 1;
  
  return newQueue;
}


void freeQueue(Queue* queue) {
  State* current;  
  for (int i = 0; i < queue->queueEnd; i++) {
    current = queue->states[i];
    if (current) {
      freeState(current);
    }
  }
  free(queue->states);
  for (int i = 0; i < queue->triedEnd; i++) {
    current = queue->tried[i];
    if (current) {
      freeState(current);
    }
  }
  free(queue->tried);
  free(queue);
}


State* deepCopyState(State* original) {
  State* copy = allocateStateSpace();
  copy->numCars = original->numCars;
  copy->moves = original->moves;
  copy->previous = original->previous;
  for (int i = 0; i < copy->numCars; i++) {
    copy->cars[i] = deepCopyVehicle(original->cars[i]);
  }
  return copy;
}


Vehicle* deepCopyVehicle(Vehicle* original) {
  if (!original) {
    return NULL;
  }
  Vehicle* copy = allocateVehicleSpace();
  copy->name = original->name;
  copy->start->row = original->start->row;
  copy->start->col = original->start->col;
  copy->size = original->size;
  copy->vertical = original->vertical;
  return copy;
}


void throwError(char* errorMessage) {
  fputs(errorMessage, stderr);
  exit(EXIT_FAILURE);
}


void printState(State* state, Carpark* cp, bool pretty) {
  char* array = getPrintArray(state, cp);
  
  // Print array
  for (int row = 0; row < cp->height; row++) {
    for (int col = 0; col < cp->width; col++) {
      char tile = array[(row * cp->width) + col];
      if (pretty) {
        char* toPrint = getAscii(tile); 
        fputs(toPrint, stdout);
        if ((isupper(tile)) && (isupper(tile))) {
          free(toPrint);
        }
      } else {
        printf("%c", tile);
      }
    }
    printf("\n");
  }
  printf("\n");
  free(array);
}


char* getAscii(char tile) {
  char* literal = (char*)allocateSpace(SMALLARRAY, sizeof(char));
  switch (tile) {
    case GAP:
      free(literal);
      return "\x1b[15m\u2588";
    case BOLLARD:
      free(literal);
      return "\x1b[1m\u2588";
    case 'A' ... 'Z':
      sprintf(literal, "\x1b[%im\u2588", ((tile - 'A') * 39) % 231);
      return literal;
    default:
      free(literal);
      return " ";
  }
}


char* getPrintArray(State* state, Carpark* cp) {
  char* array = (char*)allocateSpace((cp->width * cp->height), sizeof(char));
  for (int i = 0; i < cp->width * cp->height; i++) {
    array[i] = GAP;
  }
  for (int i = 0; i < cp->numBollards; i++) {
    int index = (cp->width * cp->bollards[i]->row) + cp->bollards[i]->col;
    array[index] = BOLLARD;
  }
  for (int i = 0; i < state->numCars; i++) {
    Vehicle* current = state->cars[i];
    if (current) {
      addCarToArray(current, array, cp);
    }
  }
  return array;
}


void addCarToArray(Vehicle* car, char* array, Carpark* cp) {
  Location* start = car->start;
  for (int i = 0; i < car->size; i++) {
    Location carTile;
    carTile.row = (car->vertical) ? (start->row + i) : start->row;
    carTile.col = (car->vertical) ? start->col : (start->col + i);
    array[(carTile.row * cp->width) + carTile.col] = car->name;
  } 
}


void printPath(State* end, Carpark* cp, bool pretty) {
  State** pathArray = (State**)allocateSpace(SMALLARRAY, sizeof(State*));
  State* current = end;
  while (current) {
    pathArray[current->moves] = current;
    current = current->previous;
  }
  for (int i = 0; i <= end->moves; i++) {
    printState(pathArray[i], cp, pretty);
    printf("\n");
  }
}


void test(void) {
  // Set up test car
  Vehicle* car = allocateVehicleSpace();
  car->name = 'B';
  car->start->row = 5;
  car->start->col = 5;
  car->size = 2;
  car->vertical = true;
  
  // Set up test state
  Vehicle* otherCar1 = allocateVehicleSpace();
  otherCar1->name = 'A';
  otherCar1->start->col = 0;
  otherCar1->start->row = 5;
  otherCar1->size = 2;
  otherCar1->vertical = true;
  Vehicle* otherCar2 = allocateVehicleSpace();
  otherCar2->name = 'C';
  otherCar2->start->col = 2;
  otherCar2->start->row = 3;
  otherCar2->size = 4;
  otherCar2->vertical = false;
  State* start = allocateStateSpace();
  start->numCars = 3;
  start->cars[0] = otherCar1;
  start->cars[1] = car;
  start->cars[2] = otherCar2;
  
  // Set up test carpark
  Location* move = (Location*)allocateSpace(1, sizeof(Location));
  move->row = 0;
  move->col = 0;
  Location* b1 = (Location*)allocateSpace(1, sizeof(Location));
  b1->row = 1;
  b1->col = 1;
  Location* b2 = (Location*)allocateSpace(1, sizeof(Location));
  b2->row = 2;
  b2->col = 2;
  Location* b3 = (Location*)allocateSpace(1, sizeof(Location));
  b3->row = 3;
  b3->col = 3;
  Location* b4 = (Location*)allocateSpace(1, sizeof(Location));
  b4->row = 4;
  b4->col = 4;
  Carpark* cp = allocateCarparkSpace();
  cp->width = 8;
  cp->height = 8;
  cp->numBollards = 4;
  cp->bollards[0] = b1;
  cp->bollards[1] = b2;
  cp->bollards[2] = b3;
  cp->bollards[3] = b4;
  
  
  // void findNextStates(State* start, Carpark* cp, Queue* statesToTry);
  
  // void findMoves(Location* move1, Location* move2, Vehcile* currentCar);
  Location* move1 = (Location*)allocateSpace(1, sizeof(Location));
  Location* move2 = (Location*)allocateSpace(1, sizeof(Location));
  findMoves(move1, move2, car);
  assert(move1->row == 4);
  assert(move1->col == 5);
  assert(move2->row == 7);
  assert(move2->col == 5);
  car->vertical = false;
  findMoves(move1, move2, car);
  assert(move1->row == 5);
  assert(move1->col == 4);
  assert(move2->row == 5);
  assert(move2->col == 7);
  car->vertical = true;

  // void tryMove(Location* move, Vehicle* car, State* start, Carpark* cp, Queue* statesToTry);

  // bool isValid(Location* move, Vehicle* car, State* start, Carpark* cp);
  assert(isValid(move, car, start, cp)); // Should be valid
  move->row = 2;
  move->col = 2;
  assert(!isValid(move, car, start, cp)); // should hit a bollard #2
  move->row = 3;
  move->col = 4;
  assert(!isValid(move, car, start, cp)); // should hit other car #2
  
  
  // bool bollardClash(Location* move, Carpark* cp);
  move->row = 0;
  move->col = 0;
  assert(!bollardClash(move, cp));
  move->row = 1;
  assert(!bollardClash(move, cp));
  move->col = 1;
  assert(bollardClash(move, cp));
  move->row = 2;
  assert(!bollardClash(move, cp));

  // bool sameLocation(Location* loc1, Location* loc2);
  move->row = 0;
  move->col = 0;
  move1->row = 1;
  move1->col = 1;
  assert(!sameLocation(move, move1));
  move1->col = 0;
  assert(!sameLocation(move, move1));
  move1->row = 0;
  assert(sameLocation(move, move1));
  move1->col = 7;
  assert(!sameLocation(move, move1));

  // bool carClash(Location* move, Vehicle* car, State* start);
  move->row = 3;
  move->col = 3;
  
  assert(carClash(move, car, start)); // Moves into otherCar2
  otherCar2->vertical = true;
  assert(!carClash(move, car, start)); // otherCar2 no longer in the way


  // bool locationInCar(Location* loc, Vehicle* car);
  car->size = 3;
  car->start->row = 3;
  car->start->col = 1;
  car->vertical = false;
  assert(locationInCar(move, car));
  car->vertical = true;
  assert(!locationInCar(move, car));
  move->row = 0;
  move->col = 0;
  assert(!locationInCar(move, car));
  car->start->row = 0;
  car->start->col = 0;
  assert(locationInCar(move, car));
  car->vertical = false;
  assert(locationInCar(move, car));
  
  // int findSolution(State* start, Carpark* cp) - uses the example provided in the 'Exercises in C' PDF
  // there is a lot of set-up for this.
  start->numCars = 2;
  start->moves = 0;
  start->cars[0]->name = 'B';
  start->cars[0]->start->row = 1;
  start->cars[0]->start->col = 1;
  start->cars[0]->size = 3;
  start->cars[0]->vertical = false;
  start->cars[1]->name = 'A';
  start->cars[1]->start->row = 2;
  start->cars[1]->start->col = 1;
  start->cars[1]->size = 3;
  start->cars[1]->vertical = true;
  
  cp->width = 6;
  cp->height = 6;
  cp->numBollards = 18;
  cp->bollards[0]->row = 0;
  cp->bollards[0]->col = 0;
  cp->bollards[1]->row = 0;
  cp->bollards[1]->col = 2;
  cp->bollards[2]->row = 0;
  cp->bollards[2]->col = 3;
  cp->bollards[3]->row = 0;
  cp->bollards[3]->col = 4;
  cp->bollards[4] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[4]->row = 0;
  cp->bollards[4]->col = 5;
  cp->bollards[5] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[5]->row = 1;
  cp->bollards[5]->col = 5;
  cp->bollards[6] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[6]->row = 2;
  cp->bollards[6]->col = 0;
  cp->bollards[7] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[7]->row = 2;
  cp->bollards[7]->col = 5;
  cp->bollards[8] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[8]->row = 3;
  cp->bollards[8]->col = 0;
  cp->bollards[9] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[9]->row = 3;
  cp->bollards[9]->col = 5;
  cp->bollards[10] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[10]->row = 4;
  cp->bollards[10]->col = 0;
  cp->bollards[11] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[11]->row = 4;
  cp->bollards[11]->col = 5;
  cp->bollards[12] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[12]->row = 5;
  cp->bollards[12]->col = 0;
  cp->bollards[13] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[13]->row = 5;
  cp->bollards[13]->col = 1;
  cp->bollards[14] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[14]->row = 5;
  cp->bollards[14]->col = 2;
  cp->bollards[15] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[15]->row = 5;
  cp->bollards[15]->col = 3;
  cp->bollards[16] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[16]->row = 5;
  cp->bollards[16]->col = 4;
  cp->bollards[17] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[17]->row = 5;
  cp->bollards[17]->col = 5;
  // assert(findSolution(start, cp) == 3);
  
  
    
  // bool isComplete(State* state);
  freeVehicle(start->cars[0]);
  start->cars[0] = NULL;
  freeVehicle(start->cars[1]);
  start->cars[1] = NULL;
  car = NULL;
  freeVehicle(start->cars[2]);
  start->cars[2] = NULL;
  assert(isComplete(start));
  
  // Free the test variables
  free(move);
  free(move1);
  free(move2);
  freeVehicle(car);
  freeState(start);
  freeCarpark(cp);
  

  // void* allocateSpace(int num, int size);
  int* numPointer = (int*)allocateSpace(1, sizeof(int));
  assert(numPointer);
  free(numPointer);

  Carpark* carparkPointer = (Carpark*)allocateSpace(1, sizeof(Carpark));
  assert(carparkPointer);
  free(carparkPointer);

  char** charArrayPointer = (char**)allocateSpace(SMALLARRAY, sizeof(char*));
  assert(charArrayPointer);
  free(charArrayPointer);

  Location* locationPointer = (Location*)allocateSpace(1, sizeof(Location));
  assert(locationPointer);
  free(locationPointer);
}
