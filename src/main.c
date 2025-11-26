#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "../include/mecanica_principal.h"
#include "../include/ranking.h"
#include <stdio.h>
#include <stdbool.h>

// ============================================================
// CONSTANTES GLOBAIS
// ============================================================

// Constantes de renderização
#define TAMANHO_BASE_ITEM 120.0f
#define ALTURA_HORIZONTE 130.0f

// Constantes de tempo e animação
#define FRAMES_POR_SEGUNDO 60
#define TEMPO_ANIMACAO_SPRITE 0.25f
#define DURACAO_EFEITO_FONE 4.0f
#define INTERVALO_ACELERACAO 30.0f
#define INTERVALO_AUMENTO_FREQUENCIA 10.0f

// Constantes de velocidade
#define VELOCIDADE_INICIAL 3.0f
#define VELOCIDADE_MAXIMA 8.0f
#define INCREMENTO_VELOCIDADE 1.0f

// Constantes de spawn
#define FRAMES_ENTRE_OBSTACULOS_INICIAL 120
#define FRAMES_ENTRE_OBSTACULOS_MINIMO 40
#define FRAMES_ENTRE_ITENS 120

// Constantes de perspectiva (baseadas em tela 800x600)
#define LARGURA_FAIXA_TOPO_PERCENTUAL 0.083f
#define DESLOCAMENTO_FAIXA_TOPO_PERCENTUAL 0.385f
#define LARGURA_FAIXA_BASE_PERCENTUAL 0.45f
#define DESLOCAMENTO_FAIXA_BASE_PERCENTUAL -0.175f

// ============================================================
// CORES DA INTERFACE
// ============================================================
#define COR_ROSA_INTERFACE (Color){255, 102, 196, 255}      // #ff66c4
#define COR_AMARELO_INTERFACE (Color){254, 255, 153, 255}   // #feff99
#define COR_AZUL_INTERFACE (Color){175, 218, 225, 255}      // #afdae1
#define COR_VERDE_INTERFACE (Color){87, 183, 33, 255}       // #57b721
#define COR_CIANO_INTERFACE (Color){102, 255, 255, 255}     // #66FFFF

// Cores dos itens colecionáveis (fallback caso sprites não carreguem)
#define COR_ITEM_PIPOCA YELLOW
#define COR_ITEM_MOEDA SKYBLUE
#define COR_ITEM_VEM PINK
#define COR_ITEM_BOTAO_PARADA GOLD
#define COR_ITEM_FONE GREEN

// ============================================================
// ESTADOS DO JOGO
// ============================================================
typedef enum {
    ESTADO_MENU = 0,
    ESTADO_NICKNAME = 1,
    ESTADO_JOGANDO = 2,
    ESTADO_RANKING = 3,
    ESTADO_COMO_JOGAR = 4
} EstadoJogo;

typedef enum {
    DIRECAO_ESQUERDA = -1,
    DIRECAO_CENTRO = 0,
    DIRECAO_DIREITA = 1
} DirecaoJogador;

// Protótipos das funções
void TelaMenu(int *estadoJogo, int larguraTela, int alturaTela, Texture2D background, Sound somMenu);
void TelaNickname(int *estadoJogo, int larguraTela, int alturaTela, Texture2D background, char *nickname, Sound somMenu);
void TelaJogo(int *estadoJogo, int larguraTela, int alturaTela, Texture2D background_jogo, char *nickname, Sound somMenu, Sound somCorrida, Sound somItemBom, Sound somItemRuim, Sound somColisao, Sound somVitoria, Sound somMusicaVitoria);
void TelaRanking(int *estadoJogo, int larguraTela, int alturaTela, Texture2D background);
void TelaComoJogar(int *estadoJogo, int larguraTela, int alturaTela, Texture2D background);

// Ranking (persistente)
static ListaRanking ranking;

// desenha texto com quebra por largura (word wrap) e retorna a altura ocupada
static float DesenharTextoQuebrado(Font fonte, const char *texto, Vector2 posicao, float tamanhoFonte, float espacamento, float larguraQuebra, Color cor) {
    // Copia o texto para poder tokenizar
    size_t tamanho = strlen(texto);
    char *buffer = (char *)malloc(tamanho + 1);
    if (!buffer) return 0.0f;
    strcpy(buffer, texto);

    float y = posicao.y;
    char linha[1024] = {0};
    char *palavra = strtok(buffer, " ");

    while (palavra) {
        char candidato[2048] = {0};
        if (linha[0] == '\0')
            snprintf(candidato, sizeof(candidato), "%s", palavra);
        else
            snprintf(candidato, sizeof(candidato), "%s %s", linha, palavra);

        Vector2 medida = MeasureTextEx(fonte, candidato, tamanhoFonte, espacamento);
        if (medida.x <= larguraQuebra) {
            // cabe na mesma linha
            strncpy(linha, candidato, sizeof(linha) - 1);
        } else {
            // desenha a linha atual e inicia nova linha com a palavra
            if (linha[0] != '\0') {
                DrawTextEx(fonte, linha, (Vector2){posicao.x, y}, tamanhoFonte, espacamento, cor);
                y += tamanhoFonte * 1.15f;
            }
            // palavra começa a nova linha
            strncpy(linha, palavra, sizeof(linha) - 1);
        }

        palavra = strtok(NULL, " ");
    }

    if (linha[0] != '\0') {
        DrawTextEx(fonte, linha, (Vector2){posicao.x, y}, tamanhoFonte, espacamento, cor);
        y += tamanhoFonte * 1.15f;
    }

    free(buffer);
    return y - posicao.y; // altura ocupada
}

// Constantes de paths de assets
#define ASSET_PIPOCA "assets/images/pipoca.png"
#define ASSET_MOEDA "assets/images/moeda.png"
#define ASSET_VEM "assets/images/VEM.png"
#define ASSET_BOTAO_PARADA "assets/images/botao_parada.png"
#define ASSET_FONE "assets/images/fone.png"

// calcula progresso normalizado entre 0 e 1
static float CalcularProgresso(float valor, float min, float max) {
    float progresso = (valor - min) / (max - min);
    if (progresso < 0) progresso = 0;
    if (progresso > 1) progresso = 1;
    return progresso;
}

