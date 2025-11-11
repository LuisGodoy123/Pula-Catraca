#ifndef MECANICA_PRINCIPAL_H
#define MECANICA_PRINCIPAL_H

#define MAX_OBSTACULOS 10
#define MAX_ITENS 25
#define TIPOS_ITENS 5

typedef struct {
    float pos_x;
    float pos_y;
    int lane; // 0 = esquerda, 1 = centro, 2 = direita
    int ativo; // 1 = ativo, 0 = inativo
    int tipo; // 0 a 4 (5 tipos diferentes)
    float largura;
    float altura;
    int coletado; // 1 = já coletado, 0 = disponível
} ItemColetavel;

typedef struct {
    int x;
    int y;
    int lane; // 0 = esquerda, 1 = centro, 2 = direita
    int pulando;
    int abaixado;
    int velocidade_pulo;
    int tempo_abaixado;
    float pos_x_real; // posição real em pixels
    float pos_y_real; // posição real em pixels
    float chao_y; // posição do chão
} Jogador;

typedef struct {
    float pos_x;
    float pos_y;
    int lane; // 0 = esquerda, 1 = centro, 2 = direita
    int ativo; // 1 = ativo, 0 = inativo
    int tipo; // 0 = ônibus, 1 = obstáculo baixo, 2 = obstáculo alto
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
void criarObstaculo(Obstaculo obstaculos[], int tamanho, float screenHeight, float horizon_y);
void criarMultiplosObstaculos(Obstaculo obstaculos[], int tamanho, float screenHeight, int quantidade, float horizon_y);
void atualizarObstaculos(Obstaculo obstaculos[], int tamanho, float velocidade);
int verificarColisao(Jogador *j, Obstaculo *obs, float lane_width, float lane_offset, float horizon_y, float screenHeight);

// funções para itens colecionáveis
void inicializarItens(ItemColetavel itens[], int tamanho);
void criarItem(ItemColetavel itens[], int tamanho, float screenHeight, Obstaculo obstaculos[], int tamanhoObstaculos, float horizon_y);
void atualizarItens(ItemColetavel itens[], int tamanho, float velocidade);
int verificarColeta(Jogador *j, ItemColetavel *item, float lane_width, float lane_offset);

#endif
