#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 20
#define GAP '.'
#define BOLLARD '#'
#define NUMFLAGS 3

typedef enum {SHOW, PRETTY, ANIMATE} Flags;

typedef struct {
  int row;
  int col;
} Location;

typedef struct {
  int height;
  int width;
  Location** bollards;
  int numBollards;
} Cp;

typedef struct {
  bool escaped;
  bool vertical;
  char name;
  Location* start;
  int size;
} Car;

typedef struct Node {
  bool tried;
  Car** cars;
  int numCars;
  int moves;
  struct Node* parent;
  struct Node** children;
  int numChildren;
} Node;

typedef struct Fail {
  char* names;
  int* rows;
  int* cols;
  int numCars;
  struct Fail* next;
} Fail;

typedef struct {
  Node* root;
  Fail* tried;
  Cp* carpark;
} Tree;

bool* checkInputs(int argc, char* argv[]);
int getFlags(int argc, char* argv[], bool* flagsArray);
char* getFileName(int argc, char* argv[]);
void solveCarPark(char* fileName, bool* flagsArray);
void handleSolution(int moves);
void populateCarpark(FILE* fp, Node* start, Cp* cp);
void addBollard(Cp* carpark, int row, int col);
void addCarTile(Node* start, char name, int row, int col);
void updateCar(Car* car, int row, int col);
void addCar(Node* start, char name, int row, int col);
int findSolution(Node* start, Cp* carpark);
void freeCarpark(Cp* carpark);
void freeFailList(Fail* list);
void freeTree(Tree* tree);
void freeNodeTree(Node* node);
int clearBoard(Node* current, Cp* carpark, Tree* tree);
void addFail(Fail* fail, Tree* tree);
Fail* getFailFromNode(Node* node);
bool nodeHasFailed(Node* node, Tree* tree);
bool nodeSameAsFail(Node* node, Fail* fail);
bool isSolved(Node* state);
Node* findBestChild(Node* parent, Cp* carpark, Tree* tree);
int findMovesToRemoveACar(Node* state, Cp* carpark);
int findMovesToRemoveThisCar(Car* car, Node* state, Cp* carpark);
Car** reallocateCarArray(Car** array, int num);
bool isBlocked(Car* car, Node* state, Cp* carpark, bool ahead);
void makeChildren(Node* state, Cp* carpark);
void getMoves(Location* moveBack, Location* moveFore, Car* car);
bool isValid(Location* move, Cp* carpark, Node* state);
bool bollardClash(Location* move, Cp* carpark);
bool sameLocations(Location* loc1, Location* loc2);
bool carClash(Location* move, Node* state);
bool moveInCar(Location* move, Car* car);
void makeChild(Location* move, char carName, Node* state, Cp* cp);
Node** reallocateNodeArray(Node** array, int num);
Location** reallocateLocationArray(Location** array, int num);
void moveCar(Location* move, char carName, Node* state, Cp* cp);
void implementMove(Location* move, Car* car);
bool atEdge(Car* car, Cp* cp);
Node* copyNodeNoChild(Node* original);
Car* copyCar(Car* original);
void* allocateSpace(int num, int size);
Node* allocateNode(void);
void freeNode(Node* node);
Car* allocateCar(void);
void freeCar(Car* car);
Tree* makeTree(Node* root, Cp* cp);
Cp* allocateCarpark(void);
void throwError(const char* message);
void printState(Node* state, Cp* cp);
char* getPrintArray(Node* state, Cp* cp);
void addCarToArray(Car* car, char* array, Cp* cp);
void printFail(Fail* fail);
void test(void);


int main(int argc, char* argv[]) {
  test();
  
  bool* flagsArray = checkInputs(argc, argv);
  char* fileName = getFileName(argc, argv);
  
  solveCarPark(fileName, flagsArray);
  free(flagsArray);
  
  return EXIT_SUCCESS;
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
  Node* start = allocateNode();
  Cp* cp = allocateCarpark();
  
  FILE* fp = fopen(fileName, "r");
  if (!fp) {
    throwError("ERROR: unable to open file\n");
  }
  populateCarpark(fp, start, cp);
  fclose(fp);
  
  int moves = findSolution(start, cp);
  handleSolution(moves);
}


void handleSolution(int moves) {
  if (moves == -1) {
    printf("No Solution?\n");
  } else {
    printf("%i moves\n", moves);
  }
}


