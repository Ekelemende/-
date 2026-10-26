#define _POSIX_C_SOURCE 199309L
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static void resetBoard(GameBoard* b) {
    for (int i = 0; i < b->rows * b->cols; i++) {
        b->cells[i].isRevealed = false;
        b->cells[i].hasFlag = false;
    }
    b->revealedCount = 0;
}

static void runCase(const char* name, int rows, int cols, int mines, int reps) {
    srand(12345);                        
    GameBoard b;
    initGameBoard(&b, rows, cols, mines, 0);
    placeMines(&b, rows / 2, cols / 2);

    double total = 0;
    int revealed = 0;
    for (int r = 0; r < reps; r++) {
        resetBoard(&b);
        double t0 = time_ms();
        revealCell(&b, rows / 2, cols / 2);
        total += time_ms() - t0;
        revealed = b.revealedCount;
    }
    printf("%-22s %4dx%-4d mines=%-5d revealed=%-7d avg=%9.4f ms\n",
           name, rows, cols, mines, revealed, total / reps);
    cleanupGameBoard(&b);
}

int main(void) {
    runCase("Пустое поле", 30, 30, 0, 500);
    runCase("Пустое поле", 100, 100, 0, 20);
    runCase("Пустое поле", 200, 200, 0, 5);
    runCase("С минами", 200, 200, 400, 5);
    return 0;
}