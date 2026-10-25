#include "dual_bot.h"
#include "bot.h"
#include "records.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

void initBotGameState(BotGameState* state) {
    if (!state) return;
    state->active = true;
    state->win = false;
    state->lose = false;
    state->explosionActive = false;
    state->simultaneousMode = false;
    state->elapsedSeconds = 0;
    state->startTime = 0;
    state->lastMoveTime = 0;
    state->bombList = NULL;
    state->bombCount = 0;
    state->currentExplosionIndex = 0;
    state->explosionFrame = 0;
    state->explodedPosition.row = -1;
    state->explodedPosition.col = -1;
}

void cleanupBotGameState(BotGameState* state) {
    if (state && state->bombList) {
        free(state->bombList);
        state->bombList = NULL;
    }
}

void collectMinesList(const GameBoard* board, Position** bombList, int* bombCount, Position explodedPos) {
    if (!board || !bombList || !bombCount) return;
    
    *bombCount = 0;
    int totalCells = board->rows * board->cols;
    for (int i = 0; i < totalCells; i++) {
        if (board->cells[i].isMine) (*bombCount)++;
    }
    
    *bombList = (Position*)malloc(*bombCount * sizeof(Position));
    if (!*bombList) return;
    
    int idx = 0;
    for (int i = 0; i < board->rows; i++) {
        for (int j = 0; j < board->cols; j++) {
            if (getCell(board, i, j)->isMine) {
                (*bombList)[idx].row = i;
                (*bombList)[idx].col = j;
                idx++;
            }
        }
    }
    
    for (int i = 0; i < *bombCount; i++) {
        if ((*bombList)[i].row == explodedPos.row && (*bombList)[i].col == explodedPos.col) {
            Position temp = (*bombList)[0];
            (*bombList)[0] = (*bombList)[i];
            (*bombList)[i] = temp;
            break;
        }
    }
}

void updateExplosionAnimation(BotGameState* state, GameBoard* board) {
    if (!state->explosionActive || !state->bombList) return;
    
    if (state->simultaneousMode) {
        if (state->explosionFrame < EXPLOSION_FRAMES) {
            state->explosionFrame++;
        } else {
            for (int i = state->currentExplosionIndex; i < state->bombCount; i++) {
                Cell* c = getCell(board, state->bombList[i].row, state->bombList[i].col);
                if (c) {
                    c->hasExploded = true;
                    c->isAnimating = false;
                }
            }
            state->explosionActive = false;
            state->lose = true;
        }
    } else {
        if (state->explosionFrame < EXPLOSION_FRAMES) {
            state->explosionFrame++;
        } else {
            if (state->currentExplosionIndex < state->bombCount) {
                Cell* c = getCell(board, state->bombList[state->currentExplosionIndex].row, 
                                state->bombList[state->currentExplosionIndex].col);
                if (c) {
                    c->hasExploded = true;
                    c->isAnimating = false;
                }
            }
            state->currentExplosionIndex++;
            state->explosionFrame = 0;
            
            if (state->currentExplosionIndex >= SEQUENTIAL_EXPLOSIONS || 
                state->currentExplosionIndex >= state->bombCount) {
                if (state->currentExplosionIndex < state->bombCount) {
                    state->simultaneousMode = true;
                    for (int i = state->currentExplosionIndex; i < state->bombCount; i++) {
                        Cell* c = getCell(board, state->bombList[i].row, state->bombList[i].col);
                        if (c) c->isAnimating = true;
                    }
                } else {
                    state->explosionActive = false;
                    state->lose = true;
                }
            }
        }
    }
}

bool processBotTurn(GameBoard* board, BotGameState* state, bool useHardLogic, double delay) {
    if (!state->active || state->win) return false;
    
    double now = GetTime();
    if (now - state->lastMoveTime < delay) return false;
    
    bool moveResult = useHardLogic ? botMakeMoveHard(board) : botMakeMoveSimple(board);
    if (!moveResult) return false;
    
    state->lastMoveTime = now;
    
    if (hasRevealedMine(board)) {
        state->active = false;
        
        for (int i = 0; i < board->rows; i++) {
            for (int j = 0; j < board->cols; j++) {
                Cell* c = getCell(board, i, j);
                if (c->isMine && c->isRevealed) {
                    state->explodedPosition.row = i;
                    state->explodedPosition.col = j;
                    break;
                }
            }
        }
        
        collectMinesList(board, &state->bombList, &state->bombCount, state->explodedPosition);
        state->explosionActive = true;
        state->currentExplosionIndex = 0;
        state->explosionFrame = 0;
        state->simultaneousMode = false;
        
        for (int i = 0; i < board->rows; i++) {
            for (int j = 0; j < board->cols; j++) {
                if (getCell(board, i, j)->isMine) {
                    getCell(board, i, j)->isRevealed = true;
                }
            }
        }
    } else if (checkVictory(board)) {
        state->win = true;
        state->active = false;
    }
    
    return true;
}