// desenha fundo (lanes com perspectiva)
static void DesenharFundo(Texture2D background, int larguraTela, int alturaTela) {
    if (background.id > 0) {
        Rectangle origem = {0, 0, (float)background.width, (float)background.height};
        Rectangle destino = {0, 0, (float)larguraTela, (float)alturaTela};
        DrawTexturePro(background, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
    }
}

// desenha botão centralizado e retorna se foi clicado
static bool DesenharBotao(const char *text, float centerX, float y, float larguraBotao, float alturaBotao, float fontSize, Color corNormal, Color corHover, Color textoNormal, Color textoHover) {
    Font font = GetFontDefault();
    Rectangle btn = {centerX - larguraBotao / 2, y, larguraBotao, alturaBotao};
    Vector2 mousePos = GetMousePosition();
    bool hover = CheckCollisionPointRec(mousePos, btn);

    DrawRectangleRounded(btn, 0.3f, 10, hover ? corHover : corNormal);

    float textWidth = MeasureTextEx(font, text, fontSize, 2).x;
    DrawTextEx(font, text,
               (Vector2){btn.x + larguraBotao / 2 - textWidth / 2, btn.y + alturaBotao / 2 - fontSize / 2},
               fontSize, 2, hover ? textoHover : textoNormal);

    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// carrega texturas de itens colecionáveis (0-4 = bons)
static void CarregarTexturasItens(Texture2D texturas[]) {
    texturas[0] = LoadTexture(ASSET_PIPOCA);
    texturas[1] = LoadTexture(ASSET_MOEDA);
    texturas[2] = LoadTexture(ASSET_VEM);
    texturas[3] = LoadTexture(ASSET_BOTAO_PARADA);
    texturas[4] = LoadTexture(ASSET_FONE);
}

int main(void) {
    // resolução  e init da janela
    int larguraTela = 800;
    int alturaTela = 600;
    InitWindow(larguraTela, alturaTela, "Pula-Catraca");
    SetTraceLogLevel(LOG_WARNING); // Desabilita mensagens de INFO e DEBUG
    SetTargetFPS(60);
    
    // Inicializa sistema de áudio
    InitAudioDevice();

    // Inicializa e carrega ranking salvo (se existir)
    initRanking(&ranking);
    carregarTodosRanking(&ranking, "ranking_all.txt");
    
    // Carrega sons
    Sound somMenu = LoadSound("assets/sound/menu.wav");
    Sound somCorrida = LoadSound("assets/sound/corrida.wav");
    Sound somItemBom = LoadSound("assets/sound/item_bom.wav");
    Sound somItemRuim = LoadSound("assets/sound/item_ruim.wav");
    Sound somColisao = LoadSound("assets/sound/ouch.wav");
    Sound somVitoria = LoadSound("assets/sound/vitoria.wav");
    Sound somMusicaVitoria = LoadSound("assets/sound/musica_vitoria.mp3");
    
    // Ajusta volume dos sons (0.0 a 1.0)
    SetSoundVolume(somMenu, 1.0f);
    SetSoundVolume(somCorrida, 1.0f);
    SetSoundVolume(somItemBom, 1.0f);
    SetSoundVolume(somItemRuim, 1.0f);
    SetSoundVolume(somColisao, 1.0f);
    SetSoundVolume(somVitoria, 1.0f);
    SetSoundVolume(somMusicaVitoria, 1.0f);

    // carrega imagens de fundo
    Texture2D background_menu = {0};
    Texture2D background_jogo = {0};
    Image fundo_menu = LoadImage("assets/images/fundo_menu.png");
    Image fundo_do_jogo = LoadImage("assets/images/fundo_do_jogo.png");

    if (fundo_menu.data != NULL) {
        background_menu = LoadTextureFromImage(fundo_menu);
        UnloadImage(fundo_menu);
    } else {
        // fundo = cor, caso não carregue a imagem do menu
        Image tempImg = GenImageColor(larguraTela, alturaTela, (Color){215, 50, 133, 255}); // #d73285
        background_menu = LoadTextureFromImage(tempImg);
        UnloadImage(tempImg);
    }

    if (fundo_do_jogo.data != NULL) {
        background_jogo = LoadTextureFromImage(fundo_do_jogo);
        UnloadImage(fundo_do_jogo);
    } else {
        // fundo = cor, caso não carregue a imagem do menu
        Image tempImg = GenImageColor(larguraTela, alturaTela, (Color){96, 80, 125, 255}); // #60507d
        background_jogo = LoadTextureFromImage(tempImg);
        UnloadImage(tempImg);
    }

    int estadoJogo = 0; // 0 = menu, 1 = tela nickname, 2 = jogando, 3 = ranking, 4 = como jogar
    char nickname[21] = ""; // Armazena até 20 caracteres + null terminator
    
    while (!WindowShouldClose()) {
        if (estadoJogo == 0) {
            TelaMenu(&estadoJogo, larguraTela, alturaTela, background_menu, somMenu);
        } else if (estadoJogo == 1) {
            TelaNickname(&estadoJogo, larguraTela, alturaTela, background_menu, nickname, somMenu);
        } else if (estadoJogo == 2) {
            TelaJogo(&estadoJogo, larguraTela, alturaTela, background_jogo, nickname, somMenu, somCorrida, somItemBom, somItemRuim, somColisao, somVitoria, somMusicaVitoria);
        } else if (estadoJogo == 3) {
            TelaRanking(&estadoJogo, larguraTela, alturaTela, background_menu);
        } else if (estadoJogo == 4) {
            TelaComoJogar(&estadoJogo, larguraTela, alturaTela, background_menu);
        }
    }
    // salva ranking completo e top5 antes de sair
    salvarRankingCompleto(&ranking, "ranking_all.txt");
    salvarTopTXT(&ranking, "ranking_top5.txt", 5);
    freeRanking(&ranking);
    
    // Descarrega sons
    UnloadSound(somMenu);
    UnloadSound(somCorrida);
    UnloadSound(somItemBom);
    UnloadSound(somItemRuim);
    UnloadSound(somColisao);
    UnloadSound(somVitoria);
    UnloadSound(somMusicaVitoria);

    UnloadTexture(background_menu);
    UnloadTexture(background_jogo);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void TelaMenu(int *estadoJogo, int larguraTela, int alturaTela, Texture2D background, Sound somMenu) {
    // Toca som do menu em loop (se não estiver tocando)
    if (!IsSoundPlaying(somMenu)) {
        PlaySound(somMenu);
    }

    // botões proporcionais à tela
    float larguraBotao = larguraTela * 0.25f;
    float alturaBotao = alturaTela * 0.08f;
    float fontSize = larguraTela * 0.035f;

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // fundo redimensionado
    DesenharFundo(background, larguraTela, alturaTela);

    // botão "PLAY"
    if (DesenharBotao("PLAY", larguraTela / 2.0f, alturaTela * 0.55f, larguraBotao, alturaBotao, fontSize, COR_AZUL_INTERFACE, COR_AMARELO_INTERFACE, COR_VERDE_INTERFACE, COR_ROSA_INTERFACE)) {
        StopSound(somMenu);
        *estadoJogo = 1;
    }

    // botão "RANKING"
    if (DesenharBotao("RANKING", larguraTela / 2.0f, alturaTela * 0.65f, larguraBotao, alturaBotao, fontSize, COR_AZUL_INTERFACE, COR_AMARELO_INTERFACE, COR_VERDE_INTERFACE, COR_ROSA_INTERFACE)) {
        *estadoJogo = 3;
    }

    // botão "COMO JOGAR"
    if (DesenharBotao("COMO JOGAR", larguraTela / 2.0f, alturaTela * 0.75f, larguraBotao, alturaBotao, fontSize, COR_AZUL_INTERFACE, COR_AMARELO_INTERFACE, COR_VERDE_INTERFACE, COR_ROSA_INTERFACE)) {
        *estadoJogo = 4;
    }

    EndDrawing();
}

void TelaNickname(int *estadoJogo, int larguraTela, int alturaTela, Texture2D background, char *nickname, Sound somMenu) {
    if (!IsSoundPlaying(somMenu)) {
        PlaySound(somMenu);
    }

    Font fonteTitulo = GetFontDefault();

    // caixa de texto
    float larguraCaixa = larguraTela * 0.4f;
    float alturaCaixa = alturaTela * 0.08f;
    Rectangle caixaInput = {
        larguraTela / 2 - larguraCaixa / 2,
        alturaTela * 0.5f,
        larguraCaixa,
        alturaCaixa
    };

    // botão confirmar
    float larguraBotao = larguraTela * 0.25f;
    float alturaBotao = alturaTela * 0.08f;
    Rectangle confirmBtn = {
        larguraTela / 2 - larguraBotao / 2,
        alturaTela * 0.65f,
        larguraBotao,
        alturaBotao
    };

    Vector2 mousePos = GetMousePosition();
    bool hoverConfirm = CheckCollisionPointRec(mousePos, confirmBtn);

    // captura teclas
    int tecla = GetCharPressed();
    int nicknameLen = strlen(nickname);
    
    while (tecla > 0) {
        // aceita letras, números, espaço (32-126 ASCII)
        if ((tecla >= 32) && (tecla <= 126) && (nicknameLen < 20)) {
            nickname[nicknameLen] = (char)tecla;
            nickname[nicknameLen + 1] = '\0';
            nicknameLen++;
        }
        tecla = GetCharPressed();
    }

    // backspace
    if (IsKeyPressed(KEY_BACKSPACE) && nicknameLen > 0) {
        nickname[nicknameLen - 1] = '\0';
    }

    // enter ou clique no botão confirmar
    if ((IsKeyPressed(KEY_ENTER) || (hoverConfirm && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) && nicknameLen > 0) {
        *estadoJogo = 2; // vai p jogo
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DesenharFundo(background, larguraTela, alturaTela);

    // título
    float fontSize = larguraTela * 0.06f;
    const char *title = "Digite seu nickname:";
    float titleWidth = MeasureTextEx(fonteTitulo, title, fontSize * 0.7f, 2).x;
    DrawTextEx(fonteTitulo, title,
               (Vector2){larguraTela / 2 - titleWidth / 2, alturaTela * 0.35f},
               fontSize * 0.7f, 2, COR_ROSA_INTERFACE);

    // caixa de input
    DrawRectangleRounded(caixaInput, 0.3f, 10, COR_AZUL_INTERFACE);
    DrawRectangleLinesEx(caixaInput, 3.0f, COR_VERDE_INTERFACE);

    // texto digitado
    if (nicknameLen > 0) {
        float textWidth = MeasureTextEx(fonteTitulo, nickname, fontSize * 0.5f, 2).x;
        DrawTextEx(fonteTitulo, nickname,
                   (Vector2){
                       caixaInput.x + larguraCaixa / 2 - textWidth / 2,
                       caixaInput.y + alturaCaixa / 2 - fontSize * 0.25f
                   },
                   fontSize * 0.5f, 2, BLACK);
    } else {
        // placeholder
        const char *placeholder = "Seu nome aqui...";
        float phWidth = MeasureTextEx(fonteTitulo, placeholder, fontSize * 0.4f, 2).x;
        DrawTextEx(fonteTitulo, placeholder,
                   (Vector2){
                       caixaInput.x + larguraCaixa / 2 - phWidth / 2,
                       caixaInput.y + alturaCaixa / 2 - fontSize * 0.2f
                   },
                   fontSize * 0.4f, 2, GRAY);
    }

    // cursor piscando
    static float cursorTimer = 0.0f;
    cursorTimer += GetFrameTime();
    if (((int)(cursorTimer * 2)) % 2 == 0 && nicknameLen < 20) {
        float textWidth = MeasureTextEx(fonteTitulo, nickname, fontSize * 0.5f, 2).x;
        DrawRectangle(
            caixaInput.x + larguraCaixa / 2 + textWidth / 2 + 5,
            caixaInput.y + alturaCaixa * 0.25f,
            2,
            alturaCaixa * 0.5f,
            BLACK
        );
    }

    // botão confirmar (só ativo se tiver texto)
    if (nicknameLen > 0) {
        DrawRectangleRounded(confirmBtn, 0.3f, 10, hoverConfirm ? COR_AMARELO_INTERFACE : COR_AZUL_INTERFACE);
        DrawTextEx(fonteTitulo, "CONFIRMAR",
                   (Vector2){
                       confirmBtn.x + larguraBotao / 2 - MeasureTextEx(fonteTitulo, "CONFIRMAR", fontSize * 0.5f, 2).x / 2,
                       confirmBtn.y + alturaBotao / 2 - fontSize * 0.25f
                   },
                   fontSize * 0.5f, 2, hoverConfirm ? COR_ROSA_INTERFACE : COR_VERDE_INTERFACE);
    } else {
        DrawRectangleRounded(confirmBtn, 0.3f, 10, GRAY);
        DrawTextEx(fonteTitulo, "CONFIRMAR",
                   (Vector2){
                       confirmBtn.x + larguraBotao / 2 - MeasureTextEx(fonteTitulo, "CONFIRMAR", fontSize * 0.5f, 2).x / 2,
                       confirmBtn.y + alturaBotao / 2 - fontSize * 0.25f
                   },
                   fontSize * 0.5f, 2, DARKGRAY);
    }

    // Botão de voltar
    if (DesenharBotao("VOLTAR", larguraTela / 2.0f, alturaTela * 0.9f, larguraTela * 0.2f, alturaTela * 0.08f, larguraTela * 0.04f, COR_CIANO_INTERFACE, COR_AMARELO_INTERFACE, BLACK, COR_ROSA_INTERFACE)) {
        *estadoJogo = 0;
    }
    
    EndDrawing();
}

void TelaJogo(int *estadoJogo, int larguraTela, int alturaTela, Texture2D background_jogo, char *nickname, Sound somMenu, Sound somCorrida, Sound somItemBom, Sound somItemRuim, Sound somColisao, Sound somVitoria, Sound somMusicaVitoria) {
    static Jogador jogador;
    static Obstaculo obstaculos[MAX_OBSTACULOS];
    static ItemColetavel itens[MAX_ITENS];
    static int itensColetados[TIPOS_ITENS]; // contador de cada tipo coletado
    static bool inicializado = false;
    static int frameCont = 0;
    static int frameContItens = -120; // Começa negativo para delay de 2 segundos (120 frames)
    static float velocidadeJogo = VELOCIDADE_INICIAL;
    static float velocidadeMaxima = VELOCIDADE_MAXIMA;
    static float intervaloAceleracao = INTERVALO_ACELERACAO;
    static float tempoUltimaAceleracao = 0.0f;
    static float incrementoVelocidade = INCREMENTO_VELOCIDADE;
    static float tempoDecorrido = 0.0f;
    static bool fimDeJogo = false;
    static bool vitoria = false;
    static bool rankingInserido = false;
    static int direcaoJogador = DIRECAO_CENTRO;
    static float tempoAnimacao = 0.0f;
    static bool quadroAnimacao = false;

    // Sistema progressivo de obstáculos
    static int framesEntreObstaculos = FRAMES_ENTRE_OBSTACULOS_INICIAL;
    static int framesMinimos = FRAMES_ENTRE_OBSTACULOS_MINIMO;
    static float tempoUltimoAumentoFrequencia = 0.0f;
    static float intervaloAumentoFrequencia = INTERVALO_AUMENTO_FREQUENCIA;
    
    // Texturas dos obstáculos
    static Texture2D spriteOnibusEsquerdo = {0};
    static Texture2D spriteOnibusCentro = {0};
    static Texture2D spriteOnibusDireito = {0};
    static Texture2D spriteCatraca = {0};
    static Texture2D spriteLaranja = {0};
    static bool spritesCarregadas = false;
    
    // Texturas dos itens colecionáveis
    static Texture2D texturasItens[TIPOS_ITENS] = {0};
    static bool texturasCarregadas = false;
    
    // Texturas do jogador
    static Texture2D spriteCorrendoDireita = {0};
    static Texture2D spriteCorrendoEsquerda = {0};
    static Texture2D spritePulandoDireita = {0};
    static Texture2D spritePulandoEsquerda = {0};
    static Texture2D spriteDeslizandoDireita = {0};
    static Texture2D spriteDeslizandoEsquerda = {0};
    static bool spritesJogadorCarregadas = false;
    
    // Textura de Game Over
    static Texture2D texturaGameOver = {0};
    static bool texturaGameOverCarregada = false;
    
    // Texturas de vitória
    static Texture2D texturaVitoria1 = {0};
    static Texture2D texturaVitoria2 = {0};
    static bool texturasVitoriaCarregadas = false;
    static int cenaVitoria = 0; // 0 = tela normal, 1 = scene1, 2 = scene2, 3 = voltou ao normal
    
    // ============================================================
    // PERSPECTIVA DAS LANES
    // ============================================================
    float larguraTopoLane = larguraTela * LARGURA_FAIXA_TOPO_PERCENTUAL;
    float deslocTopoLane = larguraTela * DESLOCAMENTO_FAIXA_TOPO_PERCENTUAL;
    float larguraLaneFundo = larguraTela * LARGURA_FAIXA_BASE_PERCENTUAL;
    float deslocLaneFundo = larguraTela * DESLOCAMENTO_FAIXA_BASE_PERCENTUAL;

    // ============================================================
    // SEÇÃO: INICIALIZAÇÃO E CARREGAMENTO DE RECURSOS
    // ============================================================

    // Inicializa jogador e arrays (apenas na primeira vez)
    if (!inicializado) {
        float pos_x = larguraTela / 2;
        float pos_y = alturaTela - 100;
        inicializarJogador(&jogador, pos_x, pos_y);
        inicializarObstaculos(obstaculos, MAX_OBSTACULOS);
        inicializarItens(itens, MAX_ITENS);

        for (int i = 0; i < TIPOS_ITENS; i++) {
            itensColetados[i] = 0;
        }

        int quantidade_inicial = (rand() % 3) + 1;
        criarMultiplosObstaculos(obstaculos, MAX_OBSTACULOS, alturaTela, quantidade_inicial, ALTURA_HORIZONTE);

        // Reseta variáveis de progresso e dificuldade
        velocidadeJogo = VELOCIDADE_INICIAL;
        framesEntreObstaculos = FRAMES_ENTRE_OBSTACULOS_INICIAL;
        tempoDecorrido = 0.0f;
        tempoUltimaAceleracao = 0.0f;
        tempoUltimoAumentoFrequencia = 0.0f;
        frameCont = 0;
        frameContItens = -120;
        fimDeJogo = false;
        vitoria = false;
        rankingInserido = false;
        cenaVitoria = 0;
        direcaoJogador = DIRECAO_CENTRO;
        tempoAnimacao = 0.0f;
        quadroAnimacao = false;

        inicializado = true;
    }

    // Carrega texturas dos obstáculos (apenas uma vez)
    if (!spritesCarregadas) {
        spriteOnibusEsquerdo = LoadTexture("assets/images/onibus_lane_esq.png");
        spriteOnibusCentro = LoadTexture("assets/images/onibus_centro.png");
        spriteOnibusDireito = LoadTexture("assets/images/onibus_lane_dir.png");
        spriteCatraca = LoadTexture("assets/images/catraca.png");
        spriteLaranja = LoadTexture("assets/images/cerca_laranja.png");
        spritesCarregadas = true;
    }

    // Carrega texturas dos itens (apenas uma vez)
    if (!texturasCarregadas) {
        CarregarTexturasItens(texturasItens);
        texturasItens[ITEM_SONO] = LoadTexture("assets/images/sono.png");
        texturasItens[ITEM_BALACLAVA] = LoadTexture("assets/images/balaclava.png");
        texturasItens[ITEM_IDOSA] = LoadTexture("assets/images/idosa.png");
        texturasCarregadas = true;
    }

    // Carrega sprites do jogador (apenas uma vez)
    if (!spritesJogadorCarregadas) {
        spriteCorrendoDireita = LoadTexture("assets/images/correndo_dir_frente.png");
        spriteCorrendoEsquerda = LoadTexture("assets/images/correndo_esq_frente.png");
        spritePulandoDireita = LoadTexture("assets/images/pulando_p_dir.png");
        spritePulandoEsquerda = LoadTexture("assets/images/pulando_p_esq.png");
        spriteDeslizandoDireita = LoadTexture("assets/images/abaixado_p_dir.png");
        spriteDeslizandoEsquerda = LoadTexture("assets/images/abaixado_p_esq.png");
        spritesJogadorCarregadas = true;
    }
    
    // Carrega textura de Game Over (apenas uma vez)
    if (!texturaGameOverCarregada) {
        texturaGameOver = LoadTexture("assets/images/gameOver.png");
        texturaGameOverCarregada = true;
    }
    
    // Carrega texturas de vitória (apenas uma vez)
    if (!texturasVitoriaCarregadas) {
        texturaVitoria1 = LoadTexture("assets/images/vitoria_scene1.png");
        texturaVitoria2 = LoadTexture("assets/images/vitoria_scene2.png");
        texturasVitoriaCarregadas = true;
    }

    // ============================================================
    // SEÇÃO: LÓGICA DO JOGO (INPUT E FÍSICA)
    // ============================================================

    if (!fimDeJogo) {
        // Gerenciamento de áudio
        static bool somInicializado = false;
        if (!somInicializado) {
            StopSound(somMenu);
            somInicializado = true;
        }
        if (!IsSoundPlaying(somCorrida)) {
            PlaySound(somCorrida);
        }

        // Processamento de input do jogador
        if (IsKeyPressed(KEY_W)) {
            pular(&jogador);
        }
        if (IsKeyPressed(KEY_A)) {
            moverEsquerda(&jogador);
            direcaoJogador = DIRECAO_ESQUERDA;
        }
        if (IsKeyPressed(KEY_D)) {
            moverDireita(&jogador);
            direcaoJogador = DIRECAO_DIREITA;
        }
        if (IsKeyPressed(KEY_S)) {
            deslizar(&jogador);
        }

        // Atualiza física do jogador
        atualizarFisica(&jogador);

        // Atualiza tempo decorrido
        tempoDecorrido += 1.0f / FRAMES_POR_SEGUNDO;

        // Sistema de aceleração progressiva
        // Verifica se deve acelerar baseado no tempo decorrido
        if (velocidadeJogo < velocidadeMaxima) {
            if (tempoDecorrido - tempoUltimaAceleracao >= intervaloAceleracao) {
                velocidadeJogo += incrementoVelocidade;
                // Garante que não ultrapasse a velocidade máxima
                if (velocidadeJogo > velocidadeMaxima) {
                    velocidadeJogo = velocidadeMaxima;
                }
                tempoUltimaAceleracao = tempoDecorrido;
                // Ativa mensagem de aceleração
            }
        }

        // Sistema progressivo: aumenta frequência de obstáculos a cada 5 segundos
        if (framesEntreObstaculos > framesMinimos) {
            if (tempoDecorrido - tempoUltimoAumentoFrequencia >= intervaloAumentoFrequencia) {
                framesEntreObstaculos -= 10; // Reduz 10 frames (~0.17 segundos)
                if (framesEntreObstaculos < framesMinimos) {
                    framesEntreObstaculos = framesMinimos;
                }
                tempoUltimoAumentoFrequencia = tempoDecorrido;
            }
        }

        // Timer para animação das sprites do jogador (alterna a cada 0.25s)
        tempoAnimacao += GetFrameTime();
        if (tempoAnimacao >= 0.25f) {
            quadroAnimacao = !quadroAnimacao;
            tempoAnimacao = 0.0f;
        }
        
        // Spawn de obstáculos
        frameCont++;
        if (frameCont >= framesEntreObstaculos) {
            int quantidade = (rand() % 3) + 1;
            criarMultiplosObstaculos(obstaculos, MAX_OBSTACULOS, alturaTela, quantidade, ALTURA_HORIZONTE);
            frameCont = 0;
        }

        // Spawn de itens
        frameContItens++;
        if (frameContItens >= FRAMES_ENTRE_ITENS) {
            criarItem(itens, MAX_ITENS, alturaTela, obstaculos, MAX_OBSTACULOS, ALTURA_HORIZONTE, itensColetados);
            frameContItens = 0;
        }

        // Atualiza posição de obstáculos e itens
        float deltaTempo = GetFrameTime();
        atualizarObstaculos(obstaculos, MAX_OBSTACULOS, velocidadeJogo, ALTURA_HORIZONTE, alturaTela, deltaTempo);
        atualizarItens(itens, MAX_ITENS, velocidadeJogo, ALTURA_HORIZONTE, alturaTela, deltaTempo);

        // posição X baseada na lane com perspectiva
        // O jogador está em uma posição Y específica, então precisa interpolar igual aos obstáculos/itens

        // Calcula progresso baseado na posição Y do jogador (mesma lógica dos obstáculos)
        float progresso_jogador = (jogador.pos_y_real - ALTURA_HORIZONTE) / (alturaTela - ALTURA_HORIZONTE);
        if (progresso_jogador < 0) progresso_jogador = 0;
        if (progresso_jogador > 1) progresso_jogador = 1;

        // Posição X interpolada entre topo e base
        float x_topo = deslocTopoLane + larguraTopoLane * jogador.lane + larguraTopoLane / 2;
        float x_base = deslocLaneFundo + larguraLaneFundo * jogador.lane + larguraLaneFundo / 2;
        float x_alvo = x_topo + (x_base - x_topo) * progresso_jogador;

        // transição entre lanes
        if (jogador.pos_x_real < x_alvo) {
            jogador.pos_x_real += 10;
            if (jogador.pos_x_real > x_alvo) jogador.pos_x_real = x_alvo;
        } else if (jogador.pos_x_real > x_alvo) {
            jogador.pos_x_real -= 10;
            if (jogador.pos_x_real < x_alvo) jogador.pos_x_real = x_alvo;
        }

        // verifica coleta de itens
        for (int i = 0; i < MAX_ITENS; i++) {
            if (verificarColeta(&jogador, &itens[i], larguraLaneFundo, deslocLaneFundo)) {
                int tipo = itens[i].tipo;

                // Itens BONS (ITEM_PIPOCA até ITEM_FONE)
                if (tipo >= ITEM_PIPOCA && tipo <= ITEM_FONE) {
                    // incrementa apenas se ainda não atingiu o limite de 5
                    if (itensColetados[tipo] < 5) {
                        itensColetados[tipo]++;
                        PlaySound(somItemBom);
                    }
                }
                // Itens RUINS
                else if (tipo == ITEM_SONO) {
                    // SONO: aumenta 5 segundos no tempo
                    tempoDecorrido += 5.0f;
                    itensColetados[tipo]++;
                    PlaySound(somItemRuim);
                }
                else if (tipo == ITEM_BALACLAVA) {
                    // BALACLAVA: perde TODOS os itens bons
                    for (int j = ITEM_PIPOCA; j <= ITEM_FONE; j++) {
                        itensColetados[j] = 0;
                    }
                    itensColetados[tipo]++;
                    PlaySound(somItemRuim);
                }
                else if (tipo == ITEM_IDOSA) {
                    // IDOSA: perde 1 item aleatório
                    int itensDisponiveis[5];
                    int quantidadeDisponiveis = 0;
                    for (int j = ITEM_PIPOCA; j <= ITEM_FONE; j++) {
                        if (itensColetados[j] > 0) {
                            itensDisponiveis[quantidadeDisponiveis] = j;
                            quantidadeDisponiveis++;
                        }
                    }
                    if (quantidadeDisponiveis > 0) {
                        int indiceAleatorio = rand() % quantidadeDisponiveis;
                        int itemRemovido = itensDisponiveis[indiceAleatorio];
                        itensColetados[itemRemovido]--;
                    }
                    itensColetados[tipo]++;
                    PlaySound(somItemRuim);
                }
            }
        }

        // verifica vitoria (pelo menos 1 item de cada tipo BOM)
        if (!vitoria) {
            bool ganhou = true;
            for (int i = ITEM_PIPOCA; i <= ITEM_FONE; i++) {
                if (itensColetados[i] == 0) {
                    ganhou = false;
                    break;
                }
            }
            vitoria = ganhou;
            // Se o jogador acabou de vencer, insere no ranking (apenas uma vez)
            if (vitoria && !rankingInserido && nickname[0] != '\0') {
                insertRanking(&ranking, nickname, tempoDecorrido);
                salvarTopTXT(&ranking, "ranking_top5.txt", 5);
                salvarRankingCompleto(&ranking, "ranking_all.txt");
                rankingInserido = true;

                fimDeJogo = true; // Termina o jogo
                cenaVitoria = 1; // Inicia sequência de cenas de vitória
                PlaySound(somVitoria); // Toca som de vitória
                StopSound(somCorrida); // Para som de corrida
            }
        }
        // colisões (posição sem perspectiva p cálculo)
        for (int i = 0; i < MAX_OBSTACULOS; i++) {
            if (verificarColisao(&jogador, &obstaculos[i], larguraLaneFundo, deslocLaneFundo, ALTURA_HORIZONTE, alturaTela)) {
                fimDeJogo = true;
                PlaySound(somColisao); // Toca som de colisão
                StopSound(somCorrida); // Para som de corrida
                break;
            }
        }
    } else {
        // ENTER para avançar entre cenas de vitória
        if (IsKeyPressed(KEY_ENTER) && vitoria && cenaVitoria > 0 && cenaVitoria < 3) {
            cenaVitoria++; // Avança para próxima cena
        }
        
        // "Game Over" ou "Vitória" - C p continuar a run (mantém tempo e itens coletados)
        if (IsKeyPressed(KEY_C)) {
            // Reinicia o jogador, obstáculos e velocidade
            float pos_x = larguraTela / 2;
            float pos_y = alturaTela - 100;
            inicializarJogador(&jogador, pos_x, pos_y);
            inicializarObstaculos(obstaculos, MAX_OBSTACULOS);
            velocidadeJogo = 3.0f; // Reinicia na velocidade inicial
            tempoUltimaAceleracao = tempoDecorrido; // Mantém tempo acumulado
            framesEntreObstaculos = 180; // Reinicia frequência de obstáculos (3 segundos)
            tempoUltimoAumentoFrequencia = tempoDecorrido;
            frameCont = 0; // Reseta contador para criar obstáculos imediatamente
            
            // Cria obstáculos iniciais imediatamente ao reiniciar
            int quantidade_inicial = (rand() % 3) + 1; // 1, 2 ou 3
            criarMultiplosObstaculos(obstaculos, MAX_OBSTACULOS, alturaTela, quantidade_inicial, ALTURA_HORIZONTE);
            
            fimDeJogo = false;
            vitoria = false;
            cenaVitoria = 0; // Reseta cenas de vitória
            StopSound(somMusicaVitoria); // Para música de vitória
            // NÃO reseta tempoDecorrido e itensColetados
        }
    }

    // ============================================================
    // SEÇÃO: PROCESSAMENTO DE INPUT (TECLAS)
    // ============================================================

    // Tecla P para voltar ao menu sem resetar progresso (pausa)
    if (IsKeyPressed(KEY_P)) {
        *estadoJogo = 0; // de volta ao menu
        StopSound(somCorrida); // Para som de corrida
        StopSound(somMusicaVitoria); // Para música de vitória
        // NÃO define inicializado = false, então mantém tempo e itens coletados
    }

    // Tecla X para resetar o tempo, os itens e voltar ao menu
    if (IsKeyPressed(KEY_X)) {
        *estadoJogo = 0; // de volta ao menu
        StopSound(somCorrida); // Para som de corrida
        StopSound(somMusicaVitoria); // Para música de vitória
        inicializado = false; // Força reinicialização completa (reseta tempo e itens)
    }

    // Tecla ESC para voltar ao menu e resetar tudo (incluindo nickname)
    if (IsKeyPressed(KEY_ESCAPE)) {
        *estadoJogo = 0; // de volta ao menu
        StopSound(somCorrida); // Para som de corrida
        StopSound(somMusicaVitoria); // Para música de vitória
        inicializado = false; // Força reinicialização completa (reseta tempo e itens)
        nickname[0] = '\0'; // Limpa o nickname (fim da run)
    }

    // ============================================================
    // SEÇÃO: RENDERIZAÇÃO
    // ============================================================

    // Cores dos itens (declarado aqui para uso em toda a função)
    Color coresItens[TIPOS_ITENS] = {
        COR_ITEM_PIPOCA,         // Tipo 0 - Pipoca (BOM)
        COR_ITEM_MOEDA,          // Tipo 1 - Moeda (BOM)
        COR_ITEM_VEM,            // Tipo 2 - VEM (BOM)
        COR_ITEM_BOTAO_PARADA,   // Tipo 3 - Botão parada (BOM)
        COR_ITEM_FONE,           // Tipo 4 - Fone (BOM)
        PURPLE,                  // Tipo 5 - Sono (RUIM - aumenta 5 seg)
        DARKGRAY,                // Tipo 6 - Balaclava (RUIM - perde todos)
        ORANGE                   // Tipo 7 - Idosa (RUIM - perde 1 aleatório)
    };

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DesenharFundo(background_jogo, larguraTela, alturaTela);

    // lanes com perspectiva
    // lane esq
    DrawTriangle(
        (Vector2){deslocLaneFundo, alturaTela},
        (Vector2){deslocTopoLane, ALTURA_HORIZONTE},
        (Vector2){deslocTopoLane + larguraTopoLane, ALTURA_HORIZONTE},
        (Color){100, 100, 100, 100}
    );
    DrawTriangle(
        (Vector2){deslocLaneFundo, alturaTela},
        (Vector2){deslocTopoLane + larguraTopoLane, ALTURA_HORIZONTE},
        (Vector2){deslocLaneFundo + larguraLaneFundo, alturaTela},
        (Color){100, 100, 100, 100}
    );

    // lane meio
    DrawTriangle(
        (Vector2){deslocLaneFundo + larguraLaneFundo, alturaTela},      // inferior esquerdo
        (Vector2){deslocTopoLane + larguraTopoLane, ALTURA_HORIZONTE},               // superior esquerdo
        (Vector2){deslocTopoLane + larguraTopoLane * 2, ALTURA_HORIZONTE},           // superior direito
        (Color){80, 80, 80, 100}
    );
    DrawTriangle(
        (Vector2){deslocLaneFundo + larguraLaneFundo, alturaTela},      // inferior esquerdo
        (Vector2){deslocTopoLane + larguraTopoLane * 2, ALTURA_HORIZONTE},           // superior direito
        (Vector2){deslocLaneFundo + larguraLaneFundo * 2, alturaTela},  // inferior direito
        (Color){80, 80, 80, 100}
    );
    
    // lane dir
    DrawTriangle(
        (Vector2){deslocLaneFundo + larguraLaneFundo * 2, alturaTela},  // inferior esquerdo
        (Vector2){deslocTopoLane + larguraTopoLane * 2, ALTURA_HORIZONTE},           // superior esquerdo
        (Vector2){deslocTopoLane + larguraTopoLane * 3, ALTURA_HORIZONTE},           // superior direito
        (Color){100, 100, 100, 100}
    );
    DrawTriangle(
        (Vector2){deslocLaneFundo + larguraLaneFundo * 2, alturaTela},  // inferior esquerdo
        (Vector2){deslocTopoLane + larguraTopoLane * 3, ALTURA_HORIZONTE},           // superior direito
        (Vector2){deslocLaneFundo + larguraLaneFundo * 3, alturaTela},  // inferior direito
        (Color){100, 100, 100, 100}
    );

    // ============================================================
    // SEÇÃO: RENDERIZAÇÃO - OBSTÁCULOS
    // ============================================================

    for (int indiceObstaculo = 0; indiceObstaculo < MAX_OBSTACULOS; indiceObstaculo++) {
        if (!obstaculos[indiceObstaculo].ativo) continue;

        float progresso = CalcularProgresso(obstaculos[indiceObstaculo].pos_y, ALTURA_HORIZONTE, alturaTela);
        float escala = 0.3f + (progresso * 0.7f);

        float largura_escalada = obstaculos[indiceObstaculo].largura * escala;
        float altura_escalada = obstaculos[indiceObstaculo].altura * escala;

        // Calcula posição X com perspectiva
        float x_topo = deslocTopoLane + larguraTopoLane * obstaculos[indiceObstaculo].lane + larguraTopoLane / 2;
        float x_base = deslocLaneFundo + larguraLaneFundo * obstaculos[indiceObstaculo].lane + larguraLaneFundo / 2;
        float posicaoXObstaculo = x_topo + (x_base - x_topo) * progresso;

        if (obstaculos[indiceObstaculo].tipo == OBSTACULO_ONIBUS) {
            Texture2D spriteOnibusAtual = spriteOnibusCentro;
            if (obstaculos[indiceObstaculo].lane == 0) {
                spriteOnibusAtual = spriteOnibusEsquerdo;
            } else if (obstaculos[indiceObstaculo].lane == 2) {
                spriteOnibusAtual = spriteOnibusDireito;
            }

            if (spriteOnibusAtual.id > 0) {
                float sprite_largura = 300.0f * escala;
                float sprite_altura = 300.0f * escala;
                Rectangle origem = {0, 0, (float)spriteOnibusAtual.width, (float)spriteOnibusAtual.height};
                Rectangle destino = {posicaoXObstaculo - sprite_largura / 2, obstaculos[indiceObstaculo].pos_y, sprite_largura, sprite_altura};
                DrawTexturePro(spriteOnibusAtual, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                DrawRectangle(posicaoXObstaculo - largura_escalada / 2, obstaculos[indiceObstaculo].pos_y, largura_escalada, altura_escalada, ORANGE);
            }
        } else if (obstaculos[indiceObstaculo].tipo == OBSTACULO_CATRACA) {
            if (spriteCatraca.id > 0) {
                float sprite_largura_catraca = 100.0f * escala;
                float sprite_altura_catraca = 95.0f * escala;
                Rectangle origem = {0, 0, (float)spriteCatraca.width, (float)spriteCatraca.height};
                Rectangle destino = {posicaoXObstaculo - sprite_largura_catraca / 2, obstaculos[indiceObstaculo].pos_y + 60, sprite_largura_catraca, sprite_altura_catraca};
                DrawTexturePro(spriteCatraca, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                DrawRectangle(posicaoXObstaculo - largura_escalada / 2, obstaculos[indiceObstaculo].pos_y, largura_escalada, altura_escalada, GREEN);
            }
        } else {
            if (spriteLaranja.id > 0) {
                float sprite_largura_laranja = 200.0f * escala;
                float sprite_altura_laranja = 200.0f * escala;
                Rectangle origem = {0, 0, (float)spriteLaranja.width, (float)spriteLaranja.height};
                Rectangle destino = {posicaoXObstaculo - sprite_largura_laranja / 2, obstaculos[indiceObstaculo].pos_y, sprite_largura_laranja, sprite_altura_laranja};
                DrawTexturePro(spriteLaranja, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                Color cor = PURPLE;
                float borda = 8 * escala;
                DrawRectangle(posicaoXObstaculo - largura_escalada / 2, obstaculos[indiceObstaculo].pos_y, largura_escalada, borda, cor);
                DrawRectangle(posicaoXObstaculo - largura_escalada / 2, obstaculos[indiceObstaculo].pos_y, borda, altura_escalada, cor);
                DrawRectangle(posicaoXObstaculo + largura_escalada / 2 - borda, obstaculos[indiceObstaculo].pos_y, borda, altura_escalada, cor);
            }
        }
    }

    // ============================================================
    // RENDERIZAÇÃO - ITENS

    for (int indiceItem = 0; indiceItem < MAX_ITENS; indiceItem++) {
        if (!itens[indiceItem].ativo || itens[indiceItem].coletado) continue;

        float progresso = CalcularProgresso(itens[indiceItem].pos_y, ALTURA_HORIZONTE, alturaTela);
        float escala = 0.3f + (progresso * 0.7f);
        float tamanho_escalado = TAMANHO_BASE_ITEM * escala;

        // Calcula posição X com perspectiva
        float x_topo = deslocTopoLane + (itens[indiceItem].lane * larguraTopoLane) + larguraTopoLane / 2;
        float x_base = deslocLaneFundo + (itens[indiceItem].lane * larguraLaneFundo) + larguraLaneFundo / 2;
        float posicaoXItem = x_topo + (x_base - x_topo) * progresso;

        // Desenha sprite do item
        if (texturasItens[itens[indiceItem].tipo].id > 0) {
            Rectangle origem = {0, 0, (float)texturasItens[itens[indiceItem].tipo].width, (float)texturasItens[itens[indiceItem].tipo].height};

            float proporcaoAspecto = (float)texturasItens[itens[indiceItem].tipo].width / (float)texturasItens[itens[indiceItem].tipo].height;
            float largura_item = tamanho_escalado;
            float altura_item = tamanho_escalado / proporcaoAspecto;

            Rectangle destino = {posicaoXItem - largura_item / 2, itens[indiceItem].pos_y, largura_item, altura_item};
            DrawTexturePro(texturasItens[itens[indiceItem].tipo], origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawCircle(posicaoXItem, itens[indiceItem].pos_y + tamanho_escalado / 2, tamanho_escalado / 2, coresItens[itens[indiceItem].tipo]);
            DrawCircleLines(posicaoXItem, itens[indiceItem].pos_y + tamanho_escalado / 2, tamanho_escalado / 2, BLACK);
        }
    }

    // desenha o jogador com sprites
    if (jogador.deslizando) { // deslizando - usa direção do movimento
        Texture2D spriteAtual = (direcaoJogador < 0) ? spriteDeslizandoDireita : spriteDeslizandoEsquerda;
        if (spriteAtual.id > 0) {
            Rectangle origem = {0, 0, (float)spriteAtual.width, (float)spriteAtual.height};
            // Sprite 150x150 mas hitbox = 40x30
            Rectangle destino = {jogador.pos_x_real - 75, jogador.pos_y_real - 40, 110, 110};
            DrawTexturePro(spriteAtual, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real + 20, 40, 20, RED);
        }
    } else if (jogador.pulando) { // pulando - usa direção do movimento
        Texture2D spriteAtual = (direcaoJogador < 0) ? spritePulandoDireita : spritePulandoEsquerda;
        if (spriteAtual.id > 0) {
            Rectangle origem = {0, 0, (float)spriteAtual.width, (float)spriteAtual.height};
            // Sprite 150x150 mas hitbox mantém 40x50
            Rectangle destino = {jogador.pos_x_real - 75, jogador.pos_y_real - 60, 150, 150};
            DrawTexturePro(spriteAtual, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real - 20, 40, 40, RED);
        }
    } else { // correndo - alterna entre direita e esquerda a cada 0.25s
        Texture2D spriteAtual = quadroAnimacao ? spriteCorrendoEsquerda : spriteCorrendoDireita;
        if (spriteAtual.id > 0) {
            Rectangle origem = {0, 0, (float)spriteAtual.width, (float)spriteAtual.height};
            // Sprite 150x150 mas hitbox mantém 40x50
            Rectangle destino = {jogador.pos_x_real - 75, jogador.pos_y_real - 60, 150, 150};
            DrawTexturePro(spriteAtual, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real, 40, 40, RED);
        }
    }

    if (fimDeJogo) {
        if (vitoria && cenaVitoria == 1 && texturaVitoria1.id > 0) { // Se está na cena de vitória 1
            if (!IsSoundPlaying(somVitoria) && !IsSoundPlaying(somMusicaVitoria)) {
                PlaySound(somMusicaVitoria); // Toca música de vitória em loop após som de vitória acabar
            }
            // Mostra cena de vitória 1
            Rectangle origem = {0, 0, (float)texturaVitoria1.width, (float)texturaVitoria1.height};
            Rectangle destino = {0, 0, (float)larguraTela, (float)alturaTela};
            DrawTexturePro(texturaVitoria1, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
            
            const char* instrucao = "Pressione ENTER para continuar...";
            int instrWidth = MeasureText(instrucao, 20);
            DrawText(instrucao, larguraTela/2 - instrWidth/2, alturaTela - 40, 20, WHITE);
        } else { // Tela normal de game over (após as cenas ou se não for vitória)
            // Overlay semi-transparente para melhorar legibilidade dos textos
            DrawRectangle(0, 0, larguraTela, alturaTela, (Color){0, 0, 0, 100});
            // Offset de 57 pixels (aproximadamente 1.5cm considerando DPI padrão)
            int offsetY = -57;

            // Desenha imagem de fundo conforme o resultado
            if (vitoria) {
                // Usa vitoria_scene2 como fundo da tela de vitória
                if (texturaVitoria2.id > 0) {
                    Rectangle origem = {0, 0, (float)texturaVitoria2.width, (float)texturaVitoria2.height};
                    Rectangle destino = {0, 0, (float)larguraTela, (float)alturaTela};
                    DrawTexturePro(texturaVitoria2, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
                } else {
                    // Alternativa: overlay escuro caso a imagem não carregue
                    DrawRectangle(0, 0, larguraTela, alturaTela, (Color){0, 0, 0, 150});
                }
                
                // Overlay semi-transparente para melhorar legibilidade dos textos
                DrawRectangle(0, 0, larguraTela, alturaTela, (Color){0, 0, 0, 100});
                
                const char* titulo = "VOCÊ VENCEU!";
                int tituloWidth = MeasureText(titulo, 70);
                // Sombra do texto para melhor contraste - sobe mais 30 pixels
                DrawText(titulo, larguraTela/2 - tituloWidth/2 + 3, alturaTela/2 - 150 + offsetY, 70, BLACK);
                DrawText(titulo, larguraTela/2 - tituloWidth/2, alturaTela/2 - 147 + offsetY, 70, GREEN);
                
                const char* subtitulo = "Coletou todos os tipos de itens!";
                int subtituloWidth = MeasureText(subtitulo, 25);
                DrawText(subtitulo, larguraTela/2 - subtituloWidth/2 + 2, alturaTela/2 - 77 + offsetY, 25, BLACK);
                DrawText(subtitulo, larguraTela/2 - subtituloWidth/2, alturaTela/2 - 75 + offsetY, 25, WHITE);

            } else {
                // Usa gameOver.png como fundo quando perde
                if (texturaGameOver.id > 0) {
                    Rectangle origem = {0, 0, (float)texturaGameOver.width, (float)texturaGameOver.height};
                    Rectangle destino = {0, 0, (float)larguraTela, (float)alturaTela};
                    DrawTexturePro(texturaGameOver, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
                } else {
                    // Alternativa: overlay escuro caso a imagem não carregue
                    DrawRectangle(0, 0, larguraTela, alturaTela, (Color){0, 0, 0, 150});
                }
                
                // Overlay semi-transparente para melhorar legibilidade dos textos
                DrawRectangle(0, 0, larguraTela, alturaTela, (Color){0, 0, 0, 100});
            }
            
            // Tempo centralizado
            int minutos = (int)tempoDecorrido / 60;
            int segundos = (int)tempoDecorrido % 60;
            const char* textoTempo = TextFormat("Tempo: %02d:%02d", minutos, segundos);
            int tempoWidth = MeasureText(textoTempo, 40);
            DrawText(textoTempo, larguraTela/2 - tempoWidth/2 + 2, alturaTela/2 + 12 + offsetY, 40, BLACK);
            DrawText(textoTempo, larguraTela/2 - tempoWidth/2, alturaTela/2 + 10 + offsetY, 40, WHITE);
            
            // Label "Itens coletados" centralizado - desce mais 30 pixels
            const char* labelItens = "Itens coletados:";
            int labelWidth = MeasureText(labelItens, 28);
            DrawText(labelItens, larguraTela/2 - labelWidth/2 + 2, alturaTela/2 + 82 + offsetY, 28, BLACK);
            DrawText(labelItens, larguraTela/2 - labelWidth/2, alturaTela/2 + 80 + offsetY, 28, WHITE);
            
            // Ícones dos itens centralizados - desce mais 30 pixels
            // Cada ícone: 48px de largura, espaçamento de 60px entre centros
            // Total: 5 ícones com 4 espaços de 60px = 240px de espaçamento + 48px/2 em cada ponta
            int totalWidth = (TIPOS_ITENS - 1) * 60 + 48; // Largura total: 4*60 + 48 = 288px
            int startX = larguraTela/2 - totalWidth/2 + 24; // +24 para começar no centro do primeiro ícone
            for (int i = 0; i < TIPOS_ITENS; i++) {
                int icon_x = startX + (i * 60);
                int icon_y = alturaTela/2 + 105 + offsetY;
                if (texturasItens[i].id > 0) {
                    Rectangle origem = {0, 0, (float)texturasItens[i].width, (float)texturasItens[i].height};
                    Rectangle destino = {icon_x - 24, icon_y, 48, 48};
                    DrawTexturePro(texturasItens[i], origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
                } else {
                    DrawCircle(icon_x, icon_y + 24, 12, coresItens[i]);
                }
                // Quantidade centralizada abaixo da imagem
                const char* texto = TextFormat("%d", itensColetados[i]);
                int textWidth = MeasureText(texto, 30);
                DrawText(texto, icon_x - textWidth/2 + 2, icon_y + 57, 30, BLACK);
                DrawText(texto, icon_x - textWidth/2, icon_y + 55, 30, WHITE);
            }
            
            // Instruções centralizadas - desce mais 30 pixels
            const char* instrucao1 = "Pressione R para continuar";
            int instr1Width = MeasureText(instrucao1, 25);
            DrawText(instrucao1, larguraTela/2 - instr1Width/2 + 2, alturaTela/2 + 202 + offsetY, 25, BLACK);
            DrawText(instrucao1, larguraTela/2 - instr1Width/2, alturaTela/2 + 200 + offsetY, 25, WHITE);
            
            const char* instrucao2 = "P=Pausar | X=Resetar | ESC=Fechar jogo";
            int instr2Width = MeasureText(instrucao2, 25);
            DrawText(instrucao2, larguraTela/2 - instr2Width/2 + 2, alturaTela/2 + 237 + offsetY, 25, BLACK);
            DrawText(instrucao2, larguraTela/2 - instr2Width/2, alturaTela/2 + 235 + offsetY, 25, WHITE);
        }
    } else {
        // HUD do jogo - Caixa estilizada com tempo e itens (canto superior esquerdo)
        // Cores da caixa
        Color cyanBorder = (Color){102, 255, 255, 255};  // Ciano para bordas
        Color darkPink = (Color){180, 50, 120, 220};      // Rosa escuro para fundo da caixa
        Color yellow = (Color){255, 255, 100, 255};       // Amarelo para ícones
        
        // Dimensões da caixa (reduzidas)
        float larguraCaixa = 350.0f;  // Largura fixa menor
        float alturaCaixa = 70.0f;  // Altura reduzida
        float boxX = 20.0f;  // 20px da borda esquerda
        float boxY = 20.0f;  // 20px do topo
        
        // Desenha caixa principal com bordas arredondadas
        DrawRectangleRounded((Rectangle){boxX, boxY, larguraCaixa, alturaCaixa}, 0.2f, 8, darkPink);
        
        // Bordas externas (múltiplas camadas para efeito 3D)
        DrawRectangleRoundedLines((Rectangle){boxX - 3, boxY - 3, larguraCaixa + 6, alturaCaixa + 6}, 0.2f, 8, cyanBorder);
        DrawRectangleRoundedLines((Rectangle){boxX - 1, boxY - 1, larguraCaixa + 2, alturaCaixa + 2}, 0.2f, 8, (Color){80, 200, 200, 255});
        
        // Linha divisória vertical no meio
        float middleX = boxX + larguraCaixa / 2;
        DrawLineEx((Vector2){middleX, boxY + 10}, (Vector2){middleX, boxY + alturaCaixa - 10}, 3, cyanBorder);
        
        // Listras diagonais decorativas (canto superior direito da caixa)
        for (int i = 0; i < 5; i++) {
            float stripeX = boxX + larguraCaixa * 0.65f + (i * 12);
            DrawLineEx(
                (Vector2){stripeX, boxY + 5}, 
                (Vector2){stripeX + 25, boxY + 30}, 
                2, (Color){cyanBorder.r, cyanBorder.g, cyanBorder.b, 80}
            );
        }
        
        // SEÇÃO ESQUERDA - TEMPO
        float leftSectionX = boxX + 15;
        float leftSectionY = boxY + alturaCaixa / 2;
        
        // Ícone de relógio (representação simples)
        float clockCenterX = leftSectionX + 18;
        float clockCenterY = leftSectionY;
        DrawCircle(clockCenterX, clockCenterY, 14, yellow);
        DrawCircle(clockCenterX, clockCenterY, 12, darkPink);
        DrawLineEx((Vector2){clockCenterX, clockCenterY}, (Vector2){clockCenterX, clockCenterY - 8}, 2, yellow);
        DrawLineEx((Vector2){clockCenterX, clockCenterY}, (Vector2){clockCenterX + 5, clockCenterY}, 2, yellow);
        
        // Tempo em formato MM:SS
        int minutos = (int)tempoDecorrido / 60;
        int segundos = (int)tempoDecorrido % 60;
        const char* tempoTexto = TextFormat("%02d:%02d", minutos, segundos);
        DrawText(tempoTexto, leftSectionX + 42, leftSectionY - 14, 28, cyanBorder);
       
        
        // SEÇÃO DIREITA - ITENS
        float rightSectionX = middleX + 10;
        float rightSectionY = boxY + alturaCaixa / 2;
        
        // Ícone de mochila (representação simples)
        float bagCenterX = rightSectionX + 15;
        float bagCenterY = rightSectionY;
        DrawRectangle(bagCenterX - 10, bagCenterY - 8, 20, 16, yellow);
        DrawRectangle(bagCenterX - 6, bagCenterY - 12, 12, 6, yellow);
        
        // Texto "ITENS X/5" (ajustado para caber na caixa)
        int itensColetadosTotal = 0;
        for (int i = 0; i < 5; i++) {  // Conta apenas itens bons (0-4)
            if (itensColetados[i] > 0) itensColetadosTotal++;
        }
        const char* itensTexto = TextFormat("ITENS %d/5", itensColetadosTotal);
        DrawText(itensTexto, rightSectionX + 35, rightSectionY - 12, 22, cyanBorder);
        
        // Instruções de controle no canto inferior
        DrawText("W=Pular | A=Esq | D=Dir | S=Deslizar", 10, alturaTela - 50, 18, WHITE);
        DrawText("P=Pausar | X=Menu", 10, alturaTela - 28, 18, WHITE);
    }

    EndDrawing();
}

void TelaRanking(int *estadoJogo, int larguraTela, int alturaTela, Texture2D background) {
    // Paleta de cores
    Color pink = (Color){215, 50, 133, 255};      // #d73285 - fundo rosa/magenta
    Color cyan = (Color){102, 255, 255, 255};      // #66FFFF - azul ciano para título
    Color cyanBorder = (Color){150, 255, 255, 255}; // borda do título
    Color green1 = (Color){150, 255, 100, 255};    // #96FF64 - verde claro para linhas
    Color cyan2 = (Color){120, 240, 240, 255};     // ciano para linhas alternadas
    Color white = (Color){255, 255, 255, 255};
    
    BeginDrawing();
    ClearBackground(pink); // Fundo rosa/magenta

    // Desenha fundo se existir (mas com overlay rosa)
    if (background.id > 0) {
        DesenharFundo(background, larguraTela, alturaTela);
        DrawRectangle(0, 0, larguraTela, alturaTela, (Color){0, 0, 0, 155});
    }

    // Dimensões da tabela
    float tableWidth = larguraTela * 0.8f;
    float tableHeight = alturaTela * 0.7f;
    float tableX = (larguraTela - tableWidth) / 2;
    float tableY = alturaTela * 0.15f;
    
    // Borda externa branca/ciano
    DrawRectangleLinesEx((Rectangle){tableX - 5, tableY - 80, tableWidth + 10, tableHeight + 90}, 4, cyanBorder);
    DrawRectangleLinesEx((Rectangle){tableX - 8, tableY - 83, tableWidth + 16, tableHeight + 96}, 2, white);
    
    // Título "RANKING" grande
    float titleSize = larguraTela * 0.12f;
    const char* titleText = "RANKING";
    Vector2 titleMeasure = MeasureTextEx(GetFontDefault(), titleText, titleSize, 4);
    float titleX = larguraTela / 2 - titleMeasure.x / 2;
    float titleY = tableY - 40;
    
    // Sombra do título
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX + 4, titleY + 4}, titleSize, 4, (Color){0, 0, 0, 100});
    // Borda rosa do título
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX - 2, titleY}, titleSize, 4, pink);
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX + 2, titleY}, titleSize, 4, pink);
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX, titleY - 2}, titleSize, 4, pink);
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX, titleY + 2}, titleSize, 4, pink);
    // Texto principal ciano
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX, titleY}, titleSize, 4, cyan);
    
    // Cabeçalhos das colunas
    float headerY = tableY + 75;
    float headerSize = larguraTela * 0.04f;
    float colRankX = tableX + 40;
    float colPlayerX = tableX + tableWidth * 0.25f;
    float colScoreX = tableX + tableWidth * 0.7f;
    
    // Cabeçalhos em branco
    DrawTextEx(GetFontDefault(), "RANK", (Vector2){colRankX, headerY}, headerSize, 2, white);
    DrawTextEx(GetFontDefault(), "PLAYER", (Vector2){colPlayerX, headerY}, headerSize, 2, white);
    DrawTextEx(GetFontDefault(), "SCORE", (Vector2){colScoreX, headerY}, headerSize, 2, white);
    
    // Linha abaixo do cabeçalho
    float lineY = headerY + headerSize + 10;
    DrawRectangle(tableX, lineY, tableWidth, 3, white);
    
    // Desenha top 10 do ranking
    float rowHeight = 40;
    float rowY = lineY + 15;
    float rowSize = larguraTela * 0.03f;
    
    RankingNode* current = ranking.head;
    int rank = 1;
    
    while (current != NULL && rank <= 5) {
        // Alterna cores das linhas (verde e ciano)
        Color rowColor = (rank % 2 == 1) ? green1 : cyan2;
        
        // Fundo da linha
        DrawRectangle(tableX + 5, rowY - 5, tableWidth - 10, rowHeight - 5, rowColor);
        
        // Número do rank
        DrawTextEx(GetFontDefault(), TextFormat("%d.", rank), 
                   (Vector2){colRankX, rowY}, rowSize, 2, (Color){0, 0, 0, 255});
        
        // Nome do jogador (trunca se for muito longo)
        char playerName[21];
        strncpy(playerName, current->name, 20);
        playerName[20] = '\0';
        DrawTextEx(GetFontDefault(), playerName, 
                   (Vector2){colPlayerX, rowY}, rowSize, 2, (Color){0, 0, 0, 255});
        
        // Score (tempo em formato MM:SS.ms)
        int minutos = (int)current->time / 60;
        float segundos = current->time - (minutos * 60);
        DrawTextEx(GetFontDefault(), TextFormat("%02d:%05.2f", minutos, segundos), 
                   (Vector2){colScoreX, rowY}, rowSize, 2, (Color){0, 0, 0, 255});
        
        rowY += rowHeight;
        current = current->next;
        rank++;
    }
    
    // Preenche linhas vazias se houver menos de 5
    while (rank <= 5) {
        Color rowColor = (rank % 2 == 1) ? green1 : cyan2;
        DrawRectangle(tableX + 5, rowY - 5, tableWidth - 10, rowHeight - 5, rowColor);
        
        DrawTextEx(GetFontDefault(), TextFormat("%d.", rank), 
                   (Vector2){colRankX, rowY}, rowSize, 2, (Color){100, 100, 100, 255});
        DrawTextEx(GetFontDefault(), "---", 
                   (Vector2){colPlayerX, rowY}, rowSize, 2, (Color){100, 100, 100, 255});
        DrawTextEx(GetFontDefault(), "---", 
                   (Vector2){colScoreX, rowY}, rowSize, 2, (Color){100, 100, 100, 255});
        
        rowY += rowHeight;
        rank++;
    }
    
    // Botão de voltar
    Color yellow = (Color){254, 255, 153, 255};
    if (DesenharBotao("VOLTAR", larguraTela / 2.0f, alturaTela * 0.9f, larguraTela * 0.2f, alturaTela * 0.08f, larguraTela * 0.04f, cyan, yellow, BLACK, pink)) {
        *estadoJogo = 0;
    }

    EndDrawing();
}

void TelaComoJogar(int *estadoJogo, int larguraTela, int alturaTela, Texture2D background) {
    // Carrega as texturas dos itens colecionáveis (apenas uma vez)
    static Texture2D texturasItensComoJogar[5] = {0};
    static bool texturasCarregadasComoJogar = false;
    
    if (!texturasCarregadasComoJogar) {
        CarregarTexturasItens(texturasItensComoJogar);
        texturasCarregadasComoJogar = true;
    }
    
    // Paleta de cores (mesma da imagem de referência)
    Color pink = (Color){215, 50, 133, 255};       // #d73285 - fundo rosa/magenta
    Color cyan = (Color){102, 255, 255, 255};      // #66FFFF - azul ciano para título
    Color cyanLight = (Color){150, 255, 255, 255}; // ciano claro para texto
    Color yellow = (Color){254, 255, 153, 255};    // #feff99 - amarelo para botões
    Color white = (Color){255, 255, 255, 255};
    
    BeginDrawing();
    ClearBackground(pink); // Fundo rosa/magenta

    // Desenha fundo se existir (mas com overlay rosa)
    if (background.id > 0) {
        DesenharFundo(background, larguraTela, alturaTela);
        DrawRectangle(0, 0, larguraTela, alturaTela, (Color){0, 0, 0, 155});
    }

    // Box principal (ajustado: largura menor, altura maior para o texto "Para vencer")
    float larguraCaixa = larguraTela * 0.78f; // reduzido de 0.85 -> 0.78
    float alturaCaixa = alturaTela * 0.82f; // aumentado de 0.75 -> 0.82
    float boxX = (larguraTela - larguraCaixa) / 2;
    float boxY = alturaTela * 0.06f; // movido mais para cima (0.06)
    
    // Borda externa
    DrawRectangleLinesEx((Rectangle){boxX - 5, boxY - 5, larguraCaixa + 10, alturaCaixa + 10}, 4, cyanLight);
    DrawRectangleLinesEx((Rectangle){boxX - 8, boxY - 8, larguraCaixa + 16, alturaCaixa + 16}, 2, white);
    
    // Fundo do box
    DrawRectangleRounded((Rectangle){boxX, boxY, larguraCaixa, alturaCaixa}, 0.02f, 10, pink);
    
    // Título "COMO JOGAR" grande
    float titleSize = larguraTela * 0.08f;
    const char* titleText = "COMO JOGAR";
    Vector2 titleMeasure = MeasureTextEx(GetFontDefault(), titleText, titleSize, 4);
    float titleX = larguraTela / 2 - titleMeasure.x / 2;
    float titleY = boxY + 15;
    
    // Sombra do título
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX + 3, titleY + 3}, titleSize, 4, (Color){0, 0, 0, 100});
    // Borda rosa do título
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX - 2, titleY}, titleSize, 4, pink);
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX + 2, titleY}, titleSize, 4, pink);
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX, titleY - 2}, titleSize, 4, pink);
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX, titleY + 2}, titleSize, 4, pink);
    // Texto principal ciano
    DrawTextEx(GetFontDefault(), titleText, (Vector2){titleX, titleY}, titleSize, 4, cyan);
    
    // Linha divisória
    float dividerY = titleY + titleSize + 20;
    DrawLine(boxX + 20, dividerY, boxX + larguraCaixa - 20, dividerY, white);
    
    // Conteúdo dividido em duas colunas
    float contentY = dividerY + 25;
    float col1X = boxX + 40;
    float col2X = boxX + larguraCaixa / 2 - 35;
    float textSize = larguraTela * 0.028f;
    float lineHeight = textSize * 1.5f;
    
    // COLUNA 1: CONTROLES
    DrawTextEx(GetFontDefault(), "CONTROLES", (Vector2){col1X, contentY}, textSize * 1.2f, 2, yellow);
    
    float ctrlY = contentY + lineHeight * 1.5f;
    DrawTextEx(GetFontDefault(), "W: PULAR", (Vector2){col1X, ctrlY}, textSize, 2, cyanLight);
    ctrlY += lineHeight;
    DrawTextEx(GetFontDefault(), "A: ESQUERDA", (Vector2){col1X, ctrlY}, textSize, 2, cyanLight);
    ctrlY += lineHeight;
    DrawTextEx(GetFontDefault(), "D: DIREITA", (Vector2){col1X, ctrlY}, textSize, 2, cyanLight);
    ctrlY += lineHeight;
    DrawTextEx(GetFontDefault(), "S: AGACHAR", (Vector2){col1X, ctrlY}, textSize, 2, cyanLight);
    ctrlY += lineHeight * 1.5f;
    DrawTextEx(GetFontDefault(), "P: PAUSAR", (Vector2){col1X, ctrlY}, textSize, 2, cyanLight);
    ctrlY += lineHeight;
    DrawTextEx(GetFontDefault(), "X: RESETAR", (Vector2){col1X, ctrlY}, textSize, 2, cyanLight);
    ctrlY += lineHeight;
    DrawTextEx(GetFontDefault(), "ESC: SAIR", (Vector2){col1X, ctrlY}, textSize, 2, cyanLight);

    // COLUNA 2: OBJETIVO — usar DrawTextRec para caber todo o conteúdo no box
    DrawTextEx(GetFontDefault(), "OBJETIVO", (Vector2){col2X, contentY}, textSize * 1.2f, 2, yellow);

    // Preparar área e reservas para texto, ícones e aviso
    float iconSize = 50.0f;
    float iconSpacing = 60.0f;
    float iconsStartX = col2X - 20.0f;

    float footerReserve = iconSize + 20.0f; // reduzido para dar mais espaço vertical

    // Área do texto do objetivo (coluna da direita)
    float objTextSize = textSize * 0.9f;
    float objBoxPadding = 12.0f; // padding interno do box de objetivo
    float objBoxX = col2X - 15.0f;
    float objBoxY = contentY + lineHeight * 1.5f;
    float objlarguraCaixa = (boxX + larguraCaixa - 30) - objBoxX;
    float objalturaCaixa = (boxY + alturaCaixa) - objBoxY - footerReserve - 5.0f;
    
    // Desenha o retângulo de fundo para o objetivo
    //Rectangle objBgRect = {objBoxX, objBoxY, objlarguraCaixa, objalturaCaixa };
    //DrawRectangleRounded(objBgRect, 0.05f, 10, (Color){0, 0, 0, 60}); // fundo escuro transparente
    //DrawRectangleLinesEx(objBgRect, 2.0f, cyanLight); // borda ciano
    
    // Área de texto interna (com padding)
    Rectangle objRec = {objBoxX, objBoxY, objlarguraCaixa - (objBoxPadding * 2), objalturaCaixa - (objBoxPadding * 2) };
    const char* objetivoFull = "Para fugir ds rotas do ônibus, você precisa de mais do que apenas velocidade. Você deve coletar pelo menos 1 de CADA ITEM BOM espalhado pelos ruas. Cada item coletado te deixa um passo mais perto da vitória. Se você não pegar todos, a corrida não terá fim!";
    // Desenha com quebra automática dentro de objRec
    Vector2 objPos = {objRec.x, objRec.y };
    DesenharTextoQuebrado(GetFontDefault(), objetivoFull, objPos, objTextSize, 2, objRec.width, cyanLight);

    // Posicao Y para desenhar icones: logo abaixo do box de objetivo
    float iconsY = objBoxY + objalturaCaixa + 10.0f;
    
    // Cores dos itens como alternativa (caso as sprites não carreguem)
    Color coresItens[5] = {
        COR_ITEM_PIPOCA,         // Tipo 0 - Pipoca
        COR_ITEM_MOEDA,          // Tipo 1 - Moeda
        COR_ITEM_VEM,            // Tipo 2 - VEM
        COR_ITEM_BOTAO_PARADA,   // Tipo 3 - Botão de parada
        COR_ITEM_FONE            // Tipo 4 - Fone
    };

    for (int i = 0; i < 5; i++) {
        float iconX = iconsStartX + (i * iconSpacing);

        // Desenha a textura do item se carregada
        if (texturasItensComoJogar[i].id > 0) {
            Rectangle origem = {0, 0, (float)texturasItensComoJogar[i].width, (float)texturasItensComoJogar[i].height};
            Rectangle destino = {iconX, iconsY, iconSize, iconSize};
            DrawTexturePro(texturasItensComoJogar[i], origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            // Alternativa: desenha círculo colorido caso a textura não carregue
            DrawCircle(iconX + iconSize/2, iconsY + iconSize/2, iconSize/2 - 2, coresItens[i]);
            DrawCircleLines(iconX + iconSize/2, iconsY + iconSize/2, iconSize/2 - 2, (Color){0, 0, 0, 255});
        }
    }
    
    // Aviso removido por pedido do usuario - nada a desenhar aqui
    (void)0; // placeholder para manter lógica

    // Botão de voltar
    if (DesenharBotao("VOLTAR", larguraTela / 2.0f, boxY + alturaCaixa + 15, larguraTela * 0.2f, alturaTela * 0.08f, larguraTela * 0.04f, cyan, yellow, BLACK, pink)) {
        *estadoJogo = 0;
    }

    EndDrawing();
}