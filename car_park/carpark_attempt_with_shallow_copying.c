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
#define LARGEARRAY 1000000


typedef struct {
  int width;
  int height;
  int moves;
  char board[MAXROW][MAXROW];
  int parents[SMALLARRAY];
} Cp;

typedef struct {
  int nextCp;
  int endArray;
  int endTried;
} State;

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


void checkInputs(int argc, char* argv[]);
bool flagNumIsCorrect(int argc, char* argv[]);
char* getFilename(int argc, char* argv[]);
void solveCarpark(char* fileName);
Cp populateCarpark(FILE* fp);
bool isValidTile(char tile);
void isValidCp(Cp start);
int findSolution(Cp cpArray[], Cp tried[], State* state);
bool isComplete(Cp cp);
void makeNextStates(Cp current, Cp cpArray[], Cp tried[], State* state);
int getCars(Cp current, Car cars[]);
int updateCars(int row, int col, char tile, Car cars[], int numCars);
void updateCar(Car* car, int row, int col);
void addCar(int row, int col, char tile, Car cars[], int numCars);
void moveCar(Car car, Cp carpark, Cp cpArray[], Cp tried[], State* state);
void getMoves(Car car, Location* moveBack, Location* moveForwards);
void tryMove(Location move, Car car, Cp carpark, Cp cpArray[], Cp tried[], State* state);
bool movePossible(Location move, Cp carpark);
bool atEdge(Location move, Cp carpark);
void removeCar(Car car, Cp carpark, Cp cpArray[], Cp tried[], State* state);
void makeMove(Location move, Car car, Cp carpark, Cp cpArray[], Cp tried[], State* state);
void getNewCarTiles(Location newCarTiles[], Location move, Car car);
Location getNewGap(Location move, Car car);
void addCarpark(Cp carpark, Cp cpArray[], Cp tried[], State* state);
bool carparksAreSame(Cp carpark1, Cp carpark2);
void handleResult(int moves);
void throwError(char* errorMessage);
void printCarpark(Cp carpark);
void printPath(Cp carpark, Cp cpArray[]);
void test(void);


int main(int argc, char* argv[]) {
  test();
  checkInputs(argc, argv);
  char* fileName = getFilename(argc, argv);
  
  solveCarpark(fileName);

  return 0;
}


void checkInputs(int argc, char* argv[]) {
  if (argc < 2) {
    throwError("ERROR: correct usage = './carpark <carpark file> <flags (optional)>'\n");
  }

  if (!flagNumIsCorrect(argc, argv)) {
    throwError("ERROR: correct usage = './carpark <carpark file> <flags (optional)>'\n");
  }
}