void runDualBotGame(Difficulty diff, HighScores* scores, int difficultyId) {
    Difficulty displayDiff = diff;
    
    // Для EXPERT - вертикальное расположение
    if (diff.difficulty == 2) {
        int newCellSize = 23;
        displayDiff.cellSize = newCellSize;
        displayDiff.windowWidth = newCellSize * diff.cols + displayDiff.borderSize * 2;
        displayDiff.windowHeight = displayDiff.headerHeight + newCellSize * diff.rows + displayDiff.borderSize;
        
        int infoPanelWidth = 200;
        int windowWidth = displayDiff.windowWidth + infoPanelWidth + 60;
        int windowHeight = 2 * displayDiff.windowHeight;
        
        InitWindow(windowWidth, windowHeight - 175, TextFormat("Minesweeper - %s vs %s (Dual Bot)", diff.name, diff.name));
        
        int topFieldX = 30;
        int topFieldY = 10;
        int bottomFieldX = 30;
        int bottomFieldY = displayDiff.windowHeight - 85;
        int infoX = displayDiff.windowWidth + 50;
        
        GameBoard tempBoard;
        if (!initGameBoard(&tempBoard, diff.rows, diff.cols, diff.bombs, diff.difficulty)) {
            return;
        }
        
        int safeRow = rand() % diff.rows;
        int safeCol = rand() % diff.cols;
        placeMines(&tempBoard, safeRow, safeCol);
        
        GameBoard bot1Board, bot2Board;
        cloneBoardForDual(&tempBoard, &bot1Board);
        cloneBoardForDual(&tempBoard, &bot2Board);
        
        BotGameState bot1, bot2;
        initBotGameState(&bot1);
        initBotGameState(&bot2);
        
        bool initialDelayDone = false;
        double initialDelayStart = GetTime();
        
        int btnW = 120, btnH = 40;
        int btnX = infoX + (infoPanelWidth - btnW) / 2;
        int btnY = 365;
        Rectangle restartBtn = {btnX, btnY, btnW, btnH};
        
        int titleFontSize = 16;
        int infoFontSize = 18;
        int gameOverFontSize = 50;
        int winFontSize = 50;
        int finalFontSize = 40;
        
        while (!WindowShouldClose()) {
            Vector2 mouse = GetMousePosition();
            bool hoverRestart = CheckCollisionPointRec(mouse, restartBtn);
            
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverRestart) {
                break;
            }
            
            double now = GetTime();
            
            if (!initialDelayDone) {
                if (now - initialDelayStart >= 1.0) {
                    initialDelayDone = true;
                    
                    int safeRow1, safeCol1;
                    do {
                        safeRow1 = rand() % diff.rows;
                        safeCol1 = rand() % diff.cols;
                    } while (getCell(&bot1Board, safeRow1, safeCol1)->isMine);
                    revealCell(&bot1Board, safeRow1, safeCol1);
                    
                    int safeRow2, safeCol2;
                    do {
                        safeRow2 = rand() % diff.rows;
                        safeCol2 = rand() % diff.cols;
                    } while (getCell(&bot2Board, safeRow2, safeCol2)->isMine);
                    revealCell(&bot2Board, safeRow2, safeCol2);
                    
                    bot1.startTime = now;
                    bot2.startTime = now;
                    bot1.lastMoveTime = now;
                    bot2.lastMoveTime = now;
                }
            } else {
                processBotTurn(&bot1Board, &bot1, false, BOT_MOVE_DELAY);
                processBotTurn(&bot2Board, &bot2, true, BOT_MOVE_DELAY);
                
                if (bot1.active && !bot1.win) {
                    bot1.elapsedSeconds = (int)(now - bot1.startTime);
                }
                if (bot2.active && !bot2.win) {
                    bot2.elapsedSeconds = (int)(now - bot2.startTime);
                }
            }
            
            updateExplosionAnimation(&bot1, &bot1Board);
            updateExplosionAnimation(&bot2, &bot2Board);
            
            bool bothFinished = !bot1.active && !bot2.active && !bot1.explosionActive && !bot2.explosionActive;
            
            BeginDrawing();
            ClearBackground(COLOR_HEADER);
            DrawRectangle(0, 0, windowWidth, windowHeight, COLOR_HEADER);
            
            // Верхнее поле (Simple Bot)
            DrawText("SIMPLE BOT", infoX + (infoPanelWidth - MeasureText("SIMPLE BOT", titleFontSize)) / 2, 
                     topFieldY + 150, titleFontSize, DARKBLUE);
            
            char bombs1[32];
            sprintf(bombs1, "Bombs: %d", bot1Board.totalMines - bot1Board.flagsPlaced);
            int bw1 = MeasureText(bombs1, infoFontSize);
            DrawText(bombs1, infoX + (infoPanelWidth - bw1) / 2, topFieldY + 180, infoFontSize, BLACK);
            
            char time1[32];
            sprintf(time1, "Time: %d", bot1.elapsedSeconds);
            int tw1 = MeasureText(time1, infoFontSize);
            DrawText(time1, infoX + (infoPanelWidth - tw1) / 2, topFieldY + 210, infoFontSize, BLACK);
            
            for (int i = 0; i < displayDiff.rows; i++) {
                for (int j = 0; j < displayDiff.cols; j++) {
                    int px = topFieldX + j * displayDiff.cellSize;
                    int py = topFieldY + i * displayDiff.cellSize;
                    
                    if (!bot1.simultaneousMode && bot1.explosionActive && bot1.bombList && 
                        bot1.currentExplosionIndex < bot1.bombCount) {
                        if (bot1.bombList[bot1.currentExplosionIndex].row == i && 
                            bot1.bombList[bot1.currentExplosionIndex].col == j && 
                            bot1.explosionFrame > 0) {
                            drawExplosionBackground(px, py, displayDiff.cellSize, bot1.explosionFrame);
                        }
                    }
                    
                    bool isExploded = (i == bot1.explodedPosition.row && j == bot1.explodedPosition.col);
                    drawCell(&bot1Board, i, j, px, py, displayDiff.cellSize, isExploded);
                }
            }
            DrawRectangleLines(topFieldX - 1, topFieldY - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                             displayDiff.cellSize * displayDiff.rows + 2, COLOR_BORDER_DARK);
            
            if (bot1.win) {
                DrawRectangle(topFieldX - 1, topFieldY - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                            displayDiff.cellSize * displayDiff.rows + 2, (Color){0, 0, 0, 180});
                int tw2 = MeasureText("WIN", winFontSize);
                DrawText("WIN", topFieldX + displayDiff.cellSize * displayDiff.cols / 2 - tw2 / 2, 
                        topFieldY + displayDiff.cellSize * displayDiff.rows / 2 - winFontSize / 2, winFontSize, GREEN);
            } else if (bot1.lose) {
                DrawRectangle(topFieldX - 1, topFieldY - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                            displayDiff.cellSize * displayDiff.rows + 2, (Color){0, 0, 0, 180});
                int tw2 = MeasureText("GAME OVER", gameOverFontSize);
                DrawText("GAME OVER", topFieldX + displayDiff.cellSize * displayDiff.cols / 2 - tw2 / 2, 
                        topFieldY + displayDiff.cellSize * displayDiff.rows / 2 - gameOverFontSize / 2, gameOverFontSize, RED);
            }
            
            // Нижнее поле (Advanced Bot)
            DrawText("ADVANCED BOT", infoX + (infoPanelWidth - MeasureText("ADVANCED BOT", titleFontSize)) / 2, 
                     bottomFieldY + 150, titleFontSize, DARKBLUE);
            
            char bombs2[32];
            sprintf(bombs2, "Bombs: %d", bot2Board.totalMines - bot2Board.flagsPlaced);
            int bw2 = MeasureText(bombs2, infoFontSize);
            DrawText(bombs2, infoX + (infoPanelWidth - bw2) / 2, bottomFieldY + 180, infoFontSize, BLACK);
            
            char time2[32];
            sprintf(time2, "Time: %d", bot2.elapsedSeconds);
            int tw2 = MeasureText(time2, infoFontSize);
            DrawText(time2, infoX + (infoPanelWidth - tw2) / 2, bottomFieldY + 210, infoFontSize, BLACK);
            
            for (int i = 0; i < displayDiff.rows; i++) {
                for (int j = 0; j < displayDiff.cols; j++) {
                    int px = bottomFieldX + j * displayDiff.cellSize;
                    int py = bottomFieldY + i * displayDiff.cellSize;
                    
                    if (!bot2.simultaneousMode && bot2.explosionActive && bot2.bombList && 
                        bot2.currentExplosionIndex < bot2.bombCount) {
                        if (bot2.bombList[bot2.currentExplosionIndex].row == i && 
                            bot2.bombList[bot2.currentExplosionIndex].col == j && 
                            bot2.explosionFrame > 0) {
                            drawExplosionBackground(px, py, displayDiff.cellSize, bot2.explosionFrame);
                        }
                    }
                    
                    bool isExploded = (i == bot2.explodedPosition.row && j == bot2.explodedPosition.col);
                    drawCell(&bot2Board, i, j, px, py, displayDiff.cellSize, isExploded);
                }
            }
            DrawRectangleLines(bottomFieldX - 1, bottomFieldY - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                             displayDiff.cellSize * displayDiff.rows + 2, COLOR_BORDER_DARK);
            
            if (bot2.win) {
                DrawRectangle(bottomFieldX - 1, bottomFieldY - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                            displayDiff.cellSize * displayDiff.rows + 2, (Color){0, 0, 0, 180});
                int tw3 = MeasureText("WIN", winFontSize);
                DrawText("WIN", bottomFieldX + displayDiff.cellSize * displayDiff.cols / 2 - tw3 / 2, 
                        bottomFieldY + displayDiff.cellSize * displayDiff.rows / 2 - winFontSize / 2, winFontSize, GREEN);
            } else if (bot2.lose) {
                DrawRectangle(bottomFieldX - 1, bottomFieldY - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                            displayDiff.cellSize * displayDiff.rows + 2, (Color){0, 0, 0, 180});
                int tw3 = MeasureText("GAME OVER", gameOverFontSize);
                DrawText("GAME OVER", bottomFieldX + displayDiff.cellSize * displayDiff.cols / 2 - tw3 / 2, 
                        bottomFieldY + displayDiff.cellSize * displayDiff.rows / 2 - gameOverFontSize / 2, gameOverFontSize, RED);
            }
            
            drawRestartButton(btnX, btnY, btnW, btnH, hoverRestart);
            
            if (bothFinished) {
                const char* winnerText = "";
                Color winColor = YELLOW;
                
                if (bot1.win && !bot2.win) {
                    winnerText = "SIMPLE BOT WINS!";
                    winColor = GREEN;
                } else if (!bot1.win && bot2.win) {
                    winnerText = "ADVANCED BOT WINS!";
                    winColor = GREEN;
                } else if (bot1.win && bot2.win) {
                    if (bot1.elapsedSeconds < bot2.elapsedSeconds) {
                        winnerText = "SIMPLE BOT WINS!";
                        winColor = GREEN;
                    } else if (bot2.elapsedSeconds < bot1.elapsedSeconds) {
                        winnerText = "ADVANCED BOT WINS!";
                        winColor = GREEN;
                    } else {
                        winnerText = "DRAW!";
                        winColor = YELLOW;
                    }
                } else {
                    int safe1 = getSafeRevealedCount(&bot1Board);
                    int safe2 = getSafeRevealedCount(&bot2Board);
                    if (safe1 > safe2) {
                        winnerText = "SIMPLE BOT WINS!";
                        winColor = GREEN;
                    } else if (safe2 > safe1) {
                        winnerText = "ADVANCED BOT WINS!";
                        winColor = GREEN;
                    } else {
                        winnerText = "DRAW!";
                        winColor = YELLOW;
                    }
                }
                
                DrawRectangle(0, 0, windowWidth, windowHeight, (Color){0, 0, 0, 200});
                int tw = MeasureText(winnerText, finalFontSize);
                DrawText(winnerText, (windowWidth - tw) / 2, windowHeight / 2 - finalFontSize - 100, finalFontSize, winColor);
                
                int exitFontSize = 18;
                DrawText("Press MENU to exit", (windowWidth - MeasureText("Press MENU to exit", exitFontSize)) / 2, 
                        windowHeight / 2 - finalFontSize - 40, exitFontSize, WHITE);
            }
            
            EndDrawing();
        }
        
        cleanupGameBoard(&tempBoard);
        cleanupGameBoard(&bot1Board);
        cleanupGameBoard(&bot2Board);
        cleanupBotGameState(&bot1);
        cleanupBotGameState(&bot2);
        CloseWindow();
        return;
    }
    
    // Для BEGINNER и INTERMEDIATE - горизонтальное расположение
    GameBoard tempBoard;
    if (!initGameBoard(&tempBoard, diff.rows, diff.cols, diff.bombs, diff.difficulty)) {
        return;
    }
    
    int safeRow = rand() % diff.rows;
    int safeCol = rand() % diff.cols;
    placeMines(&tempBoard, safeRow, safeCol);
    
    GameBoard bot1Board, bot2Board;
    cloneBoardForDual(&tempBoard, &bot1Board);
    cloneBoardForDual(&tempBoard, &bot2Board);
    
    int windowWidth = 2 * displayDiff.windowWidth + 60;
    int windowHeight = displayDiff.windowHeight;
    InitWindow(windowWidth, windowHeight, TextFormat("Minesweeper - %s vs %s", diff.name, diff.name));
    SetTargetFPS(60);
    
    BotGameState bot1, bot2;
    initBotGameState(&bot1);
    initBotGameState(&bot2);
    
    bool initialDelayDone = false;
    double initialDelayStart = GetTime();
    
    int leftX = 30;
    int rightX = 30 + displayDiff.windowWidth + 30;
    int yStart = displayDiff.headerHeight;
    int btnW = 150, btnH = 45;
    int btnX = (windowWidth - btnW) / 2;
    Rectangle restartBtn = {btnX, 25, btnW, btnH};
    
    int titleFontSize = 20;
    int infoFontSize = 25;
    int gameOverFontSize = 100;
    int winFontSize = 100;
    int finalFontSize = 80;
    
    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        bool hoverRestart = CheckCollisionPointRec(mouse, restartBtn);
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverRestart) {
            break;
        }
        
        double now = GetTime();
        
        if (!initialDelayDone) {
            if (now - initialDelayStart >= 1.0) {
                initialDelayDone = true;
                
                int safeRow1, safeCol1;
                do {
                    safeRow1 = rand() % diff.rows;
                    safeCol1 = rand() % diff.cols;
                } while (getCell(&bot1Board, safeRow1, safeCol1)->isMine);
                revealCell(&bot1Board, safeRow1, safeCol1);
                
                int safeRow2, safeCol2;
                do {
                    safeRow2 = rand() % diff.rows;
                    safeCol2 = rand() % diff.cols;
                } while (getCell(&bot2Board, safeRow2, safeCol2)->isMine);
                revealCell(&bot2Board, safeRow2, safeCol2);
                
                bot1.startTime = now;
                bot2.startTime = now;
                bot1.lastMoveTime = now;
                bot2.lastMoveTime = now;
            }
        } else {
            processBotTurn(&bot1Board, &bot1, false, BOT_MOVE_DELAY);
            processBotTurn(&bot2Board, &bot2, true, BOT_MOVE_DELAY);
            
            if (bot1.active && !bot1.win) {
                bot1.elapsedSeconds = (int)(now - bot1.startTime);
            }
            if (bot2.active && !bot2.win) {
                bot2.elapsedSeconds = (int)(now - bot2.startTime);
            }
        }
        
        updateExplosionAnimation(&bot1, &bot1Board);
        updateExplosionAnimation(&bot2, &bot2Board);
        
        bool bothFinished = !bot1.active && !bot2.active && !bot1.explosionActive && !bot2.explosionActive;
        
        BeginDrawing();
        ClearBackground(COLOR_HEADER);
        
        DrawRectangle(0, 0, windowWidth, displayDiff.headerHeight, COLOR_HEADER);
        DrawRectangleLines(0, 0, windowWidth, displayDiff.headerHeight, COLOR_BORDER_DARK);
        drawRestartButton(btnX, 25, btnW, btnH, hoverRestart);
        
        DrawText("SIMPLE BOT", leftX + (displayDiff.cellSize * displayDiff.cols - MeasureText("SIMPLE BOT", titleFontSize)) / 2, 10, titleFontSize, DARKBLUE);
        
        char bombs1[32];
        sprintf(bombs1, "Bombs: %d", bot1Board.totalMines - bot1Board.flagsPlaced);
        int bw1 = MeasureText(bombs1, infoFontSize);
        DrawText(bombs1, leftX + (displayDiff.cellSize * displayDiff.cols - bw1) / 2, 35, infoFontSize, BLACK);
        
        char time1[32];
        sprintf(time1, "Time: %d", bot1.elapsedSeconds);
        int tw1 = MeasureText(time1, infoFontSize);
        DrawText(time1, leftX + (displayDiff.cellSize * displayDiff.cols - tw1) / 2, 65, infoFontSize, BLACK);
        
        for (int i = 0; i < displayDiff.rows; i++) {
            for (int j = 0; j < displayDiff.cols; j++) {
                int px = leftX + j * displayDiff.cellSize;
                int py = yStart + i * displayDiff.cellSize;
                
                if (!bot1.simultaneousMode && bot1.explosionActive && bot1.bombList && 
                    bot1.currentExplosionIndex < bot1.bombCount) {
                    if (bot1.bombList[bot1.currentExplosionIndex].row == i && 
                        bot1.bombList[bot1.currentExplosionIndex].col == j && 
                        bot1.explosionFrame > 0) {
                        drawExplosionBackground(px, py, displayDiff.cellSize, bot1.explosionFrame);
                    }
                }
                
                bool isExploded = (i == bot1.explodedPosition.row && j == bot1.explodedPosition.col);
                drawCell(&bot1Board, i, j, px, py, displayDiff.cellSize, isExploded);
            }
        }
        DrawRectangleLines(leftX - 1, yStart - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                         displayDiff.cellSize * displayDiff.rows + 2, COLOR_BORDER_DARK);
        
        if (bot1.win) {
            DrawRectangle(leftX - 1, yStart - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                        displayDiff.cellSize * displayDiff.rows + 2, (Color){0, 0, 0, 180});
            int tw2 = MeasureText("WIN", winFontSize);
            DrawText("WIN", leftX + displayDiff.cellSize * displayDiff.cols / 2 - tw2 / 2, 
                    yStart + displayDiff.cellSize * displayDiff.rows / 2 - winFontSize / 2, winFontSize, GREEN);
        } else if (bot1.lose) {
            DrawRectangle(leftX - 1, yStart - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                        displayDiff.cellSize * displayDiff.rows + 2, (Color){0, 0, 0, 180});
            int tw2 = MeasureText("GAME OVER", gameOverFontSize);
            DrawText("GAME OVER", leftX + displayDiff.cellSize * displayDiff.cols / 2 - tw2 / 2, 
                    yStart + displayDiff.cellSize * displayDiff.rows / 2 - gameOverFontSize / 2, gameOverFontSize, RED);
        }
        
        DrawText("ADVANCED BOT", rightX + (displayDiff.cellSize * displayDiff.cols - MeasureText("ADVANCED BOT", titleFontSize)) / 2, 10, titleFontSize, DARKBLUE);
        
        char bombs2[32];
        sprintf(bombs2, "Bombs: %d", bot2Board.totalMines - bot2Board.flagsPlaced);
        int bw2 = MeasureText(bombs2, infoFontSize);
        DrawText(bombs2, rightX + (displayDiff.cellSize * displayDiff.cols - bw2) / 2, 35, infoFontSize, BLACK);
        
        char time2[32];
        sprintf(time2, "Time: %d", bot2.elapsedSeconds);
        int tw2 = MeasureText(time2, infoFontSize);
        DrawText(time2, rightX + (displayDiff.cellSize * displayDiff.cols - tw2) / 2, 65, infoFontSize, BLACK);
        
        for (int i = 0; i < displayDiff.rows; i++) {
            for (int j = 0; j < displayDiff.cols; j++) {
                int px = rightX + j * displayDiff.cellSize;
                int py = yStart + i * displayDiff.cellSize;
                
                if (!bot2.simultaneousMode && bot2.explosionActive && bot2.bombList && 
                    bot2.currentExplosionIndex < bot2.bombCount) {
                    if (bot2.bombList[bot2.currentExplosionIndex].row == i && 
                        bot2.bombList[bot2.currentExplosionIndex].col == j && 
                        bot2.explosionFrame > 0) {
                        drawExplosionBackground(px, py, displayDiff.cellSize, bot2.explosionFrame);
                    }
                }
                
                bool isExploded = (i == bot2.explodedPosition.row && j == bot2.explodedPosition.col);
                drawCell(&bot2Board, i, j, px, py, displayDiff.cellSize, isExploded);
            }
        }
        DrawRectangleLines(rightX - 1, yStart - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                         displayDiff.cellSize * displayDiff.rows + 2, COLOR_BORDER_DARK);
        
        if (bot2.win) {
            DrawRectangle(rightX - 1, yStart - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                        displayDiff.cellSize * displayDiff.rows + 2, (Color){0, 0, 0, 180});
            int tw3 = MeasureText("WIN", winFontSize);
            DrawText("WIN", rightX + displayDiff.cellSize * displayDiff.cols / 2 - tw3 / 2, 
                    yStart + displayDiff.cellSize * displayDiff.rows / 2 - winFontSize / 2, winFontSize, GREEN);
        } else if (bot2.lose) {
            DrawRectangle(rightX - 1, yStart - 1, displayDiff.cellSize * displayDiff.cols + 2, 
                        displayDiff.cellSize * displayDiff.rows + 2, (Color){0, 0, 0, 180});
            int tw3 = MeasureText("GAME OVER", gameOverFontSize);
            DrawText("GAME OVER", rightX + displayDiff.cellSize * displayDiff.cols / 2 - tw3 / 2, 
                    yStart + displayDiff.cellSize * displayDiff.rows / 2 - gameOverFontSize / 2, gameOverFontSize, RED);
        }
        
        if (bothFinished) {
            const char* winnerText = "";
            Color winColor = YELLOW;
            
            if (bot1.win && !bot2.win) {
                winnerText = "SIMPLE BOT WINS!";
                winColor = GREEN;
            } else if (!bot1.win && bot2.win) {
                winnerText = "ADVANCED BOT WINS!";
                winColor = GREEN;
            } else if (bot1.win && bot2.win) {
                if (bot1.elapsedSeconds < bot2.elapsedSeconds) {
                    winnerText = "SIMPLE BOT WINS!";
                    winColor = GREEN;
                } else if (bot2.elapsedSeconds < bot1.elapsedSeconds) {
                    winnerText = "ADVANCED BOT WINS!";
                    winColor = GREEN;
                } else {
                    winnerText = "DRAW!";
                    winColor = YELLOW;
                }
            } else {
                int safe1 = getSafeRevealedCount(&bot1Board);
                int safe2 = getSafeRevealedCount(&bot2Board);
                if (safe1 > safe2) {
                    winnerText = "SIMPLE BOT WINS!";
                    winColor = GREEN;
                } else if (safe2 > safe1) {
                    winnerText = "ADVANCED BOT WINS!";
                    winColor = GREEN;
                } else {
                    winnerText = "DRAW!";
                    winColor = YELLOW;
                }
            }
            
            DrawRectangle(0, 0, windowWidth, windowHeight, (Color){0, 0, 0, 200});
            int tw = MeasureText(winnerText, finalFontSize);
            DrawText(winnerText, (windowWidth - tw) / 2, windowHeight / 2 - finalFontSize / 2, finalFontSize, winColor);
            
            int exitFontSize = 24;
            DrawText("Press MENU to exit", (windowWidth - MeasureText("Press MENU to exit", exitFontSize)) / 2, 
                    windowHeight / 2 + 60, exitFontSize, WHITE);
        }
        
        EndDrawing();
    }
    
    cleanupGameBoard(&tempBoard);
    cleanupGameBoard(&bot1Board);
    cleanupGameBoard(&bot2Board);
    cleanupBotGameState(&bot1);
    cleanupBotGameState(&bot2);
    CloseWindow();
}