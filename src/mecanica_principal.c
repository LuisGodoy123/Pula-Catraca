#include "../include/mecanica_principal.h"
#include <stdlib.h>
#include <time.h>

// Inicializa o jogador com posição customizável para Raylib
void inicializarJogador(Jogador *j, float pos_x_inicial, float pos_y_inicial) {
    j->lane = 1; // começa no centro
    j->pulando = 0;
    j->abaixado = 0;
    j->velocidade_pulo = 0;
    j->tempo_abaixado = 0;
    j->pos_x_real = pos_x_inicial;
    j->pos_y_real = pos_y_inicial;
    j->chao_y = pos_y_inicial;
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
        j->velocidade_pulo = 15; // Reduzido de 17 para 15 (mais 10% de redução)
    }
}

void abaixar(Jogador *j) {
    if (!j->pulando && !j->abaixado) {
        j->abaixado = 1;
        j->tempo_abaixado = 46; // Reduzido de 51 para 46 frames (mais 10% de redução = ~0.77 segundos)
    }
}

void atualizarFisica(Jogador *j) {
    // Física do pulo (gravidade)
    if (j->pulando) {
        j->pos_y_real -= j->velocidade_pulo;
        j->velocidade_pulo -= 1; // gravidade
        
        // Volta ao chão
        if (j->pos_y_real >= j->chao_y) {
            j->pos_y_real = j->chao_y;
            j->pulando = 0;
            j->velocidade_pulo = 0;
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

// ========== FUNÇÕES DE OBSTÁCULOS ==========

void inicializarObstaculos(Obstaculo obstaculos[], int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        obstaculos[i].ativo = 0;
        obstaculos[i].pos_x = 0;
        obstaculos[i].pos_y = 0;
        obstaculos[i].lane = 0;
        obstaculos[i].tipo = 0;
        obstaculos[i].largura = 0;
        obstaculos[i].altura = 0;
    }
    srand(time(NULL)); // Inicializa gerador aleatório
}

void criarObstaculo(Obstaculo obstaculos[], int tamanho, float screenHeight) {
    // Procura um slot inativo
    for (int i = 0; i < tamanho; i++) {
        if (!obstaculos[i].ativo) {
            obstaculos[i].ativo = 1;
            obstaculos[i].lane = rand() % 3; // Lane aleatória (0, 1 ou 2)
            obstaculos[i].pos_y = -100; // Começa acima da tela
            obstaculos[i].tipo = rand() % 3; // 0 = ônibus alto, 1 = baixo, 2 = alto vazado
            
            if (obstaculos[i].tipo == 0) {
                // Ônibus alto (precisa desviar ou abaixar)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 80;
            } else if (obstaculos[i].tipo == 1) {
                // Obstáculo baixo no chão (precisa pular)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 30;
            } else {
                // Obstáculo alto vazado (precisa abaixar para passar por baixo)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 50; // Alto, mas deixa espaço embaixo
            }
            
            break;
        }
    }
}

void criarMultiplosObstaculos(Obstaculo obstaculos[], int tamanho, float screenHeight, int quantidade) {
    int criados = 0;
    int lanes_usadas[3] = {0, 0, 0}; // Controla quais lanes já têm obstáculo
    int tipos_criados[3] = {0, 0, 0}; // Conta quantos de cada tipo: [laranja, verde, roxo]
    
    for (int i = 0; i < tamanho && criados < quantidade; i++) {
        if (!obstaculos[i].ativo) {
            int lane_tentativa;
            int tentativas = 0;
            
            // Tenta encontrar uma lane que ainda não tem obstáculo
            do {
                lane_tentativa = rand() % 3;
                tentativas++;
                if (tentativas >= 20) break; // Evita loop infinito
            } while (lanes_usadas[lane_tentativa] && tentativas < 20);
            
            // Se não conseguiu achar lane livre, pula este obstáculo
            if (lanes_usadas[lane_tentativa]) {
                continue;
            }
            
            obstaculos[i].ativo = 1;
            obstaculos[i].lane = lane_tentativa;
            obstaculos[i].pos_y = -100; // Começa acima da tela
            
            // Define o tipo, mas evita 3 laranjas
            int tipo_tentativa;
            if (quantidade == 3 && criados == 2 && tipos_criados[0] == 2) {
                // Se já tem 2 laranjas e está criando o terceiro, força outro tipo
                tipo_tentativa = (rand() % 2) + 1; // 1 (verde) ou 2 (roxo)
            } else {
                tipo_tentativa = rand() % 3; // 0, 1 ou 2
            }
            
            obstaculos[i].tipo = tipo_tentativa;
            tipos_criados[tipo_tentativa]++;
            
            if (obstaculos[i].tipo == 0) {
                // Ônibus alto (precisa desviar ou abaixar)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 80;
            } else if (obstaculos[i].tipo == 1) {
                // Obstáculo baixo no chão (precisa pular)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 30;
            } else {
                // Obstáculo alto vazado (precisa abaixar para passar por baixo)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 50;
            }
            
            lanes_usadas[lane_tentativa] = 1; // Marca a lane como usada
            criados++;
        }
    }
}

void atualizarObstaculos(Obstaculo obstaculos[], int tamanho, float velocidade) {
    for (int i = 0; i < tamanho; i++) {
        if (obstaculos[i].ativo) {
            obstaculos[i].pos_y += velocidade; // Move para baixo
            
            // Desativa se sair da tela
            if (obstaculos[i].pos_y > 700) {
                obstaculos[i].ativo = 0;
            }
        }
    }
}

int verificarColisao(Jogador *j, Obstaculo *obs, float lane_width) {
    if (!obs->ativo) return 0;
    
    // Se é um obstáculo baixo e o jogador está pulando, não colide
    if (obs->tipo == 1 && j->pulando) {
        return 0;
    }
    
    // Se é um obstáculo alto vazado e o jogador está abaixado, não colide
    if (obs->tipo == 2 && j->abaixado) {
        return 0;
    }
    
    // Calcula posição do obstáculo baseado na lane
    float obs_x = lane_width * obs->lane + lane_width / 2;
    
    // Hitbox do jogador
    float player_left = j->pos_x_real - 20;
    float player_right = j->pos_x_real + 20;
    float player_top = j->abaixado ? j->pos_y_real + 20 : j->pos_y_real;
    float player_bottom = j->abaixado ? j->pos_y_real + 40 : j->pos_y_real + 40;
    
    // Hitbox do obstáculo
    float obs_left = obs_x - obs->largura / 2;
    float obs_right = obs_x + obs->largura / 2;
    float obs_top = obs->pos_y;
    float obs_bottom = obs->pos_y + obs->altura;
    
    // Verifica colisão (AABB)
    if (player_right > obs_left && 
        player_left < obs_right && 
        player_bottom > obs_top && 
        player_top < obs_bottom) {
        return 1; // Colidiu!
    }
    
    return 0; // Não colidiu
}