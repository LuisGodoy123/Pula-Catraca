#include "../include/ranking.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char *strdup_local(const char *s) {
    if (!s) return NULL;
    size_t L = strlen(s) + 1;
    char *p = (char*)malloc(L);
    if (p) memcpy(p, s, L);
    return p;
}

void initRanking(RankingList *r) {
    if (!r) return;
    r->head = NULL;
    r->size = 0;
}

static void removeTailIfNeeded(RankingList *r) {
    if (!r) return;
    if (r->size <= RANKING_MAX) return;
    RankingNode *cur = r->head;
    RankingNode *prev = NULL;
    while (cur && cur->next) {
        prev = cur;
        cur = cur->next;
    }
    if (cur) {
        if (prev) prev->next = NULL;
        else r->head = NULL;
        free(cur->name);
        free(cur);
        r->size--;
    }
}

void insertRanking(RankingList *r, const char *name, float time) {
    if (!r || !name) return;
    RankingNode *node = (RankingNode*)malloc(sizeof(RankingNode));
    if (!node) return;
    node->name = strdup_local(name);
    node->time = time;
    node->next = NULL;

    // Insert sorted ascending (lower time first)
    if (!r->head || time < r->head->time) {
        node->next = r->head;
        r->head = node;
    } else {
        RankingNode *cur = r->head;
        while (cur->next && cur->next->time <= time) cur = cur->next;
        node->next = cur->next;
        cur->next = node;
    }
    r->size++;
    removeTailIfNeeded(r);
}

void saveRankingAll(RankingList *r, const char *filepath) {
    if (!r || !filepath) return;
    FILE *f = fopen(filepath, "w");
    if (!f) return;
    RankingNode *cur = r->head;
    while (cur) {
        fprintf(f, "%s - %.3f", cur->name ? cur->name : "", cur->time);
        cur = cur->next;
    }
    fclose(f);
}

void saveTopCSV(RankingList *r, const char *filepath, int topN) {
    if (!r || !filepath) return;
    FILE *f = fopen(filepath, "w");
    if (!f) return;
    RankingNode *cur = r->head;
    int i = 0;
    while (cur && i < topN) {
        fprintf(f, "%s,%.3f", cur->name ? cur->name : "", cur->time);
        cur = cur->next; i++;
    }
    fclose(f);
}

void loadRankingAll(RankingList *r, const char *filepath) {
    if (!r || !filepath) return;
    FILE *f = fopen(filepath, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *nl = strchr(line, '\n'); if (nl) *nl = '\0';
        char *comma = strchr(line, ',');
        if (!comma) continue;
        *comma = '\0';
        char *name = line;
        char *time_s = comma + 1;
        float t = (float)atof(time_s);
        insertRanking(r, name, t);
    }
    fclose(f);
}

void freeRanking(RankingList *r) {
    if (!r) return;
    RankingNode *cur = r->head;
    while (cur) {
        RankingNode *next = cur->next;
        free(cur->name);
        free(cur);
        cur = next;
    }
    r->head = NULL;
    r->size = 0;
}