#include "records.h"
#include "raylib.h"
#include "graphics.h"
#include <stdio.h>

void formatTime(int seconds, char* buffer) {
    if (!buffer) return;
    
    if (seconds < 60) {
        sprintf(buffer, "%d sec", seconds);
    } else {
        int minutes = seconds / 60;
        int secs = seconds % 60;
        sprintf(buffer, "%d min %d sec", minutes, secs);
    }
}

void loadHighScores(HighScores* scores) {
    if (!scores) return;
    
    FILE* f = fopen("records.txt", "r");
    scores->beginnerCount = scores->intermediateCount = scores->expertCount = 0;
    
    for (int i = 0; i < MAX_RECORDS; i++) {
        scores->beginner[i].seconds = 999999;
        scores->intermediate[i].seconds = 999999;
        scores->expert[i].seconds = 999999;
    }
    
    if (!f) return;
    
    fscanf(f, "BEGINNER:%d\n", &scores->beginnerCount);
    for (int i = 0; i < scores->beginnerCount && i < MAX_RECORDS; i++) {
        fscanf(f, "%d\n", &scores->beginner[i].seconds);
    }
    
    fscanf(f, "INTERMEDIATE:%d\n", &scores->intermediateCount);
    for (int i = 0; i < scores->intermediateCount && i < MAX_RECORDS; i++) {
        fscanf(f, "%d\n", &scores->intermediate[i].seconds);
    }
    
    fscanf(f, "EXPERT:%d\n", &scores->expertCount);
    for (int i = 0; i < scores->expertCount && i < MAX_RECORDS; i++) {
        fscanf(f, "%d\n", &scores->expert[i].seconds);
    }
    
    fclose(f);
}

void saveHighScores(const HighScores* scores) {
    if (!scores) return;
    
    FILE* f = fopen("records.txt", "w");
    if (!f) return;
    
    fprintf(f, "BEGINNER:%d\n", scores->beginnerCount);
    for (int i = 0; i < scores->beginnerCount && i < MAX_RECORDS; i++) {
        fprintf(f, "%d\n", scores->beginner[i].seconds);
    }
    
    fprintf(f, "INTERMEDIATE:%d\n", scores->intermediateCount);
    for (int i = 0; i < scores->intermediateCount && i < MAX_RECORDS; i++) {
        fprintf(f, "%d\n", scores->intermediate[i].seconds);
    }
    
    fprintf(f, "EXPERT:%d\n", scores->expertCount);
    for (int i = 0; i < scores->expertCount && i < MAX_RECORDS; i++) {
        fprintf(f, "%d\n", scores->expert[i].seconds);
    }
    
    fclose(f);
}

void addRecord(HighScores* scores, int difficulty, int gameTime) {
    if (!scores) return;
    
    Record* rec = NULL;
    int* cnt = NULL;
    
    switch (difficulty) {
        case 0:
            rec = scores->beginner;
            cnt = &scores->beginnerCount;
            break;
        case 1:
            rec = scores->intermediate;
            cnt = &scores->intermediateCount;
            break;
        case 2:
            rec = scores->expert;
            cnt = &scores->expertCount;
            break;
        default:
            return;
    }
    
    int pos = -1;
    int maxCmp = (*cnt < MAX_RECORDS) ? *cnt : MAX_RECORDS;
    for (int i = 0; i < maxCmp; i++) {
        if (gameTime < rec[i].seconds) {
            pos = i;
            break;
        }
    }
    
    if (pos == -1 && *cnt < MAX_RECORDS) {
        pos = *cnt;
    }
    
    if (pos != -1 && pos < MAX_RECORDS) {
        for (int i = (*cnt) - 1; i >= pos && i < MAX_RECORDS - 1; i--) {
            rec[i + 1].seconds = rec[i].seconds;
        }
        rec[pos].seconds = gameTime;
        if (*cnt < MAX_RECORDS) (*cnt)++;
        saveHighScores(scores);
    }
}

