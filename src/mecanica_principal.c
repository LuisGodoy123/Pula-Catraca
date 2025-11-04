#include "../include/mecanica_principal.h"

// Inicializa o jogador com posição customizável para Raylib
void inicializarJogador(Jogador *j, float pos_x_inicial, float pos_y_inicial) {
    j->lane = 1; // começa no centro
    j->pulando = 0;
    j->abaixado = 0;
    j->velocidade_pulo = 0;
    j->tempo_abaixado = 0;
    j->pos_x_real = pos_x_inicial;
    j->pos_y_real = pos_y_inicial;
}

void moverEsquerda(Jogador *j) {
    if (j->lane > 0) {
        j->lane--;
    }
}

void moverDireita(Jogador *j) {
    if (j->lane < 2) {
        j->lane++;
    }
}

void pular(Jogador *j) {
    if (!j->pulando && !j->abaixado) {
        j->pulando = 1;
        j->velocidade_pulo = 15; // velocidade inicial do pulo
    }
}

void abaixar(Jogador *j) {
    if (!j->pulando && !j->abaixado) {
        j->abaixado = 1;
        j->tempo_abaixado = 10; // duração do abaixamento (10 frames)
    }
}

void atualizarFisica(Jogador *j) {
    // Física do pulo (gravidade)
    if (j->pulando) {
        j->pos_y_real -= j->velocidade_pulo;
        j->velocidade_pulo -= 1; // gravidade
        
        // Volta ao chão
        if (j->pos_y_real >= j->pos_y_real + 100 || j->velocidade_pulo < -15) {
            j->pulando = 0;
            j->velocidade_pulo = 0;
            // Reseta posição Y ao chão
        }
    }
    
    // Atualiza estado de abaixado
    if (j->abaixado) {
        j->tempo_abaixado--;
        
        if (j->tempo_abaixado <= 0) {
            j->abaixado = 0;
        }
    }
}