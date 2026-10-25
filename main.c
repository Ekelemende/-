#include "raylib.h"
#include "game.h"
#include "graphics.h"
#include "records.h"
#include "bot.h"
#include "dual_bot.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void runGame(Difficulty diff, HighScores* scores, int difficultyId, GameMode mode);

int main(void) {
    srand(time(NULL));
    
    bool running = true;
    HighScores scores;
    loadHighScores(&scores);
    
    while (running) {
        int mw = 800, mh = 650;
        InitWindow(mw, mh, "Minesweeper");
        SetTargetFPS(60);
        
        int btnW = 240, btnH = 55, btnX = mw / 2 - btnW / 2;
        Rectangle novice = {btnX, 150, btnW, btnH};
        Rectangle amateur = {btnX, 225, btnW, btnH};
        Rectangle expert = {btnX, 300, btnW, btnH};
        Rectangle stats = {btnX, 375, btnW, btnH};
        Rectangle botModeBtn = {btnX, 450, btnW, btnH};
        
        int infoRad = 30, infoX = 25 + infoRad, infoY = mh - infoRad - 25;
        int aboutRad = 30, aboutX = mw - aboutRad - 25, aboutY = mh - aboutRad - 25;
        
        bool gameStarted = false;
        Difficulty selectedDiff;
        int selectedDiffId = -1;
        GameMode selectedMode = MODE_HUMAN;
        bool showAbout = false, showInfo = false;
        bool showBotMenu = false;
        int botMenuDiff = -1;
        int botMenuBot = -1;
        
        int aboutW = 650, aboutH = 520;
        Rectangle aboutRect = {mw / 2 - aboutW / 2, mh / 2 - aboutH / 2, aboutW, aboutH};
        Rectangle aboutClose = {aboutRect.x + aboutRect.width / 2 - 100, aboutRect.y + aboutRect.height - 65, 200, 45};
        
        int infoW = 700, infoH = 580;
        Rectangle infoRect = {mw / 2 - infoW / 2, mh / 2 - infoH / 2, infoW, infoH};
        Rectangle infoClose = {infoRect.x + infoRect.width / 2 - 100, infoRect.y + infoRect.height - 65, 200, 45};
        
        while (!WindowShouldClose() && !gameStarted) {
            Vector2 mp = GetMousePosition();
            
            if (showBotMenu) {
                int bw = 750, bh = 550;
                int bx = mw / 2 - bw / 2, by = mh / 2 - bh / 2;
                
                Rectangle diffBtns[3] = {
                    {bx + 50, by + 130, 280, 55},
                    {bx + 50, by + 200, 280, 55},
                    {bx + 50, by + 270, 280, 55}
                };
                Rectangle typeBtns[3] = {
                    {bx + 420, by + 130, 280, 55},
                    {bx + 420, by + 200, 280, 55},
                    {bx + 420, by + 270, 280, 55}
                };
                Rectangle startBtn = {bx + bw / 2 - 120, by + 360, 240, 50};
                Rectangle backBtn = {bx + bw / 2 - 120, by + 430, 240, 50};
                
                bool hDiff[3] = {CheckCollisionPointRec(mp, diffBtns[0]), 
                                CheckCollisionPointRec(mp, diffBtns[1]), 
                                CheckCollisionPointRec(mp, diffBtns[2])};
                bool hType[3] = {CheckCollisionPointRec(mp, typeBtns[0]), 
                                CheckCollisionPointRec(mp, typeBtns[1]), 
                                CheckCollisionPointRec(mp, typeBtns[2])};
                bool hStart = CheckCollisionPointRec(mp, startBtn);
                bool hBack = CheckCollisionPointRec(mp, backBtn);
                bool canStart = (botMenuDiff != -1 && botMenuBot != -1);
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (hBack) {
                        showBotMenu = false;
                    } else if (hStart && canStart) {
                        switch (botMenuDiff) {
                            case 0:
                                initDifficulty(&selectedDiff, 9, 9, 10, "Beginner", 70, 0);
                                selectedDiffId = 0;
                                break;
                            case 1:
                                initDifficulty(&selectedDiff, 16, 16, 40, "Intermediate", 40, 1);
                                selectedDiffId = 1;
                                break;
                            case 2:
                                initDifficulty(&selectedDiff, 16, 30, 99, "Expert", 40, 2);
                                selectedDiffId = 2;
                                break;
                        }
                        selectedMode = (botMenuBot == 2) ? MODE_DUAL_BOT : 
                                      ((botMenuBot == 0) ? MODE_BOT_SIMPLE : MODE_BOT_HARD);
                        gameStarted = true;
                    } else {
                        if (hDiff[0]) botMenuDiff = 0;
                        else if (hDiff[1]) botMenuDiff = 1;
                        else if (hDiff[2]) botMenuDiff = 2;
                        else if (hType[0]) botMenuBot = 0;
                        else if (hType[1]) botMenuBot = 1;
                        else if (hType[2]) botMenuBot = 2;
                    }
                }
                
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawRectangle(0, 0, mw, mh, (Color){0, 0, 0, 150});
                DrawRectangle(bx, by, bw, bh, RAYWHITE);
                DrawRectangleLines(bx, by, bw, bh, DARKGRAY);
                DrawText("BOT MODE SETTINGS", bx + bw / 2 - MeasureText("BOT MODE SETTINGS", 28) / 2, by + 30, 28, DARKBLUE);
                DrawText("DIFFICULTY", bx + 125, by + 100, 22, DARKGRAY);
                DrawText("BOT TYPE", bx + 500, by + 100, 22, DARKGRAY);
                
                const char* diffNames[] = {"BEGINNER", "INTERMEDIATE", "EXPERT"};
                for (int i = 0; i < 3; i++) {
                    DrawRectangleRec(diffBtns[i], hDiff[i] ? LIGHTGRAY : GRAY);
                    DrawRectangleLines(diffBtns[i].x, diffBtns[i].y, diffBtns[i].width, diffBtns[i].height, BLACK);
                    int txtW = MeasureText(diffNames[i], 22);
                    DrawText(diffNames[i], diffBtns[i].x + diffBtns[i].width / 2 - txtW / 2, 
                            diffBtns[i].y + 16, 22, BLACK);
                    if (botMenuDiff == i) drawFlag(diffBtns[i].x, diffBtns[i].y, 50);
                }
                
                const char* typeNames[] = {"SIMPLE BOT", "ADVANCED BOT", "BOT vs BOT"};
                for (int i = 0; i < 3; i++) {
                    DrawRectangleRec(typeBtns[i], hType[i] ? LIGHTGRAY : GRAY);
                    DrawRectangleLines(typeBtns[i].x, typeBtns[i].y, typeBtns[i].width, typeBtns[i].height, BLACK);
                    int txtW = MeasureText(typeNames[i], 22);
                    DrawText(typeNames[i], typeBtns[i].x + typeBtns[i].width / 2 - txtW / 2, 
                            typeBtns[i].y + 16, 22, BLACK);
                    if (botMenuBot == i) drawFlag(typeBtns[i].x, typeBtns[i].y, 50);
                }
                
                Color startCol = canStart ? (hStart ? LIGHTGRAY : GREEN) : (Color){120, 120, 120, 150};
                DrawRectangleRec(startBtn, startCol);
                DrawRectangleLines(startBtn.x, startBtn.y, startBtn.width, startBtn.height, BLACK);
                DrawText("START GAME", startBtn.x + startBtn.width / 2 - MeasureText("START GAME", 24) / 2, 
                        startBtn.y + 13, 24, BLACK);
                
                DrawRectangleRec(backBtn, hBack ? LIGHTGRAY : GRAY);
                DrawRectangleLines(backBtn.x, backBtn.y, backBtn.width, backBtn.height, BLACK);
                DrawText("MENU", backBtn.x + backBtn.width / 2 - MeasureText("MENU", 24) / 2, 
                        backBtn.y + 13, 24, BLACK);
                EndDrawing();
            }
            else if (showAbout) {
                bool hAboutClose = CheckCollisionPointRec(mp, aboutClose);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hAboutClose) showAbout = false;
                
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawRectangle(0, 0, mw, mh, (Color){0, 0, 0, 180});
                DrawRectangleRec(aboutRect, RAYWHITE);
                DrawRectangleLines(aboutRect.x, aboutRect.y, aboutRect.width, aboutRect.height, DARKGRAY);
                DrawText("ABOUT AUTHORS", aboutRect.x + aboutRect.width / 2 - MeasureText("ABOUT AUTHORS", 34) / 2, 
                        aboutRect.y + 30, 34, DARKBLUE);
                DrawLine(aboutRect.x + 40, aboutRect.y + 80, aboutRect.x + aboutRect.width - 40, aboutRect.y + 80, GRAY);
                
                int yoff = aboutRect.y + 115;
                DrawText("Authors:", aboutRect.x + 40, yoff, 26, DARKGRAY);
                yoff += 42;
                DrawText("Students of SPbPU Peter the Great,", aboutRect.x + 60, yoff, 20, BLACK);
                yoff += 28;
                DrawText("Institute of Computer Science and Cybersecurity,", aboutRect.x + 60, yoff, 20, BLACK);
                yoff += 28;
                DrawText("Higher School of Cybersecurity,", aboutRect.x + 60, yoff, 20, BLACK);
                yoff += 28;
                DrawText("Group 5131001/50601:", aboutRect.x + 60, yoff, 20, BLACK);
                yoff += 40;
                DrawText("Moloshonok Andrey Evgenievich", aboutRect.x + 80, yoff, 22, DARKGREEN);
                yoff += 28;
                DrawText("Pshenko Vsevolod Andreevich", aboutRect.x + 80, yoff, 22, DARKGREEN);
                yoff += 40;
                DrawText("Year 2026", aboutRect.x + 40, yoff, 24, DARKGRAY);
                
                DrawRectangleRec(aboutClose, hAboutClose ? LIGHTGRAY : GRAY);
                DrawRectangleLines(aboutClose.x, aboutClose.y, aboutClose.width, aboutClose.height, BLACK);
                DrawText("CLOSE", aboutClose.x + aboutClose.width / 2 - MeasureText("CLOSE", 24) / 2, 
                        aboutClose.y + 12, 24, BLACK);
                EndDrawing();
            }
            else if (showInfo) {
                bool hInfoClose = CheckCollisionPointRec(mp, infoClose);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hInfoClose) showInfo = false;
                
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawRectangle(0, 0, mw, mh, (Color){0, 0, 0, 180});
                DrawRectangleRec(infoRect, RAYWHITE);
                DrawRectangleLines(infoRect.x, infoRect.y, infoRect.width, infoRect.height, DARKGRAY);
                DrawText("HOW TO PLAY", infoRect.x + infoRect.width / 2 - MeasureText("HOW TO PLAY", 32) / 2, 
                        infoRect.y + 25, 32, DARKBLUE);
                DrawLine(infoRect.x + 40, infoRect.y + 70, infoRect.x + infoRect.width - 40, infoRect.y + 70, GRAY);
                
                int yoff = infoRect.y + 100;
                DrawText("Controls:", infoRect.x + 40, yoff, 24, DARKGRAY);
                yoff += 36;
                DrawText("Left Mouse Button  -  Open a cell", infoRect.x + 60, yoff, 18, BLACK);
                yoff += 28;
                DrawText("Right Mouse Button  -  Place/Remove flag", infoRect.x + 60, yoff, 18, BLACK);
                yoff += 28;
                DrawText("MENU Button        -  Restart game", infoRect.x + 60, yoff, 18, BLACK);
                yoff += 42;
                
                DrawText("Rules:", infoRect.x + 40, yoff, 24, DARKGRAY);
                yoff += 36;
                DrawText("Open all cells without mines to win.", infoRect.x + 60, yoff, 18, BLACK);
                yoff += 28;
                DrawText("Use flags to mark suspected mines.", infoRect.x + 60, yoff, 18, BLACK);
                yoff += 28;
                DrawText("The number on a cell shows how many mines", infoRect.x + 60, yoff, 18, BLACK);
                yoff += 28;
                DrawText("are adjacent to it.", infoRect.x + 60, yoff, 18, BLACK);
                yoff += 42;
                DrawText("If you open a mine, you lose!", infoRect.x + 60, yoff, 18, RED);
                yoff += 28;
                DrawText("If you flag all mines and open all safe cells, you win!", 
                        infoRect.x + 60, yoff, 18, DARKGREEN);
                
                DrawRectangleRec(infoClose, hInfoClose ? LIGHTGRAY : GRAY);
                DrawRectangleLines(infoClose.x, infoClose.y, infoClose.width, infoClose.height, BLACK);
                DrawText("CLOSE", infoClose.x + infoClose.width / 2 - MeasureText("CLOSE", 22) / 2, 
                        infoClose.y + 12, 22, BLACK);
                EndDrawing();
            }
            else {
                bool hNovice = CheckCollisionPointRec(mp, novice);
                bool hAmateur = CheckCollisionPointRec(mp, amateur);
                bool hExpert = CheckCollisionPointRec(mp, expert);
                bool hStats = CheckCollisionPointRec(mp, stats);
                bool hBotMode = CheckCollisionPointRec(mp, botModeBtn);
                bool hInfo = (sqrt(pow(mp.x - infoX, 2) + pow(mp.y - infoY, 2)) <= infoRad);
                bool hAbout = (sqrt(pow(mp.x - aboutX, 2) + pow(mp.y - aboutY, 2)) <= aboutRad);
                
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (hNovice) {
                        initDifficulty(&selectedDiff, 9, 9, 10, "Beginner", 70, 0);
                        selectedDiffId = 0;
                        selectedMode = MODE_HUMAN;
                        gameStarted = true;
                    } else if (hAmateur) {
                        initDifficulty(&selectedDiff, 16, 16, 40, "Intermediate", 40, 1);
                        selectedDiffId = 1;
                        selectedMode = MODE_HUMAN;
                        gameStarted = true;
                    } else if (hExpert) {
                        initDifficulty(&selectedDiff, 16, 30, 99, "Expert", 40, 2);
                        selectedDiffId = 2;
                        selectedMode = MODE_HUMAN;
                        gameStarted = true;
                    } else if (hStats) {
                        showHighScoresWindow(&scores, mw, mh);
                    } else if (hBotMode) {
                        showBotMenu = true;
                        botMenuDiff = -1;
                        botMenuBot = -1;
                    } else if (hInfo) {
                        showInfo = true;
                    } else if (hAbout) {
                        showAbout = true;
                    }
                }
                
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawText("MINESWEEPER", mw / 2 - MeasureText("MINESWEEPER", 50) / 2, 50, 50, DARKGRAY);
                DrawText("Select Difficulty", mw / 2 - MeasureText("Select Difficulty", 30) / 2, 110, 30, GRAY);
                
                DrawRectangleRec(novice, hNovice ? LIGHTGRAY : GRAY);
                DrawRectangleLines(novice.x, novice.y, novice.width, novice.height, BLACK);
                DrawText("BEGINNER", novice.x + novice.width / 2 - MeasureText("BEGINNER", 28) / 2, 
                        novice.y + 18, 28, BLACK);
                
                DrawRectangleRec(amateur, hAmateur ? LIGHTGRAY : GRAY);
                DrawRectangleLines(amateur.x, amateur.y, amateur.width, amateur.height, BLACK);
                DrawText("INTERMEDIATE", amateur.x + amateur.width / 2 - MeasureText("INTERMEDIATE", 28) / 2, 
                        amateur.y + 18, 28, BLACK);
                
                DrawRectangleRec(expert, hExpert ? LIGHTGRAY : GRAY);
                DrawRectangleLines(expert.x, expert.y, expert.width, expert.height, BLACK);
                DrawText("EXPERT", expert.x + expert.width / 2 - MeasureText("EXPERT", 28) / 2, 
                        expert.y + 18, 28, BLACK);
                
                DrawRectangleRec(stats, hStats ? LIGHTGRAY : GRAY);
                DrawRectangleLines(stats.x, stats.y, stats.width, stats.height, BLACK);
                DrawText("STATISTICS", stats.x + stats.width / 2 - MeasureText("STATISTICS", 28) / 2, 
                        stats.y + 18, 28, BLACK);
                
                DrawRectangleRec(botModeBtn, hBotMode ? LIGHTGRAY : GRAY);
                DrawRectangleLines(botModeBtn.x, botModeBtn.y, botModeBtn.width, botModeBtn.height, BLACK);
                DrawText("BOT MODE", botModeBtn.x + botModeBtn.width / 2 - MeasureText("BOT MODE", 28) / 2, 
                        botModeBtn.y + 18, 28, BLACK);
                
                drawInfoButton(infoX, infoY, infoRad, hInfo);
                drawAboutButton(aboutX, aboutY, aboutRad, hAbout);
                EndDrawing();
            }
        }
        
        CloseWindow();
        
        if (gameStarted) {
            runGame(selectedDiff, &scores, selectedDiffId, selectedMode);
        } else {
            running = false;
        }
    }
    
    return 0;
}

