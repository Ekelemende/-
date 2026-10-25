#ifndef RECORDS_H
#define RECORDS_H

#define MAX_RECORDS 10

typedef struct {
    int seconds;
} Record;

typedef struct {
    Record beginner[MAX_RECORDS];
    Record intermediate[MAX_RECORDS];
    Record expert[MAX_RECORDS];
    int beginnerCount;
    int intermediateCount;
    int expertCount;
} HighScores;

void formatTime(int seconds, char* buffer);
void loadHighScores(HighScores* scores);
void saveHighScores(const HighScores* scores);
void addRecord(HighScores* scores, int difficulty, int gameTime);
void showHighScoresWindow(const HighScores* scores, int mw, int mh);

#endif // RECORDS_H