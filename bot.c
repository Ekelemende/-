#include "bot.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "game.h"

#define MAX_CONSTRAINTS (MAX_ROWS * MAX_COLS * 8)
#define MAX_UNKNOWN_CELLS (MAX_ROWS * MAX_COLS)
#define MAX_COMPONENT_SIZE 24

typedef struct {
    int row, col;
    int index;
} UnknownCellInfo;

typedef struct {
    int neighborIndices[8];
    int neighborCount;
    int requiredMines;
} Constraint;

typedef struct {
    int* mineCounts;
    int totalValid;
} BruteResult;

typedef struct {
    bool foundGuaranteed;
    int bestRow;
    int bestCol;
    double minProb;
    bool isFlag;
} ComponentResult;

bool canMakeUnsafeRandomMove(const GameBoard* board) {
    if (!board) return false;
    
    int totalCells = board->rows * board->cols;
    int safeRevealed = 0;
    
    for (int i = 0; i < totalCells; i++) {
        if (board->cells[i].isRevealed && !board->cells[i].isMine) {
            safeRevealed++;
        }
    }
    
    return ((double)safeRevealed / totalCells) > 0.35;
}

bool executeRandomMove(GameBoard* board, Move moves[], int moveCount) {
    if (moveCount == 0) return false;
    
    Move m = moves[rand() % moveCount];
    
    if (m.isFlag) {
        Cell* cell = getCell(board, m.row, m.col);
        if (cell && !cell->hasFlag) {
            cell->hasFlag = true;
            board->flagsPlaced++;
        }
    } else {
        revealCell(board, m.row, m.col);
    }
    return true;
}

int collectUnknownAndConstraints(const GameBoard* board, UnknownCellInfo unknown[],
                                 Constraint constraints[], int* constrCount, int indexMap[MAX_ROWS][MAX_COLS]) {
    if (!board || !unknown || !constraints || !constrCount) return 0;
    
    int unknownCount = 0;
    *constrCount = 0;
    
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j < board->cols; j++) {
            indexMap[i][j] = -1;
        }
    }
    
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j < board->cols; j++) {
            const Cell* cell = getCell(board, i, j);
            if (!cell->isRevealed || cell->mineCount == 0) continue;
            
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    if (dr == 0 && dc == 0) continue;
                    int r = i + dr, c = j + dc;
                    if (r < 0 || r >= board->rows || c < 0 || c >= board->cols) continue;
                    
                    const Cell* nb = getCell(board, r, c);
                    if (nb && !nb->isRevealed && !nb->hasFlag && indexMap[r][c] == -1 && unknownCount < MAX_UNKNOWN_CELLS) {
                        indexMap[r][c] = unknownCount;
                        unknown[unknownCount].row = r;
                        unknown[unknownCount].col = c;
                        unknown[unknownCount].index = unknownCount;
                        unknownCount++;
                    }
                }
            }
        }
    }
    
    if (unknownCount == 0) return 0;
    
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j < board->cols; j++) {
            const Cell* cell = getCell(board, i, j);
            if (!cell->isRevealed || cell->mineCount == 0) continue;
            
            int flags = countFlagsAround(board, i, j);
            int required = cell->mineCount - flags;
            if (required < 0) required = 0;
            
            int neighborIdx[8];
            int nbCount = 0;
            
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    if (dr == 0 && dc == 0) continue;
                    int r = i + dr, c = j + dc;
                    if (r < 0 || r >= board->rows || c < 0 || c >= board->cols) continue;
                    
                    const Cell* nb = getCell(board, r, c);
                    if (nb && !nb->isRevealed && !nb->hasFlag) {
                        int idx = indexMap[r][c];
                        if (idx != -1 && idx < unknownCount) {
                            neighborIdx[nbCount++] = idx;
                        }
                    }
                }
            }
            
            if (nbCount == 0 || required > nbCount) continue;
            
            if (*constrCount < MAX_CONSTRAINTS) {
                Constraint* cstr = &constraints[*constrCount];
                cstr->neighborCount = nbCount;
                cstr->requiredMines = required;
                memcpy(cstr->neighborIndices, neighborIdx, nbCount * sizeof(int));
                (*constrCount)++;
            }
        }
    }
    
    return unknownCount;
}

