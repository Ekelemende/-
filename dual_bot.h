#ifndef DUAL_BOT_H
#define DUAL_BOT_H

#include "game.h"
#include "graphics.h"
#include "records.h"

#define SEQUENTIAL_EXPLOSIONS 10
#define BOT_MOVE_DELAY 0.6

typedef struct {
    bool active;
    bool win;
    bool lose;
    bool explosionActive;
    bool simultaneousMode;
    int elapsedSeconds;
    double startTime;
    double lastMoveTime;
    Position* bombList;
    int bombCount;
    int currentExplosionIndex;
    int explosionFrame;
    Position explodedPosition;
} BotGameState;

void initBotGameState(BotGameState* state);
void cleanupBotGameState(BotGameState* state);
void collectMinesList(const GameBoard* board, Position** bombList, int* bombCount, Position explodedPos);
void updateExplosionAnimation(BotGameState* state, GameBoard* board);
bool processBotTurn(GameBoard* board, BotGameState* state, bool useHardLogic, double delay);
void runDualBotGame(Difficulty diff, HighScores* scores, int difficultyId);

#endif