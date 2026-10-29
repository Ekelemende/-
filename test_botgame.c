#define _POSIX_C_SOURCE 199309L
#include "game.h"
#include "bot.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_MOVES 20000

static double time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

typedef struct { int wins, losses, stuck; long moves; double moveTimeUs; double gameTimeUs; } Stats;

static void playGame(int rows, int cols, int mines, bool hard, Stats* st) {
    GameBoard b;
    initGameBoard(&b, rows, cols, mines, 0);

    double g0 = time_us();
    int moves = 0;
    for (;;) {
        double t0 = time_us();
        bool ok = hard ? botMakeMoveHard(&b) : botMakeMoveSimple(&b);
        st->moveTimeUs += time_us() - t0;
        moves++;

        if (hasRevealedMine(&b)) { st->losses++; break; }
        if (checkVictory(&b))    { st->wins++;   break; }
        if (!ok || moves >= MAX_MOVES) { st->stuck++; break; }
    }
    st->gameTimeUs += time_us() - g0;
    st->moves += moves;
    cleanupGameBoard(&b);
}

static void runCase(const char* name, int rows, int cols, int mines, bool hard, int games) {
    srand(12345);                        
    Stats st = {0};
    for (int g = 0; g < games; g++) playGame(rows, cols, mines, hard, &st);

    printf("%-6s %-13s игр=%-4d побед=%-4d поражений=%-4d зависло=%-3d "
           "ходов на партию=%6.1f  время хода=%8.3f us  время партии=%9.3f ms\n",
           hard ? "ADVANCED" : "SIMPLE", name, games, st.wins, st.losses, st.stuck,
           (double)st.moves / games, st.moveTimeUs / st.moves, st.gameTimeUs / games / 1000.0);
}

int main(void) {
    runCase("Beginner", 9, 9, 10, false, 2000);
    runCase("Intermediate", 16, 16, 40, false, 1000);
    runCase("Expert", 16, 30, 99, false, 500);
    runCase("Beginner", 9, 9, 10, true, 2000);
    runCase("Intermediate", 16, 16, 40, true, 1000);
    runCase("Expert", 16, 30, 99, true, 500);
    return 0;
}