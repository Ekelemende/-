#define _POSIX_C_SOURCE 199309L
#include "game.h"
#include "bot.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

// поле 2 x k: верхний ряд открыт, нижний ряд закрыт
// мины случайно стоят только в нижнем ряду, цифры согласованы с ними.
static void buildStrip(GameBoard* b, int k, double density) {
    initGameBoard(b, 2, k, 0, 0);
    b->firstMove = false;
    for (int j = 0; j < k; j++)
        getCell(b, 1, j)->isMine = ((rand() % 1000) < density * 1000);
    for (int j = 0; j < k; j++) {
        int cnt = 0;
        for (int dj = -1; dj <= 1; dj++) {
            Cell* nb = getCell(b, 1, j + dj);
            if (nb && nb->isMine) cnt++;
        }
        Cell* top = getCell(b, 0, j);
        top->mineCount = cnt;
        top->isRevealed = true;
    }
}

static void runCase(int k, int reps) {
    srand(12345);                         
    double total = 0;
    int moves = 0;
    for (int r = 0; r < reps; r++) {
        GameBoard b;
        buildStrip(&b, k, 0.4);
        double t0 = time_ms();
        bool ok = solveByBruteForce(&b);
        total += time_ms() - t0;
        moves += ok;
        cleanupGameBoard(&b);
    }
    printf("полоса k=%-3d  повторов=%-4d ходов найдено=%-4d avg=%10.4f ms\n",
           k, reps, moves, total / reps);
}

int main(void) {
    int ks[] = {8, 12, 16, 18, 20, 22, 24};
    int reps[] = {200, 100, 20, 10, 5, 3, 2};
    for (int i = 0; i < 7; i++) runCase(ks[i], reps[i]);
    return 0;
}