void bruteForceRec(int idx, int currentMines[], int unknownCount, const Constraint constraints[], 
                   int constrCount, const int localToGlobal[], BruteResult* result) {
    if (idx == unknownCount) {
        for (int c = 0; c < constrCount; c++) {
            int cnt = 0;
            for (int k = 0; k < constraints[c].neighborCount; k++) {
                int globalIdx = localToGlobal[constraints[c].neighborIndices[k]];
                if (globalIdx >= 0 && currentMines[globalIdx]) cnt++;
            }
            if (cnt != constraints[c].requiredMines) return;
        }
        
        result->totalValid++;
        for (int i = 0; i < unknownCount; i++) {
            if (currentMines[i]) result->mineCounts[i]++;
        }
        return;
    }
    
    currentMines[idx] = 0;
    bruteForceRec(idx + 1, currentMines, unknownCount, constraints, constrCount, localToGlobal, result);
    currentMines[idx] = 1;
    bruteForceRec(idx + 1, currentMines, unknownCount, constraints, constrCount, localToGlobal, result);
}

void splitIntoComponents(int unknownCount, const Constraint constraints[], int constrCount,
                         int componentId[], int* numComponents) {
    if (unknownCount == 0) {
        *numComponents = 0;
        return;
    }
    
    for (int i = 0; i < unknownCount; i++) componentId[i] = i;
    *numComponents = unknownCount;
    
    for (int c = 0; c < constrCount; c++) {
        int first = -1;
        for (int k = 0; k < constraints[c].neighborCount; k++) {
            int idx = constraints[c].neighborIndices[k];
            if (idx >= 0 && idx < unknownCount) {
                if (first == -1) {
                    first = idx;
                } else {
                    int oldId = componentId[idx];
                    int newId = componentId[first];
                    if (oldId != newId) {
                        for (int i = 0; i < unknownCount; i++) {
                            if (componentId[i] == oldId) componentId[i] = newId;
                        }
                        (*numComponents)--;
                    }
                }
            }
        }
    }
    
    int newId = 0;
    int mapping[MAX_UNKNOWN_CELLS];
    for (int i = 0; i < MAX_UNKNOWN_CELLS; i++) mapping[i] = -1;
    
    for (int i = 0; i < unknownCount; i++) {
        if (mapping[componentId[i]] == -1) {
            mapping[componentId[i]] = newId++;
        }
        componentId[i] = mapping[componentId[i]];
    }
    *numComponents = newId;
}

ComponentResult analyzeComponent(const UnknownCellInfo unknown[], int unknownCount, 
                                 const Constraint constraints[], int constrCount, 
                                 const int localToGlobal[]) {
    ComponentResult res = {false, -1, -1, 1.1, false};
    
    if (unknownCount > MAX_COMPONENT_SIZE || unknownCount == 0) return res;
    
    BruteResult result;
    result.mineCounts = (int*)calloc(unknownCount, sizeof(int));
    result.totalValid = 0;
    
    if (!result.mineCounts) return res;
    
    int* currentMines = (int*)calloc(unknownCount, sizeof(int));
    if (!currentMines) {
        free(result.mineCounts);
        return res;
    }
    
    bruteForceRec(0, currentMines, unknownCount, constraints, constrCount, localToGlobal, &result);
    free(currentMines);
    
    if (result.totalValid == 0) {
        free(result.mineCounts);
        return res;
    }
    
    double minProb = 1.1;
    int bestLocalIdx = -1;
    
    for (int i = 0; i < unknownCount; i++) {
        double prob = (double)result.mineCounts[i] / result.totalValid;
        
        if (prob == 0.0) {
            int globalIdx = localToGlobal[i];
            if (globalIdx >= 0 && globalIdx < unknownCount) {
                res.foundGuaranteed = true;
                res.bestRow = unknown[globalIdx].row;
                res.bestCol = unknown[globalIdx].col;
                res.minProb = 0.0;
                res.isFlag = false;
                free(result.mineCounts);
                return res;
            }
        }
        
        if (prob == 1.0) {
            int globalIdx = localToGlobal[i];
            if (globalIdx >= 0 && globalIdx < unknownCount) {
                res.foundGuaranteed = true;
                res.bestRow = unknown[globalIdx].row;
                res.bestCol = unknown[globalIdx].col;
                res.minProb = 1.0;
                res.isFlag = true;
                free(result.mineCounts);
                return res;
            }
        }
        
        if (prob < minProb) {
            minProb = prob;
            bestLocalIdx = i;
        }
    }
    
    if (bestLocalIdx != -1) {
        int globalIdx = localToGlobal[bestLocalIdx];
        if (globalIdx >= 0 && globalIdx < unknownCount) {
            res.bestRow = unknown[globalIdx].row;
            res.bestCol = unknown[globalIdx].col;
            res.minProb = minProb;
            res.isFlag = false;
        }
    }
    
    free(result.mineCounts);
    return res;
}

