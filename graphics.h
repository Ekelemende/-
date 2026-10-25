#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "raylib.h"
#include "game.h"

#define COLOR_CELL_CLOSED (Color){192, 192, 192, 255}
#define COLOR_CELL_OPENED (Color){220, 220, 220, 255}
#define COLOR_BORDER_LIGHT (Color){255, 255, 255, 255}
#define COLOR_BORDER_DARK (Color){128, 128, 128, 255}
#define COLOR_HEADER (Color){192, 192, 192, 255}
#define COLOR_BUTTON (Color){220, 220, 220, 255}
#define COLOR_FLAG_POLE (Color){40, 40, 40, 255}
#define COLOR_FLAG_BASE (Color){50, 50, 50, 255}
#define COLOR_FLAG_CLOTH RED

#define EXPLOSION_FRAMES 30
#define EXPLOSION_MAX_RADIUS 45

typedef struct {
    int rows;
    int cols;
    int bombs;
    int cellSize;
    int headerHeight;
    int borderSize;
    int windowWidth;
    int windowHeight;
    char name[20];
    int difficulty;
} Difficulty;

void initDifficulty(Difficulty* d, int rows, int cols, int bombs, const char* name, int cellSize, int diff);
void drawCell3D(int x, int y, int size, Color baseColor, bool raised);
void drawFlag(int x, int y, int size);
void drawSmallFlag(int x, int y, int size);
void drawBomb(int x, int y, int size, bool isRed, bool hasWick);
void drawExplosionBackground(int x, int y, int size, int frame);
void drawRestartButton(int x, int y, int w, int h, bool hover);
void drawInfoButton(int x, int y, int r, bool hover);
void drawAboutButton(int x, int y, int r, bool hover);
void drawCell(const GameBoard* board, int row, int col, int x, int y, int size, bool isExplodedMine);
Color getNumberColor(int num);

#endif // GRAPHICS_H