#define _POSIX_C_SOURCE 199309L
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

static volatile int sink;

// xудший случай: все безопасные клетки открыты, все мины с флагами, кроме последней 
static void buildNearWin(GameBoard* b, int rows, int cols, int mines) {
    srand(12345);
    initGameBoard(b, rows, cols, mines, 0);
    placeMines(b, rows / 2, cols / 2);
    int lastMine = -1;
    for (int i = 0; i < rows * cols; i++) {
        Cell* c = &b->cells[i];
        if (c->isMine) { c->hasFlag = true; lastMine = i; }
        else { c->isRevealed = true; b->revealedCount++; }
    }
    b->cells[lastMine].hasFlag = false;
}

static void runCase(const char* name, int rows, int cols, int mines, int reps) {
    GameBoard b;
    buildNearWin(&b, rows, cols, mines);

    double t0 = time_us();
    for (int r = 0; r < reps; r++) sink = checkVictory(&b);
    double tVictory = (time_us() - t0) / reps;

    t0 = time_us();
    for (int r = 0; r < reps; r++) sink = hasRevealedMine(&b);
    double tMine = (time_us() - t0) / reps;

    t0 = time_us();
    for (int r = 0; r < reps; r++) sink = getSafeRevealedCount(&b);
    double tSafe = (time_us() - t0) / reps;

    printf("%-14s %4dx%-4d  checkVictory=%9.3f us  hasRevealedMine=%9.3f us  getSafeRevealedCount=%9.3f us\n",
           name, rows, cols, tVictory, tMine, tSafe);
    cleanupGameBoard(&b);
}

int main(void) {
    runCase("Beginner", 9, 9, 10, 200000);
    runCase("Intermediate", 16, 16, 40, 100000);
    runCase("Expert", 16, 30, 99, 100000);
    runCase("Большое", 100, 100,  1500, 5000);
    runCase("Огромное", 300, 300, 13500, 500);
    return 0;
}