bool solveByBruteForce(GameBoard* board) {
    if (!board || !board->cells) return false;
    
    int totalUnknown = 0;
    int totalCells = board->rows * board->cols;
    for (int i = 0; i < totalCells; i++) {
        if (!board->cells[i].isRevealed && !board->cells[i].hasFlag) {
            totalUnknown++;
        }
    }
    
    if (totalUnknown > 60) return false;
    
    UnknownCellInfo* unknown = (UnknownCellInfo*)malloc(MAX_UNKNOWN_CELLS * sizeof(UnknownCellInfo));
    Constraint* constraints = (Constraint*)malloc(MAX_CONSTRAINTS * sizeof(Constraint));
    
    if (!unknown || !constraints) {
        free(unknown);
        free(constraints);
        return false;
    }
    
    int indexMap[MAX_ROWS][MAX_COLS];
    int constrCount = 0;
    int unknownCount = collectUnknownAndConstraints(board, unknown, constraints, &constrCount, indexMap);
    
    if (unknownCount <= 0 || unknownCount > 30) {
        free(unknown);
        free(constraints);
        return false;
    }
    
    int* componentId = (int*)malloc(unknownCount * sizeof(int));
    if (!componentId) {
        free(unknown);
        free(constraints);
        return false;
    }
    
    int numComponents = 0;
    splitIntoComponents(unknownCount, constraints, constrCount, componentId, &numComponents);
    
    double globalMinProb = 1.1;
    int globalBestRow = -1, globalBestCol = -1;
    bool foundAnyMove = false;
    
    for (int comp = 0; comp < numComponents; comp++) {
        int compUnknownCount = 0;
        int compIndices[MAX_UNKNOWN_CELLS];
        int globalToLocal[MAX_UNKNOWN_CELLS];
        
        for (int i = 0; i < MAX_UNKNOWN_CELLS; i++) globalToLocal[i] = -1;
        
        for (int i = 0; i < unknownCount; i++) {
            if (componentId[i] == comp && compUnknownCount < MAX_UNKNOWN_CELLS) {
                globalToLocal[i] = compUnknownCount;
                compIndices[compUnknownCount++] = i;
            }
        }
        
        if (compUnknownCount == 0 || compUnknownCount > MAX_COMPONENT_SIZE) continue;
        
        Constraint compConstraints[MAX_CONSTRAINTS];
        int compConstrCount = 0;
        
        for (int c = 0; c < constrCount; c++) {
            bool allInComp = true;
            for (int k = 0; k < constraints[c].neighborCount; k++) {
                int idx = constraints[c].neighborIndices[k];
                if (idx < 0 || idx >= unknownCount || componentId[idx] != comp) {
                    allInComp = false;
                    break;
                }
            }
            if (!allInComp) continue;
            
            compConstraints[compConstrCount].neighborCount = constraints[c].neighborCount;
            compConstraints[compConstrCount].requiredMines = constraints[c].requiredMines;
            for (int k = 0; k < constraints[c].neighborCount; k++) {
                compConstraints[compConstrCount].neighborIndices[k] = globalToLocal[constraints[c].neighborIndices[k]];
            }
            compConstrCount++;
        }
        
        int* localToGlobal = (int*)malloc(compUnknownCount * sizeof(int));
        if (!localToGlobal) continue;
        
        for (int i = 0; i < compUnknownCount; i++) {
            localToGlobal[i] = compIndices[i];
        }
        
        ComponentResult res = analyzeComponent(unknown, compUnknownCount, compConstraints, compConstrCount, localToGlobal);
        free(localToGlobal);
        
        if (res.foundGuaranteed && res.bestRow >= 0 && res.bestRow < board->rows && 
            res.bestCol >= 0 && res.bestCol < board->cols) {
            if (res.isFlag) {
                Cell* cell = getCell(board, res.bestRow, res.bestCol);
                if (cell && !cell->hasFlag) {
                    cell->hasFlag = true;
                    board->flagsPlaced++;
                }
            } else {
                revealCell(board, res.bestRow, res.bestCol);
            }
            free(unknown);
            free(constraints);
            free(componentId);
            return true;
        }
        
        if (res.bestRow != -1 && res.minProb < globalMinProb) {
            globalMinProb = res.minProb;
            globalBestRow = res.bestRow;
            globalBestCol = res.bestCol;
            foundAnyMove = true;
        }
    }
    
    if (foundAnyMove && globalBestRow >= 0 && globalBestRow < board->rows && 
        globalBestCol >= 0 && globalBestCol < board->cols) {
        revealCell(board, globalBestRow, globalBestCol);
        free(unknown);
        free(constraints);
        free(componentId);
        return true;
    }
    
    free(unknown);
    free(constraints);
    free(componentId);
    return false;
}