// ============================================================
// ПОЛНАЯ РЕАЛИЗАЦИЯ runGame
// ============================================================
void runGame(Difficulty diff, HighScores* scores, int difficultyId, GameMode mode) {
    // Для режима DUAL BOT используем отдельную функцию
    if (mode == MODE_DUAL_BOT) {
        runDualBotGame(diff, scores, difficultyId);
        return;
    }
    
    // === ОДИНОЧНЫЙ РЕЖИМ (HUMAN, BOT_SIMPLE, BOT_HARD) ===
    GameBoard board;
    if (!initGameBoard(&board, diff.rows, diff.cols, diff.bombs, diff.difficulty)) {
        return;
    }
    
    InitWindow(diff.windowWidth, diff.windowHeight, TextFormat("Minesweeper - %s", diff.name));
    SetTargetFPS(60);
    
    bool gameOver = false;
    bool win = false;
    Position explodedPosition = {-1, -1};
    bool explosionActive = false;
    bool simultaneousMode = false;
    int currentExplosionIndex = 0;
    int explosionFrame = 0;
    Position* bombList = NULL;
    int bombCount = 0;
    
    double startTime = GetTime();
    int elapsedSeconds = 0;
    bool timerRunning = false;
    
    int btnW = 150, btnH = 45;
    int btnX = (diff.windowWidth - btnW) / 2;
    Rectangle restartBtn = {btnX, 25, btnW, btnH};
    
    double lastBotMove = 0;
    const double BOT_DELAY = 0.3;
    
    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        bool hoverRestart = CheckCollisionPointRec(mouse, restartBtn);
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverRestart) {
            break;
        }
        
        if (!gameOver && !win) {
            if (mode == MODE_HUMAN) {
                // Логика человеческого игрока
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (mouse.y >= diff.headerHeight && mouse.y < diff.windowHeight - diff.borderSize) {
                        int col = (mouse.x - diff.borderSize) / diff.cellSize;
                        int row = (mouse.y - diff.headerHeight) / diff.cellSize;
                        
                        if (row >= 0 && row < diff.rows && col >= 0 && col < diff.cols) {
                            Cell* cell = getCell(&board, row, col);
                            if (cell && !cell->isRevealed && !cell->hasFlag) {
                                if (board.firstMove) {
                                    placeMines(&board, row, col);
                                    board.firstMove = false;
                                    startTime = GetTime();
                                    timerRunning = true;
                                }
                                revealCell(&board, row, col);
                                
                                if (cell->isMine) {
                                    gameOver = true;
                                    timerRunning = false;
                                    explodedPosition.row = row;
                                    explodedPosition.col = col;
                                    
                                    collectMinesList(&board, &bombList, &bombCount, explodedPosition);
                                    
                                    explosionActive = true;
                                    currentExplosionIndex = 0;
                                    explosionFrame = 0;
                                    simultaneousMode = false;
                                    
                                    for (int i = 0; i < board.rows; i++) {
                                        for (int j = 0; j < board.cols; j++) {
                                            if (getCell(&board, i, j)->isMine) {
                                                getCell(&board, i, j)->isRevealed = true;
                                            }
                                        }
                                    }
                                } else if (checkVictory(&board)) {
                                    win = true;
                                    timerRunning = false;
                                    addRecord(scores, difficultyId, elapsedSeconds);
                                }
                            }
                        }
                    }
                }
                
                if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                    if (mouse.y >= diff.headerHeight && mouse.y < diff.windowHeight - diff.borderSize) {
                        int col = (mouse.x - diff.borderSize) / diff.cellSize;
                        int row = (mouse.y - diff.headerHeight) / diff.cellSize;
                        
                        if (row >= 0 && row < diff.rows && col >= 0 && col < diff.cols) {
                            Cell* cell = getCell(&board, row, col);
                            if (cell && !cell->isRevealed) {
                                cell->hasFlag = !cell->hasFlag;
                                board.flagsPlaced += cell->hasFlag ? 1 : -1;
                                
                                if (checkVictory(&board)) {
                                    win = true;
                                    timerRunning = false;
                                    addRecord(scores, difficultyId, elapsedSeconds);
                                }
                            }
                        }
                    }
                }
            } else {
                // Режим ботов
                double now = GetTime();
                if (now - lastBotMove >= BOT_DELAY) {
                    bool moveResult = (mode == MODE_BOT_SIMPLE) ? botMakeMoveSimple(&board) : botMakeMoveHard(&board);
                    if (moveResult) {
                        lastBotMove = now;
                        if (!timerRunning && !board.firstMove) {
                            timerRunning = true;
                            startTime = GetTime();
                        }
                        
                        bool mineOpened = false;
                        for (int i = 0; i < board.rows && !mineOpened; i++) {
                            for (int j = 0; j < board.cols; j++) {
                                Cell* c = getCell(&board, i, j);
                                if (c->isMine && c->isRevealed) {
                                    mineOpened = true;
                                    explodedPosition.row = i;
                                    explodedPosition.col = j;
                                    break;
                                }
                            }
                        }
                        
                        if (mineOpened) {
                            gameOver = true;
                            timerRunning = false;
                            
                            collectMinesList(&board, &bombList, &bombCount, explodedPosition);
                            
                            explosionActive = true;
                            currentExplosionIndex = 0;
                            explosionFrame = 0;
                            simultaneousMode = false;
                            
                            for (int i = 0; i < board.rows; i++) {
                                for (int j = 0; j < board.cols; j++) {
                                    if (getCell(&board, i, j)->isMine) {
                                        getCell(&board, i, j)->isRevealed = true;
                                    }
                                }
                            }
                        } else if (checkVictory(&board)) {
                            win = true;
                            timerRunning = false;
                        }
                    }
                }
            }
        }
        
        if (timerRunning && !gameOver && !win) {
            elapsedSeconds = (int)(GetTime() - startTime);
        }
        
        // Анимация взрыва
        if (explosionActive && bombList) {
            if (simultaneousMode) {
                if (explosionFrame < EXPLOSION_FRAMES) {
                    explosionFrame++;
                } else {
                    for (int i = currentExplosionIndex; i < bombCount; i++) {
                        Cell* c = getCell(&board, bombList[i].row, bombList[i].col);
                        if (c) {
                            c->hasExploded = true;
                            c->isAnimating = false;
                        }
                    }
                    explosionActive = false;
                }
            } else {
                if (explosionFrame < EXPLOSION_FRAMES) {
                    explosionFrame++;
                } else {
                    if (currentExplosionIndex < bombCount) {
                        Cell* c = getCell(&board, bombList[currentExplosionIndex].row, bombList[currentExplosionIndex].col);
                        if (c) {
                            c->hasExploded = true;
                            c->isAnimating = false;
                        }
                    }
                    currentExplosionIndex++;
                    explosionFrame = 0;
                    
                    if (currentExplosionIndex >= SEQUENTIAL_EXPLOSIONS || currentExplosionIndex >= bombCount) {
                        if (currentExplosionIndex < bombCount) {
                            simultaneousMode = true;
                            for (int i = currentExplosionIndex; i < bombCount; i++) {
                                Cell* c = getCell(&board, bombList[i].row, bombList[i].col);
                                if (c) c->isAnimating = true;
                            }
                        } else {
                            explosionActive = false;
                        }
                    }
                }
            }
        }
        
        BeginDrawing();
        ClearBackground(COLOR_HEADER);
        
        DrawRectangle(0, 0, diff.windowWidth, diff.headerHeight, COLOR_HEADER);
        DrawRectangleLines(0, 0, diff.windowWidth, diff.headerHeight, COLOR_BORDER_DARK);
        drawRestartButton(btnX, 25, btnW, btnH, hoverRestart);
        
        char bombsText[32];
        sprintf(bombsText, "Bombs: %d", board.totalMines - board.flagsPlaced);
        DrawText(bombsText, 20, 35, 25, BLACK);
        
        char timerText[32];
        sprintf(timerText, "Time: %d", elapsedSeconds);
        int tw = MeasureText(timerText, 25);
        DrawText(timerText, diff.windowWidth - tw - 20, 35, 25, BLACK);
        
        for (int i = 0; i < diff.rows; i++) {
            for (int j = 0; j < diff.cols; j++) {
                int px = diff.borderSize + j * diff.cellSize;
                int py = diff.headerHeight + i * diff.cellSize;
                
                if (!simultaneousMode && explosionActive && bombList && currentExplosionIndex < bombCount) {
                    if (bombList[currentExplosionIndex].row == i && bombList[currentExplosionIndex].col == j && explosionFrame > 0) {
                        drawExplosionBackground(px, py, diff.cellSize, explosionFrame);
                    }
                }
                
                bool isExploded = (i == explodedPosition.row && j == explodedPosition.col);
                drawCell(&board, i, j, px, py, diff.cellSize, isExploded);
            }
        }
        
        DrawRectangleLines(diff.borderSize - 1, diff.headerHeight - 1, 
                         diff.cellSize * diff.cols + 2, diff.cellSize * diff.rows + 2, COLOR_BORDER_DARK);
        
        if (gameOver) {
            DrawRectangle(0, 0, diff.windowWidth, diff.windowHeight, (Color){0, 0, 0, 180});
            if (!explosionActive) {
                int tw2 = MeasureText("GAME OVER", 80);
                DrawText("GAME OVER", (diff.windowWidth - tw2) / 2, diff.windowHeight / 2 - 40, 80, RED);
            }
        } else if (win) {
            DrawRectangle(0, 0, diff.windowWidth, diff.windowHeight, (Color){0, 0, 0, 180});
            int tw2 = MeasureText("WIN", 100);
            DrawText("WIN", (diff.windowWidth - tw2) / 2, diff.windowHeight / 2 - 50, 100, GREEN);
            
            char timeStr[64];
            formatTime(elapsedSeconds, timeStr);
            char winText[128];
            sprintf(winText, "Time: %s", timeStr);
            int wt = MeasureText(winText, 30);
            DrawText(winText, (diff.windowWidth - wt) / 2, diff.windowHeight / 2 + 30, 30, YELLOW);
        }
        
        EndDrawing();
    }
    
    cleanupGameBoard(&board);
    if (bombList) free(bombList);
    CloseWindow();
}