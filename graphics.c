#include "graphics.h"
#include <math.h>
#include <stdio.h> 

Color getNumberColor(int num) {
    switch (num) {
        case 1: return BLUE;
        case 2: return GREEN;
        case 3: return RED;
        case 4: return (Color){0, 0, 128, 255};
        case 5: return (Color){128, 0, 0, 255};
        case 6: return (Color){0, 128, 128, 255};
        case 7: return BLACK;
        case 8: return GRAY;
        default: return BLACK;
    }
}

void drawSmallFlag(int x, int y, int size) {
    int centerX = x + size / 2;
    
    DrawRectangle(centerX - 4, y + 3, 2, size - 7, COLOR_FLAG_POLE);
    DrawRectangle(centerX - 7, y + size - 6, 11, 3, COLOR_FLAG_BASE);
    DrawRectangle(centerX - 3, y + 3, size / 2, size / 3, COLOR_FLAG_CLOTH);
    DrawRectangleLines(centerX - 3, y + 3, size / 2, size / 3, (Color){180, 40, 40, 255});
}

void drawCell3D(int x, int y, int size, Color baseColor, bool raised) {
    DrawRectangle(x, y, size, size, baseColor);
    
    if (raised) {
        DrawRectangle(x, y, size, 3, COLOR_BORDER_LIGHT);
        DrawRectangle(x, y, 3, size, COLOR_BORDER_LIGHT);
        DrawRectangle(x + 2, y, size - 4, 2, COLOR_BORDER_LIGHT);
        DrawRectangle(x, y + 2, 2, size - 4, COLOR_BORDER_LIGHT);
        
        DrawRectangle(x, y + size - 3, size, 3, COLOR_BORDER_DARK);
        DrawRectangle(x + size - 3, y, 3, size, COLOR_BORDER_DARK);
        DrawRectangle(x + 2, y + size - 2, size - 4, 2, COLOR_BORDER_DARK);
        DrawRectangle(x + size - 2, y + 2, 2, size - 4, COLOR_BORDER_DARK);
    } else {
        DrawRectangle(x, y, size, 2, COLOR_BORDER_DARK);
        DrawRectangle(x, y, 2, size, COLOR_BORDER_DARK);
        DrawRectangle(x, y + size - 2, size, 2, COLOR_BORDER_LIGHT);
        DrawRectangle(x + size - 2, y, 2, size, COLOR_BORDER_LIGHT);
    }
}

void drawFlag(int x, int y, int size) {
    int centerX = x + size / 2 - (size >= 65 ? 6 : (size >= 55 ? 5 : 3));
    int flagTopY = y + size / 6;
    int poleBottomY = y + size - (size >= 65 ? 16 : (size >= 55 ? 15 : 9));
    int poleWidth = (size >= 55 ? 4 : 3);
    int poleXOffset = -(poleWidth / 2);
    int baseWidth = (size >= 65 ? 30 : (size >= 55 ? 27 : 17));
    int baseHeight = (size >= 65 ? 8 : (size >= 55 ? 7 : 5));
    int baseXOffset = -baseWidth / 2;
    int clothWidth = (size >= 65 ? 24 : (size >= 55 ? 22 : 14));
    int clothHeight = (size >= 65 ? 22 : (size >= 55 ? 20 : 13));
    int clothXOffset = (size >= 55 ? 2 : 1);
    int clothYOffset = (size >= 55 ? 2 : 1);
    
    DrawRectangle(centerX + poleXOffset, flagTopY, poleWidth, poleBottomY - flagTopY, COLOR_FLAG_POLE);
    DrawRectangle(centerX + baseXOffset, poleBottomY, baseWidth, baseHeight, COLOR_FLAG_BASE);
    DrawRectangle(centerX + clothXOffset, flagTopY + clothYOffset, clothWidth, clothHeight, COLOR_FLAG_CLOTH);
}

