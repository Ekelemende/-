#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#define MAX_ROWS 30
#define MAX_COLS 30
#define MAX_UNKNOWN_CELLS (MAX_ROWS * MAX_COLS)
#define MAX_CONSTRAINTS (MAX_ROWS * MAX_COLS * 8)
#define MAX_COMPONENT_SIZE 24

typedef enum {
    MODE_HUMAN,
    MODE_BOT_SIMPLE,
    MODE_BOT_HARD,
    MODE_DUAL_BOT
} GameMode;

typedef struct {
    bool isRevealed;
    bool hasFlag;
    bool hasExploded;
    bool isAnimating;
    int mineCount;
    bool isMine;
    char special;
} Cell;

typedef struct {
    Cell* cells;
    int rows;
    int cols;
    int totalMines;
    int flagsPlaced;
    int revealedCount;
    bool firstMove;
    int difficulty;
} GameBoard;

typedef struct {
    int row, col;
} Position;

void initCell(Cell* cell);
Cell* getCell(const GameBoard* board, int row, int col);
bool initGameBoard(GameBoard* board, int rows, int cols, int mines, int difficulty);
void cleanupGameBoard(GameBoard* board);
void placeMines(GameBoard* board, int firstRow, int firstCol);
void revealCell(GameBoard* board, int startRow, int startCol);
bool checkVictory(const GameBoard* board);
int countFlagsAround(const GameBoard* board, int row, int col);
int countUnexposedAround(const GameBoard* board, int row, int col);
void getNeighborsUnexposed(const GameBoard* board, int row, int col, Position neighbors[], int* outCount);
bool hasRevealedMine(const GameBoard* board);
int getSafeRevealedCount(const GameBoard* board);
void cloneBoardForDual(const GameBoard* src, GameBoard* dst);

#endif // GAME_H