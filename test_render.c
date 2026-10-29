#include "raylib.h"
#include "game.h"
#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>

#define WARMUP_FRAMES 30
#define MEASURE_FRAMES 300

typedef enum { SCENE_CLOSED, 
               SCENE_OPENED, 
               SCENE_FLAGS, 
               SCENE_MINES 
} Scene;

static const char* sceneName(Scene s) {
    switch (s) {
        case SCENE_CLOSED: return "все закрыты";
        case SCENE_OPENED: return "все открыты";
        case SCENE_FLAGS:  return "все с флагами";
        case SCENE_MINES:  return "все с минами";
    }
    return "";
}

static void setupScene(GameBoard* b, Scene s) {
    for (int i = 0; i < b->rows * b->cols; i++) {
        Cell* c = &b->cells[i];
        c->isRevealed = false;
        c->hasFlag = false;
        c->hasExploded = false;
        c->isAnimating = false;
        switch (s) {
            case SCENE_CLOSED: break;
            case SCENE_OPENED: c->isRevealed = !c->isMine; break;
            case SCENE_FLAGS:  c->hasFlag = true; break;
            case SCENE_MINES:  c->isRevealed = c->isMine; break;
        }
    }
}

static double measureScene(const GameBoard* b, const Difficulty* d) {
    double total = 0;
    for (int f = 0; f < WARMUP_FRAMES + MEASURE_FRAMES; f++) {
        if (WindowShouldClose()) return -1;

        double t0 = GetTime();
        BeginDrawing();
        ClearBackground(COLOR_HEADER);
        for (int i = 0; i < d->rows; i++) {
            for (int j = 0; j < d->cols; j++) {
                int px = d->borderSize + j * d->cellSize;
                int py = d->headerHeight + i * d->cellSize;
                drawCell(b, i, j, px, py, d->cellSize, false);
            }
        }
        EndDrawing();
        double dt = GetTime() - t0;

        if (f >= WARMUP_FRAMES) total += dt;
    }
    return total / MEASURE_FRAMES * 1000.0;
}

static int runDifficulty(const char* name, int rows, int cols, int mines, int cellSize) {
    Difficulty d;
    initDifficulty(&d, rows, cols, mines, name, cellSize, 0);

    srand(12345);   
    GameBoard b;
    initGameBoard(&b, rows, cols, mines, 0);
    placeMines(&b, rows / 2, cols / 2);

    Scene scenes[] = {SCENE_CLOSED, SCENE_OPENED, SCENE_FLAGS, SCENE_MINES};
    for (int s = 0; s < 4; s++) {
        setupScene(&b, scenes[s]);
        double ms = measureScene(&b, &d);
        if (ms < 0) { cleanupGameBoard(&b); return 0; }
        printf("%-13s %2dx%-2d клетка=%2dpx  %-26s кадр=%8.3f ms  (~%7.1f FPS)\n",
               name, rows, cols, cellSize, sceneName(scenes[s]), ms, 1000.0 / ms);
        fflush(stdout);
    }
    cleanupGameBoard(&b);
    return 1;
}

int main(void) {
    
    InitWindow(1220, 750, "Render benchmark");
    SetTargetFPS(0);   

    if (!runDifficulty("Beginner", 9, 9, 10, 70)) goto done;
    if (!runDifficulty("Intermediate", 16, 16, 40, 40)) goto done;
    if (!runDifficulty("Expert", 16, 30, 99, 40)) goto done;

done:
    CloseWindow();
    return 0;
}