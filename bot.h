#ifndef BOT_H
#define BOT_H

#include "game.h"

typedef struct {
    int row;
    int col;
    bool isFlag;
} Move;

bool botMakeMoveSimple(GameBoard* board);
bool botMakeMoveHard(GameBoard* board);
bool canMakeUnsafeRandomMove(const GameBoard* board);
bool solveByBruteForce(GameBoard* board);

#endif // BOT_H