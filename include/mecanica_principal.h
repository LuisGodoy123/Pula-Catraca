#ifndef MECANICA_PRINCIPAL_H
#define MECANICA_PRINCIPAL_H

// ============================================================
// CONSTANTES DO JOGO
#define MAX_OBSTACULOS 10
#define MAX_ITENS 25
#define TIPOS_ITENS 8  // 5 itens bons + 3 itens ruins
#define MAX_LANES 3

// ============================================================
// TIPOS DE OBSTÁCULOS
typedef enum {
    OBSTACULO_ONIBUS = 0,        // Obstáculo alto (desviar com A/D)
    OBSTACULO_CATRACA = 1,       // Obstáculo baixo (pular com W)
    OBSTACULO_CERCA_LARANJA = 2  // Obstáculo alto vazado (deslizar com S)
} TipoObstaculo;

// ============================================================
// TIPOS DE ITENS COLECIONÁVEIS
typedef enum {
    // Itens BONS (necessários para a vitória)
    ITEM_PIPOCA = 0,
    ITEM_MOEDA = 1,
    ITEM_VEM = 2,
    ITEM_BOTAO_PARADA = 3,
    ITEM_FONE = 4,

    // Itens RUINS (prejudicam o jogador)
    ITEM_SONO = 5,               // Aumenta 5 segundos no tempo
    ITEM_BALACLAVA = 6,          // Perde TODOS os itens
    ITEM_IDOSA = 7               // Perde 1 item aleatório
} TipoItem;

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
    int lane; // 0 = esquerda, 1 = centro, 2 = direita
    int pulando;
    int deslizando;
    int veloc_inicio_pulo;
    int tempo_deslizando;
    float pos_x_real; // posição em pixels
    float pos_y_real; // posição em pixels
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
void deslizar(Jogador *j);
void atualizarFisica(Jogador *j);

void inicializarObstaculos(Obstaculo obstaculos[], int tamanho);
void criarObstaculo(Obstaculo obstaculos[], int tamanho, float alturaTela, float horizonte);
void criarMultiplosObstaculos(Obstaculo obstaculos[], int tamanho, float alturaTela, int quantidade, float horizonte);
void atualizarObstaculos(Obstaculo obstaculos[], int tamanho, float velocidade, float horizonte, int alturaTela, float delta);
int verificarColisao(Jogador *j, Obstaculo *obs, float lane_width, float lane_offset, float horizonte, float alturaTela);

// funções para itens colecionáveis
void inicializarItens(ItemColetavel itens[], int tamanho);
void criarItem(ItemColetavel itens[], int tamanho, float alturaTela, Obstaculo obstaculos[], int tamanhoObstaculos, float horizonte, int itensColetados[]);
void atualizarItens(ItemColetavel itens[], int tamanho, float velocidade, float horizonte, int alturaTela, float delta);
int verificarColeta(Jogador *j, ItemColetavel *item, float lane_width, float lane_offset);

#endif