void drawBomb(int x, int y, int size, bool isRed, bool hasWick) {
    int cx = x + size / 2;
    int cy = y + size / 2;
    Color bombDark = isRed ? (Color){180, 30, 30, 255} : (Color){15, 15, 20, 255};
    Color bombLight = isRed ? (Color){220, 60, 60, 255} : (Color){60, 60, 70, 255};
    
    DrawCircle(cx + 3, cy + 4, size / 3.5, (Color){0, 0, 0, 150});
    DrawCircle(cx + 2, cy + 3, size / 4, (Color){0, 0, 0, 100});
    
    DrawCircle(cx, cy, size / 3.2, bombDark);
    
    for (int r = size / 3.2; r >= 0; r--) {
        int shade = isRed ? 120 + (int)((size / 3.2 - r) * 6) : 15 + (int)((size / 3.2 - r) * 3);
        Color circleColor = isRed ? (Color){shade, shade * 0.25f, shade * 0.25f, 255} : (Color){shade, shade, shade + 5, 255};
        DrawCircle(cx, cy - 2, r, circleColor);
    }
    
    DrawCircle(cx - size / 12, cy - size / 10, size / 10, isRed ? (Color){255, 100, 100, 200} : (Color){100, 100, 110, 200});
    DrawCircle(cx - size / 12, cy - size / 10, size / 15, isRed ? (Color){255, 150, 150, 180} : (Color){120, 120, 130, 180});
    DrawCircle(cx - size / 8, cy - size / 7, size / 25, isRed ? (Color){255, 200, 200, 150} : (Color){150, 150, 160, 150});
    DrawCircle(cx + size / 10, cy + size / 12, size / 20, (Color){0, 0, 0, 100});
    DrawCircleLines(cx, cy, size / 3.2, isRed ? (Color){100, 10, 10, 255} : (Color){0, 0, 0, 255});
    
    if (hasWick) {
        Vector2 points[] = {
            {cx + size / 6.0f, cy - size / 4.0f},
            {cx + size / 4.0f, cy - size / 3.0f},
            {cx + size / 3.0f, cy - size / 2.5f},
            {cx + size / 3.5f, cy - size / 2.0f}
        };
        DrawLineEx(points[0], points[1], size / 15.0f, (Color){101, 67, 33, 255});
        DrawLineEx(points[1], points[2], size / 20.0f, (Color){101, 67, 33, 255});
        DrawLineEx(points[2], points[3], size / 30.0f, (Color){101, 67, 33, 255});
        DrawLineEx(points[0], points[1], size / 30.0f, (Color){139, 90, 43, 255});
        
        float pulse = (float)(GetTime() * 10);
        int flicker = (int)pulse % 3;
        DrawCircle(points[3].x, points[3].y, size / 15.0f + flicker, ORANGE);
        DrawCircle(points[3].x, points[3].y, size / 25.0f + flicker / 2, YELLOW);
        DrawCircle(points[3].x - 1, points[3].y - 1, size / 50.0f, WHITE);
    }
    
    int spikes[8][2] = {
        {size / 2.7f, 0}, {size / 4, size / 4}, {0, size / 2.7f}, {-size / 4, size / 4},
        {-size / 2.7f, 0}, {-size / 4, -size / 4}, {0, -size / 2.7f}, {size / 4, -size / 4}
    };
    
    for (int i = 0; i < 8; i++) {
        int sx = cx + spikes[i][0];
        int sy = cy + spikes[i][1];
        DrawCircle(sx + 1, sy + 1, size / 20.0f, (Color){0, 0, 0, 150});
        Color spikeColor = isRed ? bombLight : (Color){70, 70, 80, 255};
        DrawCircle(sx, sy, size / 20.0f, spikeColor);
        DrawCircle(sx - 1, sy - 1, size / 50.0f, isRed ? (Color){255, 120, 120, 255} : (Color){120, 120, 130, 255});
    }
}

