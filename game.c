#include "game.h"
#include <stdlib.h>
#include <string.h>

void initCell(Cell* cell) {
    if (!cell) return;
    cell->isRevealed = false;
    cell->hasFlag = false;
    cell->hasExploded = false;
    cell->isAnimating = false;
    cell->mineCount = 0;
    cell->isMine = false;
    cell->special = '#';
}

Cell* getCell(const GameBoard* board, int row, int col) {
    if (!board || !board->cells) return NULL;
    if (row < 0 || row >= board->rows || col < 0 || col >= board->cols) return NULL;
    return &board->cells[row * board->cols + col];
}

bool initGameBoard(GameBoard* board, int rows, int cols, int mines, int difficulty) {
    if (!board) return false;
    
    board->rows = rows;
    board->cols = cols;
    board->totalMines = mines;
    board->flagsPlaced = 0;
    board->revealedCount = 0;
    board->firstMove = true;
    board->difficulty = difficulty;
    
    board->cells = (Cell*)calloc(rows * cols, sizeof(Cell));
    if (!board->cells) return false;
    
    for (int i = 0; i < rows * cols; i++) {
        initCell(&board->cells[i]);
    }
    return true;
}

void cleanupGameBoard(GameBoard* board) {
    if (board && board->cells) {
        free(board->cells);
        board->cells = NULL;
    }
}

void placeMines(GameBoard* board, int firstRow, int firstCol) {
    if (!board || !board->cells) return;
    
    int totalCells = board->rows * board->cols;
    
    for (int i = 0; i < totalCells; i++) {
        board->cells[i].mineCount = 0;
        board->cells[i].isMine = false;
    }
    
    int minesPlaced = 0;
    while (minesPlaced < board->totalMines) {
        int row = rand() % board->rows;
        int col = rand() % board->cols;
        
        if (row == firstRow && col == firstCol) continue;
        
        Cell* cell = getCell(board, row, col);
        if (cell->isMine) continue;
        
        cell->isMine = true;
        minesPlaced++;
    }
    
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j < board->cols; j++) {
            Cell* cell = getCell(board, i, j);
            if (cell->isMine) continue;
            
            int cnt = 0;
            for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                    if (di == 0 && dj == 0) continue;
                    Cell* nb = getCell(board, i + di, j + dj);
                    if (nb && nb->isMine) cnt++;
                }
            }
            cell->mineCount = cnt;
        }
    }
}

void revealCell(GameBoard* board, int startRow, int startCol) {
    if (!board || !board->cells) return;
    
    Cell* startCell = getCell(board, startRow, startCol);
    if (!startCell || startCell->isRevealed || startCell->hasFlag) return;
    
    if (startCell->isMine) {
        startCell->isRevealed = true;
        return;
    }
    
    int maxSize = board->rows * board->cols;
    Position* stack = (Position*)malloc(maxSize * sizeof(Position));
    if (!stack) return;
    
    int top = 0;
    stack[top].row = startRow;
    stack[top].col = startCol;
    top++;
    
    while (top > 0) {
        top--;
        int r = stack[top].row;
        int c = stack[top].col;
        
        Cell* cell = getCell(board, r, c);
        if (!cell || cell->isRevealed || cell->hasFlag || cell->isMine) continue;
        
        cell->isRevealed = true;
        board->revealedCount++;
        
        if (cell->mineCount == 0) {
            for (int i = r - 1; i <= r + 1; i++) {
                for (int j = c - 1; j <= c + 1; j++) {
                    if (i < 0 || i >= board->rows || j < 0 || j >= board->cols) continue;
                    
                    Cell* nb = getCell(board, i, j);
                    if (nb && !nb->isRevealed && !nb->hasFlag && !nb->isMine) {
                        bool already = false;
                        for (int k = 0; k < top; k++) {
                            if (stack[k].row == i && stack[k].col == j) {
                                already = true;
                                break;
                            }
                        }
                        if (!already) {
                            stack[top].row = i;
                            stack[top].col = j;
                            top++;
                        }
                    }
                }
            }
        }
    }
    
    free(stack);
}

bool checkVictory(const GameBoard* board) {
    if (!board || !board->cells) return false;
    
    int totalCells = board->rows * board->cols;
    for (int i = 0; i < totalCells; i++) {
        const Cell* c = &board->cells[i];
        if (c->isMine && !c->hasFlag) return false;
        if (!c->isMine && !c->isRevealed) return false;
    }
    return true;
}

int countFlagsAround(const GameBoard* board, int row, int col) {
    if (!board) return 0;
    
    int cnt = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            const Cell* c = getCell(board, row + dr, col + dc);
            if (c && c->hasFlag) cnt++;
        }
    }
    return cnt;
}

int countUnexposedAround(const GameBoard* board, int row, int col) {
    if (!board) return 0;
    
    int cnt = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            const Cell* c = getCell(board, row + dr, col + dc);
            if (c && !c->isRevealed && !c->hasFlag) cnt++;
        }
    }
    return cnt;
}

void getNeighborsUnexposed(const GameBoard* board, int row, int col, Position neighbors[], int* outCount) {
    if (!board || !neighbors || !outCount) return;
    
    *outCount = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int r = row + dr, c = col + dc;
            const Cell* cell = getCell(board, r, c);
            if (cell && !cell->isRevealed && !cell->hasFlag) {
                neighbors[*outCount].row = r;
                neighbors[*outCount].col = c;
                (*outCount)++;
            }
        }
    }
}

bool hasRevealedMine(const GameBoard* board) {
    if (!board || !board->cells) return false;
    
    int totalCells = board->rows * board->cols;
    for (int i = 0; i < totalCells; i++) {
        if (board->cells[i].isMine && board->cells[i].isRevealed) {
            return true;
        }
    }
    return false;
}

int getSafeRevealedCount(const GameBoard* board) {
    if (!board || !board->cells) return 0;
    
    int cnt = 0;
    int totalCells = board->rows * board->cols;
    for (int i = 0; i < totalCells; i++) {
        if (board->cells[i].isRevealed && !board->cells[i].isMine) {
            cnt++;
        }
    }
    return cnt;
}

void cloneBoardForDual(const GameBoard* src, GameBoard* dst) {
    if (!src || !dst) return;
    
    dst->rows = src->rows;
    dst->cols = src->cols;
    dst->totalMines = src->totalMines;
    dst->flagsPlaced = 0;
    dst->revealedCount = 0;
    dst->firstMove = false;
    dst->difficulty = src->difficulty;
    
    int totalCells = src->rows * src->cols;
    dst->cells = (Cell*)calloc(totalCells, sizeof(Cell));
    if (!dst->cells) return;
    
    for (int i = 0; i < totalCells; i++) {
        dst->cells[i].isMine = src->cells[i].isMine;
        dst->cells[i].mineCount = src->cells[i].mineCount;
        dst->cells[i].isRevealed = false;
        dst->cells[i].hasFlag = false;
        dst->cells[i].hasExploded = false;
        dst->cells[i].isAnimating = false;
        dst->cells[i].special = '#';
    }
}