#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define GAP '.'
#define BOLLARD '#'
#define SHOWFLAG "-show"
#define MAXSIZE 21
#define LARGEARRAY 1000000

typedef struct {
  int row;
  int col;
} Location;

typedef struct {
  char name;
  Location* start;
  int size;
  bool vertical;
} Vehicle;

typedef struct {
  int width;
  int height;
  Location** bollards;
  int numBollards;
} Carpark

typedef struct {
  Vehicle** vehicles;
  int numVehicles;
  int moves;
} State

void checkInputs(int argc, char* argv[]);
bool showFlagPresent(int argc, char* argv[]);
State* loadCarpark(char* fileName, Carpark* cp);
void updateBollards(Carpark* cp, int row, int col);
void updateVehicles(State* state, int row, int col, char c);
void addVehicle(State* state, int row, int col, char c);
Carpark* allocateCarpark(void);
State* allocateState(void);
void* allocateSpace(int num, int size); 
void reallocateSpace(void* pointer, int size);
void throwError(const char* message);
int solveCarpark(Carpark* cp, State* startingState);
void test(void);


int main(int argc, char* argv[]) {
  test();
  checkInputs(argc, argv);
  bool show = showFlagPresent(argc, argv);

  Carpark* cp = allocateCarpark();
  State* startingState = loadCarpark(fileName, cp); // need to work out which argv is the fileName

  int moves = solveCarpark(cp, startingState);
  if (moves == -1) {
    failure();
  } else {
    success(moves);
  }

  return 0;
}


void checkInputs(int argc, char* argv[]) {
  if ((argc != 2) && (argc != 3)) {
    throwError("ERROR: usage = './carpark <carpark.prk>\n");
  }

  if ((argc == 3) && ((strcmp(argv[1], SHOWFLAG)) && (strcmp(argv[2], SHOWFLAG)))) {
    throwError("ERROR: only valid flag is '-show'\n");
  }
}


bool showFlagPresent(int argc, char* argv[]) {
  bool correctNumArgs = (argc == 3);
  bool includesShow = ((strcmp(argv[1], SHOWFLAG) == 0) || (strcmp(argv[2], SHOWFLAG) == 0));
  return (correctNumArgs && includesShow);
}