void drawExplosionBackground(int x, int y, int size, int frame) {
    int cx = x + size / 2;
    int cy = y + size / 2;
    float progress = (float)frame / EXPLOSION_FRAMES;
    int radius = (int)(progress * EXPLOSION_MAX_RADIUS * size / 65.0);
    
    Color glow;
    if (progress < 0.3f) {
        glow = (Color){255, 200, 100, 200};
    } else if (progress < 0.6f) {
        int alpha = 200 - (int)((progress - 0.3f) * 200 / 0.3f);
        glow = (Color){255, 150, 50, alpha};
    } else {
        int alpha = 100 - (int)((progress - 0.6f) * 100 / 0.4f);
        glow = (Color){255, 50, 50, alpha};
    }
    
    DrawCircle(cx, cy, radius + 8, glow);
    DrawCircle(cx, cy, radius + 5, (Color){255, 100, 50, glow.a / 2});
    
    Color core;
    if (progress < 0.3f) {
        core = (Color){255, 255, 200, 255};
    } else if (progress < 0.6f) {
        core = (Color){255, 100, 50, 255 - (int)((progress - 0.3f) * 255 / 0.3f)};
    } else {
        core = (Color){200, 50, 50, 100 - (int)((progress - 0.6f) * 100 / 0.4f)};
    }
    
    DrawCircle(cx, cy, radius - 5, core);
    
    if (frame < EXPLOSION_FRAMES - 5) {
        for (int i = 0; i < 12; i++) {
            float angle = (i + frame * 0.5f) * 30 * DEG2RAD;
            int d = radius + (int)(progress * 15);
            int px = cx + (int)(cos(angle) * d);
            int py = cy + (int)(sin(angle) * d);
            DrawCircle(px, py, 2 + (int)(progress * 2), YELLOW);
        }
    }
    
    if (progress > 0.5f) {
        int alpha = (int)((progress - 0.5f) * 200 / 0.5f);
        DrawCircle(cx + 5, cy - 5, radius - 10, (Color){80, 80, 80, alpha / 2});
        DrawCircle(cx - 3, cy + 3, radius - 12, (Color){60, 60, 60, alpha / 2});
    }
}

void drawRestartButton(int x, int y, int w, int h, bool hover) {
    Color col = hover ? (Color){240, 240, 240, 255} : COLOR_BUTTON;
    DrawRectangle(x, y, w, h, col);
    DrawRectangle(x, y, w, 3, COLOR_BORDER_LIGHT);
    DrawRectangle(x, y, 3, h, COLOR_BORDER_LIGHT);
    DrawRectangle(x, y + h - 3, w, 3, COLOR_BORDER_DARK);
    DrawRectangle(x + w - 3, y, 3, h, COLOR_BORDER_DARK);
    
    int tw = MeasureText("MENU", 25);
    DrawText("MENU", x + (w - tw) / 2, y + h / 2 - 12, 25, BLACK);
}

void drawInfoButton(int x, int y, int r, bool hover) {
    Color col = hover ? (Color){100, 100, 100, 255} : (Color){80, 80, 80, 255};
    DrawCircle(x, y, r, col);
    DrawCircleLines(x, y, r, COLOR_BORDER_DARK);
    DrawText("i", x - MeasureText("i", r) / 2, y - r / 2, r, WHITE);
}

void drawAboutButton(int x, int y, int r, bool hover) {
    Color col = hover ? (Color){100, 100, 100, 255} : (Color){80, 80, 80, 255};
    DrawCircle(x, y, r, col);
    DrawCircleLines(x, y, r, COLOR_BORDER_DARK);
    DrawText("A", x - MeasureText("A", r) / 2, y - r / 2, r, WHITE);
}

void drawCell(const GameBoard* board, int row, int col, int x, int y, int size, bool isExplodedMine) {
    const Cell* cell = getCell(board, row, col);
    if (!cell) return;
    
    bool closed = !cell->isRevealed;
    drawCell3D(x, y, size, closed ? COLOR_CELL_CLOSED : COLOR_CELL_OPENED, closed);
    
    if (cell->isRevealed) {
        if (cell->isMine) {
            drawBomb(x, y, size, isExplodedMine, !cell->hasExploded);
        } else if (cell->mineCount > 0) {
            char txt[2] = {cell->mineCount + '0', '\0'};
            int fs = size / 2;
            int tw = MeasureText(txt, fs);
            DrawText(txt, x + (size - tw) / 2, y + (size - fs) / 2, fs, getNumberColor(cell->mineCount));
        }
    } else if (cell->hasFlag) {
        if (size <= 30) {
            drawSmallFlag(x, y, size);
        } else {
            drawFlag(x, y, size);
        }
    }
    
    if (cell->isAnimating) {
        drawExplosionBackground(x, y, size, EXPLOSION_FRAMES - 5);
    }
}

void initDifficulty(Difficulty* d, int rows, int cols, int bombs, const char* name, int cellSize, int diff) {
    if (!d) return;
    
    d->rows = rows;
    d->cols = cols;
    d->bombs = bombs;
    d->cellSize = cellSize;
    d->headerHeight = 100;
    d->borderSize = 10;
    d->difficulty = diff;
    snprintf(d->name, sizeof(d->name), "%s", name);
    d->windowWidth = d->cellSize * cols + d->borderSize * 2;
    d->windowHeight = d->headerHeight + d->cellSize * rows + d->borderSize;
}