void showHighScoresWindow(const HighScores* scores, int mw, int mh) {
    if (!scores) return;
    
    bool open = true;
    int c1x = mw / 2 - 320, c2x = mw / 2 - 100, c3x = mw / 2 + 120;
    int sw = 700, sh = 550;
    Rectangle rect = {mw / 2 - sw / 2, mh / 2 - sh / 2, sw, sh};
    Rectangle closeBtn = {rect.x + rect.width / 2 - 100, rect.y + rect.height - 65, 200, 45};
    
    while (open && !WindowShouldClose()) {
        Vector2 mp = GetMousePosition();
        bool hoverClose = CheckCollisionPointRec(mp, closeBtn);
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverClose) {
            open = false;
        }
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawText("MINESWEEPER", mw / 2 - MeasureText("MINESWEEPER", 50) / 2, 50, 50, DARKGRAY);
        DrawText("Select Difficulty", mw / 2 - MeasureText("Select Difficulty", 30) / 2, 110, 30, GRAY);
        
        DrawRectangle(mw / 2 - 120, 150, 240, 55, GRAY);
        DrawRectangleLines(mw / 2 - 120, 150, 240, 55, BLACK);
        DrawText("BEGINNER", mw / 2 - MeasureText("BEGINNER", 28) / 2, 168, 28, BLACK);
        
        DrawRectangle(mw / 2 - 120, 225, 240, 55, GRAY);
        DrawRectangleLines(mw / 2 - 120, 225, 240, 55, BLACK);
        DrawText("INTERMEDIATE", mw / 2 - MeasureText("INTERMEDIATE", 28) / 2, 243, 28, BLACK);
        
        DrawRectangle(mw / 2 - 120, 300, 240, 55, GRAY);
        DrawRectangleLines(mw / 2 - 120, 300, 240, 55, BLACK);
        DrawText("EXPERT", mw / 2 - MeasureText("EXPERT", 28) / 2, 318, 28, BLACK);
        
        DrawRectangle(mw / 2 - 120, 375, 240, 55, GRAY);
        DrawRectangleLines(mw / 2 - 120, 375, 240, 55, BLACK);
        DrawText("STATISTICS", mw / 2 - MeasureText("STATISTICS", 28) / 2, 393, 28, BLACK);
        
        drawInfoButton(25 + 30, mh - 30 - 25, 30, false);
        drawAboutButton(mw - 30 - 25, mh - 30 - 25, 30, false);
        
        DrawRectangle(0, 0, mw, mh, (Color){0, 0, 0, 200});
        DrawRectangleRec(rect, RAYWHITE);
        DrawRectangleLines(rect.x, rect.y, rect.width, rect.height, DARKGRAY);
        
        DrawText("HIGH SCORES", rect.x + rect.width / 2 - MeasureText("HIGH SCORES", 36) / 2, rect.y + 20, 36, DARKBLUE);
        DrawLine(rect.x + 40, rect.y + 70, rect.x + rect.width - 40, rect.y + 70, GRAY);
        
        DrawText("BEGINNER", c1x + 20, rect.y + 95, 20, DARKBLUE);
        DrawText("INTERMEDIATE", c2x + 10, rect.y + 95, 20, DARKBLUE);
        DrawText("EXPERT", c3x + 30, rect.y + 95, 20, DARKBLUE);
        
        int yOff = rect.y + 140;
        
        for (int i = 0; i < MAX_RECORDS; i++) {
            char buf[80];
            char timeStr[50];
            
            if (i < scores->beginnerCount) {
                formatTime(scores->beginner[i].seconds, timeStr);
                sprintf(buf, "%d. %s", i + 1, timeStr);
                DrawText(buf, c1x, yOff + i * 25, 18, BLACK);
            } else {
                sprintf(buf, "%d. ---", i + 1);
                DrawText(buf, c1x, yOff + i * 25, 18, GRAY);
            }
            
            if (i < scores->intermediateCount) {
                formatTime(scores->intermediate[i].seconds, timeStr);
                sprintf(buf, "%d. %s", i + 1, timeStr);
                DrawText(buf, c2x, yOff + i * 25, 18, BLACK);
            } else {
                sprintf(buf, "%d. ---", i + 1);
                DrawText(buf, c2x, yOff + i * 25, 18, GRAY);
            }
            
            if (i < scores->expertCount) {
                formatTime(scores->expert[i].seconds, timeStr);
                sprintf(buf, "%d. %s", i + 1, timeStr);
                DrawText(buf, c3x, yOff + i * 25, 18, BLACK);
            } else {
                sprintf(buf, "%d. ---", i + 1);
                DrawText(buf, c3x, yOff + i * 25, 18, GRAY);
            }
        }
        
        DrawRectangleRec(closeBtn, hoverClose ? LIGHTGRAY : GRAY);
        DrawRectangleLines(closeBtn.x, closeBtn.y, closeBtn.width, closeBtn.height, BLACK);
        DrawText("CLOSE", closeBtn.x + closeBtn.width / 2 - MeasureText("CLOSE", 24) / 2, closeBtn.y + 12, 24, BLACK);
        
        EndDrawing();
    }
}