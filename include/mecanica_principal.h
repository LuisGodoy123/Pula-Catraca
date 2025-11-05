#ifndef MECANICA_PRINCIPAL_H
#define MECANICA_PRINCIPAL_H

#define MAX_OBSTACULOS 10

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
    float chao_y; // posição do chão
} Jogador;

typedef struct {
    float pos_x;
    float pos_y;
    int lane; // 0 = esquerda, 1 = centro, 2 = direita
    int ativo; // 1 = ativo, 0 = inativo
    int tipo; // 0 = ônibus alto, 1 = obstáculo baixo (chão), 2 = obstáculo alto vazado
    float largura;
    float altura;
} Obstaculo;

void inicializarJogador(Jogador *j, float pos_x_inicial, float pos_y_inicial);
void moverEsquerda(Jogador *j);
void moverDireita(Jogador *j);
void pular(Jogador *j);
void abaixar(Jogador *j);
void atualizarFisica(Jogador *j);

void inicializarObstaculos(Obstaculo obstaculos[], int tamanho);
void criarObstaculo(Obstaculo obstaculos[], int tamanho, float screenHeight);
void criarMultiplosObstaculos(Obstaculo obstaculos[], int tamanho, float screenHeight, int quantidade);
void atualizarObstaculos(Obstaculo obstaculos[], int tamanho, float velocidade);
int verificarColisao(Jogador *j, Obstaculo *obs, float lane_width);

#endif
