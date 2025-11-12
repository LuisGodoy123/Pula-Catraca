// Simple persistent ranking using a linked list
#ifndef RANKING_H
#define RANKING_H

#include <stddef.h>

#define RANKING_MAX 10

typedef struct RankingNode {
    char *name;
    float time; // segundos (quanto menor  tempo, melhor a posição no ranking)
    struct RankingNode *next;
} RankingNode;

typedef struct {
    RankingNode *head;
    int size;
} RankingList;

void initRanking(RankingList *r);
void freeRanking(RankingList *r);

// Insert a new entry sorted ascending by time
void insertRanking(RankingList *r, const char *name, float time);

// Persistence: load/save the full list (all entries) in CSV text format (name,time)
// and export the top N to a separate CSV file.
void loadRankingAll(RankingList *r, const char *filepath);
void saveRankingAll(RankingList *r, const char *filepath);
void saveTopCSV(RankingList *r, const char *filepath, int topN);

#endif