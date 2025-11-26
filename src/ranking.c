#include "../include/ranking.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char *duplicarString(const char *s) {
    if (!s) return NULL;
    size_t tamanho = strlen(s) + 1;
    char *p = (char*)malloc(tamanho);
    if (p) memcpy(p, s, tamanho);
    return p;
}

void initRanking(ListaRanking *r) {
    if (!r) return;
    r->head = NULL;
}

static void removerUltimoSeNecessario(ListaRanking *r) {
    if (!r) return;
    RankingNode *atual = r->head;
    RankingNode *anterior = NULL;
    while (atual && atual->next) {
        anterior = atual;
        atual = atual->next;
    }
    if (atual) {
        if (anterior) anterior->next = NULL;
        else r->head = NULL;
        free(atual->name);
        free(atual);
    }
}

void insertRanking(ListaRanking *r, const char *nome, float tempo) {
    if (!r || !nome) return;
    RankingNode *no = (RankingNode*)malloc(sizeof(RankingNode));
    if (!no) return;
    no->name = duplicarString(nome);
    no->time = tempo;
    no->next = NULL;

    // Insere de forma crescente
    if (!r->head || tempo < r->head->time) {
        no->next = r->head;
        r->head = no;
    } else {
        RankingNode *atual = r->head;
        while (atual->next && atual->next->time <= tempo) atual = atual->next;
        no->next = atual->next;
        atual->next = no;
    }
}

void salvarRankingCompleto(ListaRanking *r, const char *caminhoArquivo) {
    if (!r || !caminhoArquivo) return;
    FILE *arquivo = fopen(caminhoArquivo, "w");
    if (!arquivo) return;
    RankingNode *atual = r->head;
    while (atual) {
        fprintf(arquivo, "%s %.3f\n", atual->name ? atual->name : "", atual->time);
        atual = atual->next;
    }
    fclose(arquivo);
}

void salvarTopTXT(ListaRanking *r, const char *caminhoArquivo, int topN) {
    if (!r || !caminhoArquivo) return;
    FILE *arquivo = fopen(caminhoArquivo, "w");
    if (!arquivo) return;
    RankingNode *atual = r->head;
    int i = 0;
    while (atual && i < topN) {
        fprintf(arquivo, "%s %.3f\n", atual->name ? atual->name : "", atual->time);
        atual = atual->next; i++;
    }
    fclose(arquivo);
}

void carregarTodosRanking(ListaRanking *r, const char *caminhoArquivo) {
    if (!r || !caminhoArquivo) return;
    FILE *arquivo = fopen(caminhoArquivo, "r");
    if (!arquivo) return;
    char linha[512];
    while (fgets(linha, sizeof(linha), arquivo)) {
        char *novaLinha = strchr(linha, '\n'); if (novaLinha) *novaLinha = '\0';
        char *espaco = strchr(linha, ' ');
        if (!espaco) continue;
        *espaco = '\0';
        char *nome = linha;
        char *tempoString = espaco + 1;
        float tempo = (float)atof(tempoString);
        insertRanking(r, nome, tempo);
    }
    fclose(arquivo);
}

void freeRanking(ListaRanking *r) {
    if (!r) return;
    RankingNode *atual = r->head;
    while (atual) {
        RankingNode *proximo = atual->next;
        free(atual->name);
        free(atual);
        atual = proximo;
    }
    r->head = NULL;
}