bool botMakeMoveSimple(GameBoard* board) {
    if (!board || !board->cells) return false;
    
    if (board->firstMove) {
        int row = rand() % board->rows;
        int col = rand() % board->cols;
        placeMines(board, row, col);
        board->firstMove = false;
        revealCell(board, row, col);
        return true;
    }
    
    if (checkVictory(board)) return true;
    
    Move possibleMoves[MAX_ROWS * MAX_COLS];
    int moveCount = 0;
    
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j < board->cols; j++) {
            const Cell* cell = getCell(board, i, j);
            if (!cell->isRevealed || cell->mineCount == 0) continue;
            
            int flags = countFlagsAround(board, i, j);
            int unexposed = countUnexposedAround(board, i, j);
            
            if (cell->mineCount == flags && unexposed > 0) {
                Position neighbors[8];
                int nbCnt;
                getNeighborsUnexposed(board, i, j, neighbors, &nbCnt);
                for (int k = 0; k < nbCnt; k++) {
                    Cell* nb = getCell(board, neighbors[k].row, neighbors[k].col);
                    if (nb && !nb->isRevealed && !nb->hasFlag) {
                        possibleMoves[moveCount].row = neighbors[k].row;
                        possibleMoves[moveCount].col = neighbors[k].col;
                        possibleMoves[moveCount].isFlag = false;
                        moveCount++;
                    }
                }
            }
            else if (cell->mineCount - flags == unexposed && unexposed > 0) {
                Position neighbors[8];
                int nbCnt;
                getNeighborsUnexposed(board, i, j, neighbors, &nbCnt);
                for (int k = 0; k < nbCnt; k++) {
                    Cell* nb = getCell(board, neighbors[k].row, neighbors[k].col);
                    if (nb && !nb->isRevealed && !nb->hasFlag) {
                        possibleMoves[moveCount].row = neighbors[k].row;
                        possibleMoves[moveCount].col = neighbors[k].col;
                        possibleMoves[moveCount].isFlag = true;
                        moveCount++;
                    }
                }
            }
        }
    }
    
    if (moveCount > 0) {
        return executeRandomMove(board, possibleMoves, moveCount);
    }
    
    bool canRisk = canMakeUnsafeRandomMove(board);
    
    Position closedCells[MAX_ROWS * MAX_COLS];
    int closedCount = 0;
    
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j < board->cols; j++) {
            const Cell* c = getCell(board, i, j);
            if (!c->isRevealed && !c->hasFlag) {
                closedCells[closedCount].row = i;
                closedCells[closedCount].col = j;
                closedCount++;
            }
        }
    }
    
    if (closedCount == 0) return false;
    
    if (!canRisk) {
        Position safeCells[MAX_ROWS * MAX_COLS];
        int safeCount = 0;
        
        for (int i = 0; i < closedCount; i++) {
            Cell* c = getCell(board, closedCells[i].row, closedCells[i].col);
            if (c && !c->isMine) {
                safeCells[safeCount++] = closedCells[i];
            }
        }
        
        if (safeCount > 0) {
            int target = rand() % safeCount;
            revealCell(board, safeCells[target].row, safeCells[target].col);
            return true;
        }
    } else {
        int target = rand() % closedCount;
        revealCell(board, closedCells[target].row, closedCells[target].col);
        return true;
    }
    
    return false;
}

