// Sistema de ranking persistente usando lista encadeada
#ifndef RANKING_H
#define RANKING_H

#include <stddef.h>

#define RANKING_MAX 10

typedef struct RankingNode {
    char *name;
    float time; // segundos (quanto menor tempo, melhor a posição no ranking)
    struct RankingNode *next;
} RankingNode;

typedef struct {
    RankingNode *head;
    int size;
} RankingList;

void initRanking(RankingList *r);
void freeRanking(RankingList *r);

// Insere uma nova entrada ordenada de forma crescente pelo tempo (menor tempo = melhor posição)
void insertRanking(RankingList *r, const char *name, float time);

// Persistência: carrega/salva a lista completa (todas as entradas) em formato TXT (nome,tempo)
// e exporta o top N para um arquivo TXT separado
void loadRankingAll(RankingList *r, const char *filepath);
void saveRankingAll(RankingList *r, const char *filepath);
void saveTopTXT(RankingList *r, const char *filepath, int topN);

#endif