State* loadCarpark(char* fileName, Carpark* cp) {
  State* startingState = allocateState();
  startingState->moves = 0;
  
  FILE* fp = fopen(fileName, "r");
  char* line = allocateSpace(MAXSIZE, sizeof(char));
  line = fgets(line, MAXSIZE, fp);
  int width, height;
  if (sscanf(line, "%ix%i", &height, &width) != 2) {
    throwError("Unable to read from file\n");
  }

  for (int i = 0; i < height, i++) {
    line = fgets(line, MAXSIZE, fp);
    for (int j = 0; j < width; j++) {
      if (line[j] == BOLLARD) {
        updateBollards(cp, i, j);
      } else if ((isalpha(line[j])) && (isupper(line[j]))) {
        updateVehicles(startingState, i, j, line[j]);
      } else if (line[j] != GAP) {
        throwError("Invalid car park definition file\n");
    }
  }

  free(line);
  fclose(fp);
  return startingState;
}



void updateBollards(Carpark* cp, int row, int col) {
  cp->numBollards++;
  reallocateSpace(cp->bollards, cp->numBollards * sizeof(Location*));
  cp->bollards[numBollards - 1] = (Location*)allocateSpace(1, sizeof(Location));
  cp->bollards[numBollards - 1]->row = row;
  cp->bollards[numBollards - 1]->col = col;
}


void updateVehicles(State* state, int row, int col, char c) {
  bool updated = false;
  // Check if this is just another segment of a known car
  for (int i = 0; i < state->numVehicles; i++) {
    Vehicle* currentVehicle = state->vehicles[i]; 
    if (currentVehicle->name == c) {
      // Need to do a check that this piece of car fits with the previously found bits!!!!!!!
      currentVehicle->verticle = (currentVehicle->start->col == col);
      currentVehicle->size++;
      updated = true;
    }
  }
  // Otherwise, make a new car isntance
  if (!updated) {
    addVehicle(state, row, col, c);
  } 
}


void addVehicle(State* state, int row, int col, char c) {
  state->numVehicles++;
  reallocateSpace(state->vehicles, state->numVehicles * sizeof(Vehicle*));
  state->vehicles[state->numVehicles - 1] = (Vehicle*)allocateSpace(1, sizeof(Vehicle));
  // Populate the properties of this new vehicle
  Vehicle* newVehicle = state->vehicles[state->numVehicles - 1];
  newVehicle->name = c;
  newVehicle->size = 1;
  newVehicle->start = (Location*)allocateSpace(1, sizeof(Location));
  Location* newLocation = newVehicle->start;
  newLocation->row = row;
  newLocation->col = col;
}


Carpark* allocateCarpark(void) {
  Carpark* cp = (Carpark*)allocateSpace(1, sizeof(Carpark));
  cp->bollards = (Location**)allocateSpace(1, sizeof(Location*));
  return cp;
}


State* allocateState(void) {
  State* state = (State*)allocateSpace(1, sizeof(State));
  state->vehicles = (Vehicle**)allocateSpace(1, sizeof(Vehicle*));
  return cp;
}


void* allocateSpace(int num, int size) {
  void* pointer = calloc(num, size);
  if (!pointer) {
    throwError("ERROR: unable to allocate space\n");
  }
  return pointer;
}


void reallocateSpace(void* pointer, int size) {
  realloc(pointer, size);
  if (!pointer) {
    throwError("ERROR: unable to reallocate space\n");
  }
}


void throwError(const char* message) {
  fprintf(stderr, message);
  exit(EXIT_FAILURE);
}


int solveCarpark(Carpark* cp, State* startingState) {
  State** queue = (State*)allocateSpace(LARGEARRAY, sizeof(State*));
  queue[0] = startignState;
  int toCheck, lastElement;
  toCheck = lastElement = 0;

  // Some variable called 'Considered' to track which states have been checked

  while (toCheck <= lastElement) {
    State* stateToConsider = queue[toCheck];
    toCheck++;
    statesAdded = findMoves(cp, queue, stateToConsider, considered);
    if (statesAdded == -1) {
      return (stateToConsider->moves) + 1;
    } else {
      lastElement += statesAdded;
    }
  }
  return -1;
}


int findMoves(Carpark* cp, State** queue, State* stateToConsider, /*unknownType considered*/) {
  int newStates = 0;
  for (int carNum = 0; carNum < stateToConsider->numVehicles; carNum++) {
    Vehicle* car = stateToConsider->vehicles[carNum];
    Location move1, move2;
    getMoves(car, &move1, &move2);
    if (tryMove(car, &move1, cp, queue, stateToConsider, /*considered*/)) {
      newStates++;
    } 
    if (tryMove(car, &move2, cp, queue, stateToConsider, /*considered*/)) {
      newState++;
    }
  }
  return newStates;
}


void getMoves(Vehicle* car, Location* move1, Location* move2) {
  if (car->verticle) {
    move1->row = car->start->row - 1;
    move2->row = car->start->row + car->size;
    move1->col = move2->col = car->start->col;
  } else {
    move1->col = car->start->col - 1;
    move2->col = car->start->col + car->size;
    move1->row = move2->row = car->start->row;
  }
}


bool tryMove(Vehicle* car, Location* move, Carpark* cp, State** queue, State* state, /*unknownType considered*/) {
  if (isPossible(car, move, cp, state)) {
    State* potentialState = makeState(cp, state, car, &move1);
    if (notTriedYet(potentialState, /*unknownType considered*/)) {
      addState(queue, potentialState);
      return true;
    } else {
      free(potentialState);
    }
  }
  return false;
}


bool isPossible(Vehicle* car, Location* move, Carpark* cp, State* state) {
  return ((noBollards(move, cp)) && (noCars(car, move, cp, state)));
}


bool noBollards(Location* move, Carpark* cp) {
  for (int i = 0; i < cp->numBollards; i++) {
    bool rowClash = (move->row == cp->bollards[i]->row);
    bool colClash = (move->col == cp->bollards[i]->col);
    if (rowClash && colClash) {
      return false;
    }
  }
  return true;
}


bool noCars(Vehicle* car, Location* move, Carpark* cp, State* state) {
  for (int i = 0; i < state->numVehicles; i++) {
    Vehicle* otherCar = state->vehicles[i];
    if ((car->name != otherCar->name) && (isCarClash(move, otherCar))) {
      return false;
    }
  }
  return true;
}


bool isCarClash(Location* move, Vehicle* otherCar) {
  Location* potentialClash = (Location*)allocateSpace(1, sizeof(Location));
  for (int i = 0; i < otherCar->size; i++) {
    potentialClash->row = (otherCar->verticle) ? (row + i) : row;
    potentialClash->col = (otherCar->verticle) ? col : (col + i);
    if ((potentialClash->row == move->row) && (potentialClash->col == move->col)) {
      free(potentialClash);
      return true;
    }
  }
  free(potentialClash);
  return false;  
}


State* makeState(Carpark* cp, State* startingState, Vehicle* car,  Location* move) {
  State* newState = (State*)allocateSpace(1, sizeof(State));
  newState->moves = startingState->moves + 1;

  for (int i = 0; i < startingState->numVehicles; i++) {
    newState->vehicles = (Vehicle**)allocateSpace(startingState->numVehicles, sizeof(Vehicle*));
    Vehicle* stateCar = copyVehicle(startingState->vehicles[i]);
    if (stateCar->name == car->name) {
      if (car->vertical) {
        stateCar->start->row = (move->row = stateCar->start->row - 1) ? (stateCar->start->row - 1) : (stateCar->start->row + 1);
      } else {
        stateCar->start->col = (move->col = stateCar->start->col - 1) ? (stateCar->start->col - 1) : (stateCar->start->col + 1);
      }
      if (isAtEdge(cp, stateCar)) {
        removeCar(newState, stateCar);
      }
    }
  }
  retrun newState; 
}


Vehicle* copyVehicle(Vehicle* original) {
  Vehicle* copy = (Vehicle*)allocateSpace(1, sizeof(Vehicle));
  copy->name = original->name;
  copy->start = (Location*)allocateSpace(1, sizeof(Location));
  copy->start->row = original->start->row;
  copy->start->col = original->start->col;
  copy->size = original->size;
  copy->vertical - original->vertical;
  return copy;
}


void failure(void) {
  printf("No Solution?\n");
}


void success(int moves) {
  printf("%i moves\n", moves);
}


void test(void) {
  assert(true);
}