bool botMakeMoveHard(GameBoard* board) {
    if (!board || !board->cells) return false;
    
    if (board->firstMove) {
        int row = rand() % board->rows;
        int col = rand() % board->cols;
        placeMines(board, row, col);
        board->firstMove = false;
        revealCell(board, row, col);
        return true;
    }
    
    if (checkVictory(board)) return true;
    
    Move possibleMoves[MAX_ROWS * MAX_COLS];
    int moveCount = 0;
    
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j < board->cols; j++) {
            const Cell* cell = getCell(board, i, j);
            if (!cell->isRevealed || cell->mineCount == 0) continue;
            
            int flags = countFlagsAround(board, i, j);
            int unexposed = countUnexposedAround(board, i, j);
            
            if (cell->mineCount == flags && unexposed > 0) {
                Position neighbors[8];
                int nbCnt;
                getNeighborsUnexposed(board, i, j, neighbors, &nbCnt);
                for (int k = 0; k < nbCnt; k++) {
                    Cell* nb = getCell(board, neighbors[k].row, neighbors[k].col);
                    if (nb && !nb->isRevealed && !nb->hasFlag) {
                        possibleMoves[moveCount].row = neighbors[k].row;
                        possibleMoves[moveCount].col = neighbors[k].col;
                        possibleMoves[moveCount].isFlag = false;
                        moveCount++;
                    }
                }
            }
            else if (cell->mineCount - flags == unexposed && unexposed > 0) {
                Position neighbors[8];
                int nbCnt;
                getNeighborsUnexposed(board, i, j, neighbors, &nbCnt);
                for (int k = 0; k < nbCnt; k++) {
                    Cell* nb = getCell(board, neighbors[k].row, neighbors[k].col);
                    if (nb && !nb->isRevealed && !nb->hasFlag) {
                        possibleMoves[moveCount].row = neighbors[k].row;
                        possibleMoves[moveCount].col = neighbors[k].col;
                        possibleMoves[moveCount].isFlag = true;
                        moveCount++;
                    }
                }
            }
        }
    }
    
    if (moveCount > 0) {
        return executeRandomMove(board, possibleMoves, moveCount);
    }
    
    if (solveByBruteForce(board)) return true;
    
    bool canRisk = canMakeUnsafeRandomMove(board);
    
    Position closedCells[MAX_ROWS * MAX_COLS];
    int closedCount = 0;
    
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j < board->cols; j++) {
            const Cell* c = getCell(board, i, j);
            if (!c->isRevealed && !c->hasFlag) {
                closedCells[closedCount].row = i;
                closedCells[closedCount].col = j;
                closedCount++;
            }
        }
    }
    
    if (closedCount == 0) return false;
    
    if (!canRisk) {
        Position safeCells[MAX_ROWS * MAX_COLS];
        int safeCount = 0;
        
        for (int i = 0; i < closedCount; i++) {
            Cell* c = getCell(board, closedCells[i].row, closedCells[i].col);
            if (c && !c->isMine) {
                safeCells[safeCount++] = closedCells[i];
            }
        }
        
        if (safeCount > 0) {
            int target = rand() % safeCount;
            revealCell(board, safeCells[target].row, safeCells[target].col);
            return true;
        }
    } else {
        int target = rand() % closedCount;
        revealCell(board, closedCells[target].row, closedCells[target].col);
        return true;
    }
    
    return false;
}