bool flagNumIsCorrect(int argc, char* argv[]) {
  int numNonFlags = 0;
  for (int i = 1; i < argc; i++) {
    char* arg = argv[i];
    if (arg[0] != '-') {
      numNonFlags++;
    }
  }
  return (numNonFlags == 1); 
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


void solveCarpark(char* fileName) {
  FILE* fp = fopen(fileName, "r");
  static Cp cpArray[LARGEARRAY];
  static Cp tried[LARGEARRAY];
  cpArray[0] = tried[0] = populateCarpark(fp);
  static State state = {0, 1, 1};
  fclose(fp);
  
  int moves = findSolution(cpArray, tried, &state);
  handleResult(moves);
}


Cp populateCarpark(FILE* fp) {
  Cp start;
  start.moves = 0;
  char line[MAXSTR];
  fgets(line, MAXSTR, fp);
  sscanf(line, "%ix%i", &(start.height), &(start.width));
  printf("Height = %i, width = %i\n", start.height, start.width); // debugging

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
  
  isValidCp(start);
  
  return start;
}


bool isValidTile(char tile) {
  bool isGap = (tile == GAP);
  bool isBol = (tile == BOLLARD);
  bool isCar = ((tile >= 'A') && (tile <= 'Z'));
  return ((isGap) || (isBol) || (isCar));
}


void isValidCp(Cp start) {
  int numCars = 0;
  Car cars[MAXCARS];
  for (int i = 0; i < MAXCARS; i++) {
    cars[i].name = '\0'; 
  }
  // getCars() throws errors if there are non-contiguous/straight cars
  numCars = getCars(start, cars); 
  
  // Therefore, just need to check cars are of size > 1
  for (int i = 0; i < numCars; i++) {
    Car ithCar = cars[i];
    if (ithCar.size < 2) {
      throwError("ERROR: invalid carpark\n");
    }
  }
}


int findSolution(Cp cpArray[], Cp tried[], State* state) {
  while (state->nextCp < state->endArray) {
    Cp current = cpArray[state->nextCp];
    if (isComplete(current)) {
      return current.moves;
    }
    (state->nextCp)++;
    
    printPath(current, cpArray);
    makeNextStates(current, cpArray, tried, state);
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


void makeNextStates(Cp current, Cp cpArray[], Cp tried[], State* state) {
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


int getCars(Cp current, Car cars[]) {
  int numCars = 0;
  for (int row = 0; row < current.height; row++) {
    for (int col = 0; col < current.width; col++) {
      char tile = current.board[row][col];
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
      throwError("ERROR: invalid car position (size >\n");
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


void moveCar(Car car, Cp carpark, Cp cpArray[], Cp tried[], State* state) {
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


void tryMove(Location move, Car car, Cp carpark, Cp cpArray[], Cp tried[], State* state) {
  if (movePossible(move, carpark)) {
    if (atEdge(move, carpark)) {
      removeCar(car, carpark, cpArray, tried, state);
    } else {
      makeMove(move, car, carpark, cpArray, tried, state);
    }
  }
}


bool movePossible(Location move, Cp carpark) {
  return (carpark.board[move.row][move.col] == GAP);
}


bool atEdge(Location move, Cp carpark) {
  bool atTop = (move.row == 0);
  bool atLeft = (move.col == 0);
  bool atRight = (move.col == carpark.width - 1);
  bool atBottom = (move.row == carpark.height - 1);
  return ((atTop) || (atLeft) || (atRight) || (atBottom));
}


void removeCar(Car car, Cp carpark, Cp cpArray[], Cp tried[], State* state) {
  for (int row = 0; row < carpark.height; row++) {
    for (int col = 0; col < carpark.width; col++) {
      if (carpark.board[row][col] == car.name) {
        carpark.board[row][col] = GAP;
      }
    }
  }
  carpark.parents[carpark.moves] = state->nextCp;
  (carpark.moves)++;
  
  // Insert new carpark
  addCarpark(carpark, cpArray, tried, state);
}


void makeMove(Location move, Car car, Cp carpark, Cp cpArray[], Cp tried[], State* state) {
  // Get tiles to update
  Location newCarTiles[MAXROW];
  getNewCarTiles(newCarTiles, move, car);
  Location newGap = getNewGap(move, car);
  
  // updateTiles
  for (int i = 0; i < car.size; i++) {
    carpark.board[newCarTiles[i].row][newCarTiles[i].col] = car.name;
  }
  carpark.board[newGap.row][newGap.col] = GAP; 
  carpark.parents[carpark.moves] = state->nextCp;
  (carpark.moves)++;
  
  // Insert new carpark
  addCarpark(carpark, cpArray, tried, state);
}


void getNewCarTiles(Location newCarTiles[], Location move, Car car) {
  for (int i = 0; i < car.size; i++) {
    if (car.vertical) {
      newCarTiles[i].row = (move.row == car.startRow - 1) ? ((car.startRow - 1) + i) : ((car.startRow + 1) + i);
      newCarTiles[i].col = car.startCol;      
    } else {
      newCarTiles[i].row = car.startRow;
      newCarTiles[i].col = (move.col = car.startCol - 1) ? ((car.startCol - 1) + i) : ((car.startCol + 1) + i);
    }
  }
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
    cpArray[state->endArray] = tried[state->endTried] = carpark;
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
    for (int i = 0; i < (carpark.width - 1) / 2 ; i++) {
      printf(" ");
    }
    printf("\u2193\n");
  }
  printCarpark(carpark);
  printf("\n");
  printf("\n");
}


void test(void) {
  assert(true);
}
