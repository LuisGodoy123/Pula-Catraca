#ifndef MECANICA_PRINCIPAL_H
#define MECANICA_PRINCIPAL_H

typedef struct {
    int x;
    int y;
    int lane; // 0 = esquerda, 1 = centro, 2 = direita
    int pulando;
    int abaixado;
    int velocidade_pulo;
    int tempo_abaixado; // contador para duração do abaixamento
    float pos_x_real; // posição real em pixels
    float pos_y_real; // posição real em pixels
} Jogador;

void inicializarJogador(Jogador *j, float pos_x_inicial, float pos_y_inicial);
void moverEsquerda(Jogador *j);
void moverDireita(Jogador *j);
void pular(Jogador *j);
void abaixar(Jogador *j);
void atualizarFisica(Jogador *j);

#endif