void populateCarpark(FILE* fp, Node* start, Cp* cp) {
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
  // checkStart(start); // To make sure it is valid (i.e., cars are longer than 1)
}


void addBollard(Cp* carpark, int row, int col) {
  (carpark->numBollards)++;
  carpark->bollards = reallocateLocationArray(carpark->bollards, carpark->numBollards);
  Location* newBollard = (Location*)allocateSpace(1, sizeof(Location));
  newBollard->row = row;
  newBollard->col = col;
  carpark->bollards[carpark->numBollards - 1] = newBollard;
}


void addCarTile(Node* start, char name, int row, int col) {
  bool alreadyPresent = false;
  Car* car;
  for (int i = 0; i < start->numCars; i++) {
    car = start->cars[i];
    if (car->name == name) {
      updateCar(car, row, col);
      alreadyPresent = true;
    }
  }
  
  if (!alreadyPresent) {
    addCar(start, name, row, col);
  } 
}


void updateCar(Car* car, int row, int col) {
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


void addCar(Node* start, char name, int row, int col) {
  start->cars = reallocateCarArray(start->cars, start->numCars + 1);
  start->cars[start->numCars] = (Car*)allocateSpace(1, sizeof(Car));
  Car* car = start->cars[start->numCars];
  car->size = 1;
  car->start = (Location*)allocateSpace(1, sizeof(Location));
  car->start->row = row;
  car->start->col = col;
  car->name = name;
  (start->numCars)++;
}


Car** reallocateCarArray(Car** array, int num) {
  array = (Car**)realloc(array, num * sizeof(Car*));
  if (!array) {
    throwError("ERROR: unable to reallcoate space\n");
  }
  return array;
}


int findSolution(Node* start, Cp* carpark) {
  Tree* tree = makeTree(start, carpark);
  printf("About to start the recursive function\n"); // debugging
  printf("starting with vvv\n"); // debugging
  printState(start, carpark); // debugging
  int moves = clearBoard(start, carpark, tree); 
  freeTree(tree);
  return moves; 
}


void freeTree(Tree* tree) {
  freeCarpark(tree->carpark);
  freeFailList(tree->tried);
  freeNodeTree(tree->root);
}


void freeCarpark(Cp* carpark) {
  for (int i = 0; i < carpark->numBollards; i++) {
    free(carpark->bollards[i]);
  }
  free(carpark);
}


void freeFailList(Fail* list) {
  if (list) {
    if (list->next) {
      freeFailList(list->next);
    }
    free(list);
  }
}


void freeNodeTree(Node* node) {
  if (node->numChildren > 0) {
    for (int i = 0; i < node->numChildren; i++) {
      freeNodeTree(node->children[i]);
    }
  }
  free(node);
}


int clearBoard(Node* current, Cp* carpark, Tree* tree) {
  if (!current) {
    return -1;
  }
  printf("\n\nCalling the recursive function on the below state\n"); // debugging
  printState(current, carpark); // debugging
  // recursive base-case
  if (isSolved(current)) {
    return current->moves;
  }
  
  //printf("Not solved yet\n"); // debugging
  int moves;
  makeChildren(current, carpark);
  printf("There are %i children\n", current->numChildren); // debugging
  Node* child = findBestChild(current, carpark, tree);
  int childrentried = 0; // debugging
  while (child) {
    printf("Trying child %i\n", childrentried + 1); // debugging
    if (!(child->tried) && !(nodeHasFailed(child, tree))) {
      child->tried = true;
      moves = clearBoard(child, carpark, tree);
      if (moves != -1) {
        return moves;
      }
    }
    addFail(getFailFromNode(child), tree);
    child = findBestChild(current, carpark, tree);
  }
  
  // If no children then return -1
  /* 
  current->tried = true;
  Fail* fail = getFailFromNode(current);
  addFail(fail, tree);
  */
  return -1;  
}


void addFail(Fail* fail, Tree* tree) {
  if (!(tree->tried)) {
    tree->tried = fail;
  } else {
    Fail* current = tree->tried;
    while (current->next) {
      current = current->next;
    }
    current->next = fail;
  }    
}


Fail* getFailFromNode(Node* node) {
  Fail* newFail = (Fail*)allocateSpace(1, sizeof(Fail));
  newFail->numCars = node->numCars;
  newFail->names = (char*)allocateSpace(newFail->numCars, sizeof(char));
  newFail->rows = (int*)allocateSpace(newFail->numCars, sizeof(int));
  newFail->cols = (int*)allocateSpace(newFail->numCars, sizeof(int));
  for (int i = 0; i < newFail->numCars; i++) {
    newFail->names[i] = node->cars[i]->name;
    newFail->rows[i] = node->cars[i]->start->row;
    newFail->cols[i] = node->cars[i]->start->col;
  }
  return newFail;
}


bool nodeHasFailed(Node* node, Tree* tree) {
  Fail* current = tree->tried;
  while (current) {
    printFail(current); // debugging
    printState(node, tree->carpark); // debugging
    if (nodeSameAsFail(node, current)) {
      return true;
    }
    printf("Not the same\n"); // debugging
    current = current->next;
  }
  return false;
}


void printFail(Fail* fail) {
  for (int i = 0; i < fail->numCars; i++) {
    printf("Car #%c R:%i C:%i ", fail->names[i], fail->rows[i], fail->cols[i]); 
  }
  printf("\n"); 
}


bool nodeSameAsFail(Node* node, Fail* fail) {
  if (node->numCars != fail->numCars) {
    return false;
  } else {
    for (int i = 0; i < node->numCars; i++) {
      bool sameName = (fail->names[i] == node->cars[i]->name);
      bool sameRow = (fail->rows[i] == node->cars[i]->start->row);
      bool sameCol = (fail->cols[i] == node->cars[i]->start->col);
      if (!sameName || !sameRow || !sameCol) {
        return false;
      }
    }
  }
  return true;
}


bool isSolved(Node* state) {
  for (int car = 0; car < state->numCars; car++) {
    if (!(state->cars[car]->escaped)) {
      return false;
    }
  }
  return true;
}


Node* findBestChild(Node* parent, Cp* carpark, Tree* tree) {
  Node* best = NULL;
  int fewestMoves = MAXLINE;
  for (int i = 0; i < parent->numChildren; i++) {
    if (isSolved(parent->children[i])) {
      return parent->children[i];
    }
    if ((parent->children[i]->tried) || (nodeHasFailed(parent->children[i], tree))) {
      parent->children[i]->tried = true;
      continue; // please forgive me
    }
    if (!best) {
      best = parent->children[i];
      fewestMoves = findMovesToRemoveACar(best, carpark);
    } else {
      int moves = findMovesToRemoveACar(parent->children[i], carpark);
      if (moves != -1) {
        if (fewestMoves == -1) {
          best = parent->children[i];
          fewestMoves = moves;
        } else { 
          best = (moves < fewestMoves) ? parent->children[i] : best; 
          fewestMoves = (moves < fewestMoves) ? moves : fewestMoves;
        }
      }
    }
  }
  return best; // Remember that this might be NULL.   
}


int findMovesToRemoveACar(Node* state, Cp* carpark) {
  int fewest = -1;
  for (int i = 0; i < state->numCars; i++) {
    if (fewest == -1) {
      fewest = findMovesToRemoveThisCar(state->cars[i], state, carpark);
    } else {
      int moves = findMovesToRemoveThisCar(state->cars[i], state, carpark);
      if (moves != -1) {
        if (fewest == -1) {
          fewest = moves;
        } else {
          fewest = (moves < fewest) ? moves : fewest;
        }
      }
    }
  }
  return fewest; 
}


int findMovesToRemoveThisCar(Car* car, Node* state, Cp* carpark) {
  int distBack = (car->vertical) ? car->start->row : car->start->col;
  int distFore = (car->vertical) ? (carpark->height - (car->start->row + car->size)) : (carpark->width - (car->start->col + car->size));
  bool blockedBehind = isBlocked(car, state, carpark, false);
  bool blockedAhead = isBlocked(car, state, carpark, true);
  int min = MAXLINE;
  min = (!(blockedBehind) && (distBack < min)) ? distBack : min;
  min = (!(blockedAhead) && (distFore < min)) ? distFore : min;
  return min;
}


bool isBlocked(Car* car, Node* state, Cp* carpark, bool ahead) {
  Location tile;
  int start;
  if (ahead) {
    start = (car->vertical) ? (car->start->row + 1) : (car->start->col + 1);
    for (int i = start; i < (car->vertical) ? carpark->height : carpark->width; i++) {
      tile.row = (car->vertical) ? i : car->start->col;
      tile.col = (car->vertical) ? car->start->row : i;
      if (!isValid(&tile, carpark, state)) {
        return true;
      }
    }
  } else {
    start = (car->vertical) ? car->start->row - 1 : car->start->col - 1;
    for (int i = start; i >= 0; i--) {
      tile.row = (car->vertical) ? i : car->start->col;
      tile.col = (car->vertical) ? car->start->row : i;
      if (!isValid(&tile, carpark, state)) {
        return true;
      }
    }
  }
  return false;
}


void makeChildren(Node* state, Cp* carpark) { 
  //printf("There are %i cars\n", state->numCars); // debugging
  for (int car = 0; car < state->numCars; car++) {
    if (!(state->cars[car]->escaped)) {
      //printf("Car %c has not escaped yet\n", state->cars[car]->name); // debugging
      Location moveBack, moveFore;
      getMoves(&moveBack, &moveFore, state->cars[car]);
      //printf("Move 1 is R:%i C:%i, move 2 is R:%i C:%i\n", moveBack.row, moveBack.col, moveFore.row, moveFore.col); // debugging
      if (isValid(&moveBack, carpark, state)) {
        //printf("Adding a child with move 1\n"); // debugging
        makeChild(&moveBack, state->cars[car]->name, state, carpark);
      }
      if (isValid(&moveFore, carpark, state)) {
        //printf("Adding a child with move 2\n"); // debugging
        makeChild(&moveFore, state->cars[car]->name, state, carpark);
      } 
    }
  }
}


void getMoves(Location* moveBack, Location* moveFore, Car* car) {
  moveBack->row = (car->vertical) ? (car->start->row - 1) : car->start->row;
  moveBack->col = (car->vertical) ? car->start->col : (car->start->col - 1);
  moveFore->row = (car->vertical) ? (car->start->row + car->size) : car->start->row;
  moveFore->col = (car->vertical) ? car->start->col : (car->start->col + car->size);
}


bool isValid(Location* move, Cp* carpark, Node* state) {
  return (!(bollardClash(move, carpark)) && !(carClash(move, state)));
}


bool bollardClash(Location* move, Cp* carpark) {
  for (int i = 0; i < carpark->numBollards; i++) {
    if (sameLocations(move, carpark->bollards[i])) {
      //printf("Bollard Clash\n"); // debugging
      return true;
    }
  }
  return false;
}


bool sameLocations(Location* loc1, Location* loc2) {
  bool rowSame = (loc1->row == loc2->row);
  bool colSame = (loc1->col == loc2->col);
  return (rowSame && colSame);
}


bool carClash(Location* move, Node* state) {
  for (int i = 0; i < state->numCars; i++) {
    if (moveInCar(move, state->cars[i])) {
      //printf("Car clash\n"); // debugging
      return true;
    }
  }
  return false;
}


bool moveInCar(Location* move, Car* car) {
  Location tile;
  for (int i = 0; i < car->size; i++) {
    tile.row = (car->vertical) ? car->start->row + i : car->start->row;
    tile.col = (car->vertical) ? car->start->col : car->start->col + i;
    //printf("Car #%c tile is R:%i C:%i\n", car->name, tile.row, tile.col); // debugging
    if (sameLocations(&tile, move)) {
      return true;
    }
  }
  return false;
}


void makeChild(Location* move, char carName, Node* state, Cp* cp) {
  Node* child = copyNodeNoChild(state);
  child->parent = state;
  (child->moves)++;
  moveCar(move, carName, child, cp);
  
  (state->numChildren)++;
  state->children = reallocateNodeArray(state->children, state->numChildren);
  state->children[state->numChildren - 1] = child;
}


Node** reallocateNodeArray(Node** array, int num) {
  array = (Node**)realloc(array, num * sizeof(Node*));
  if (!array) {
    throwError("ERROR: unable to reallocate\n");
  }
  return array;
}


Location** reallocateLocationArray(Location** array, int num) {
  array = (Location**)realloc(array, num * sizeof(Location*));
  if (!array) {
    throwError("ERROR: unable to reallocate\n");
  }
  return array;
}


void moveCar(Location* move, char carName, Node* state, Cp* cp) {
  for (int i = 0; i < state->numCars; i++) {
    if (state->cars[i]->name == carName) {
      implementMove(move, state->cars[i]);
      if (atEdge(state->cars[i], cp)) {
        state->cars[i]->escaped = true;
      } 
    }
  }
}


void implementMove(Location* move, Car* car) {
  if (car->vertical) {
    car->start->row = (move->row == car->start->row - 1) ? car->start->row - 1 : car->start->row + 1;
  } else {
    car->start->col = (move->col == car->start->col - 1) ? car->start->col - 1 : car->start->col + 1;
  }
}


bool atEdge(Car* car, Cp* cp) {
  bool atTop = (car->start->row == 0);
  bool atBottom = (car->start->row + car->size == cp->height);
  bool atLeft = (car->start->col == 0);
  bool atRight = (car->start->col + car->size == cp->width);
  return (atTop || atBottom || atLeft || atRight);
}


Node* copyNodeNoChild(Node* original) {
  Node* copy = (Node*)allocateSpace(1, sizeof(Node));
  copy->numCars = original->numCars;
  copy->cars = (Car**)allocateSpace(copy->numCars, sizeof(Car*));
  for (int i = 0; i < copy->numCars; i++) {
    copy->cars[i] = copyCar(original->cars[i]);
  }
  copy->moves = original->moves;
  copy->parent = original->parent;
  return copy;
}


Car* copyCar(Car* original) {
  Car* copy = allocateCar();
  if (original->escaped) {
    copy->escaped = original->escaped;
    return copy;
  }
  copy->start->row = original->start->row;
  copy->start->col = original->start->col;
  copy->name = original->name;
  copy->vertical = original->vertical;
  copy->size = original->size;
  copy->escaped = false;
  return copy;
}


void* allocateSpace(int num, int size) {
  void* pointer = calloc(num, size);
  if (!pointer) {
    throwError("ERROR: unable to allocate space\n");
  }
  return pointer;
}


Node* allocateNode(void) {
  Node* new = (Node*)allocateSpace(1, sizeof(Node));
  new->cars = (Car**)allocateSpace(1, sizeof(Car*));
  new->children = (Node**)allocateSpace(1, sizeof(Node*));
  return new;
}


void freeNode(Node* node) {
  for (int i = 0; i < node->numCars; i++) {
    freeCar(node->cars[i]);
  }
  for (int i = 0; i < node->numChildren; i++) {
    freeNode(node->children[i]);
  }
  free(node);
}


Car* allocateCar(void) {
  Car* new = (Car*)allocateSpace(1, sizeof(Car));
  new->start = (Location*)allocateSpace(1, sizeof(Location));
  return new;
}


void freeCar(Car* car) {
  free(car->start);
  free(car);
}


Tree* makeTree(Node* root, Cp* cp) {
  Tree* tree = (Tree*)allocateSpace(1, sizeof(Tree));
  tree->root = root;
  tree->carpark = cp;
  return tree;
}

Cp* allocateCarpark(void) {
  Cp* carpark = (Cp*)allocateSpace(1, sizeof(Cp));
  carpark->bollards = (Location**)allocateSpace(1, sizeof(Location*));
  return carpark;
}


void throwError(const char* message) {
  fputs(message, stderr);
  exit(EXIT_FAILURE);
}


void printState(Node* state, Cp* cp) {
  char* array = getPrintArray(state, cp);
  
  // Print array
  for (int row = 0; row < cp->height; row++) {
    for (int col = 0; col < cp->width; col++) {
      char tile = array[(row * cp->width) + col];
      printf("%c", tile);
    }
    printf("\n");
  }
  printf("\n");
  free(array);
}


char* getPrintArray(Node* state, Cp* cp) {
  if (!state) {
    throwError("ERROR: cannot print NULL Node\n");
  }
  char* array = (char*)allocateSpace((cp->width * cp->height), sizeof(char));
  for (int i = 0; i < cp->width * cp->height; i++) {
    array[i] = GAP;
  }
  for (int i = 0; i < cp->numBollards; i++) {
    int index = (cp->width * cp->bollards[i]->row) + cp->bollards[i]->col;
    array[index] = BOLLARD;
  }
  for (int i = 0; i < state->numCars; i++) {
    Car* current = state->cars[i];
    if (current) {
      addCarToArray(current, array, cp);
    }
  }
  return array;
}


void addCarToArray(Car* car, char* array, Cp* cp) {
  Location* start = car->start;
  for (int i = 0; i < car->size; i++) {
    Location carTile;
    carTile.row = (car->vertical) ? (start->row + i) : start->row;
    carTile.col = (car->vertical) ? start->col : (start->col + i);
    array[(carTile.row * cp->width) + carTile.col] = car->name;
  } 
}


void test(void) {
  assert(true);  
}
