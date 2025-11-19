#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "../include/mecanica_principal.h"
#include "../include/ranking.h"
#include <stdio.h>
#include <stdbool.h>

// Forward declaration for DrawTextRec (some raylib headers/toolchains may not expose it)
void DrawTextRec(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint);

#define BASE_ITEM_SIZE 120.0f

// Protótipos das funções
void TelaMenu(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background, Sound somMenu);
void TelaNickname(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background, char *nickname, Sound somMenu);
void TelaJogo(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background_jogo, char *nickname, Sound somMenu, Sound somCorrida, Sound somItemBom, Sound somItemRuim, Sound somColisao, Sound somVitoria, Sound somMusicaVitoria);
void TelaRanking(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background);
void TelaComoJogar(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background);

// Ranking (persistente)
static RankingList ranking;

// Último score do jogador (para exibir na tela de ranking)
static float ultimoTempoJogador = 0.0f;
static char ultimoNicknameJogador[50] = "";

// Helper: desenha texto com quebra por largura (word wrap) e retorna a altura ocupada
static float DrawWrappedText(Font font, const char *text, Vector2 pos, float fontSize, float spacing, float wrapWidth, Color tint) {
    // Copia o texto para poder tokenizar
    size_t len = strlen(text);
    char *buf = (char *)malloc(len + 1);
    if (!buf) return 0.0f;
    strcpy(buf, text);

    float y = pos.y;
    char line[1024] = {0};
    char *word = strtok(buf, " ");

    while (word) {
        char candidate[1024] = {0};
        if (line[0] == '\0')
            snprintf(candidate, sizeof(candidate), "%s", word);
        else
            snprintf(candidate, sizeof(candidate), "%s %s", line, word);

        Vector2 measure = MeasureTextEx(font, candidate, fontSize, spacing);
        if (measure.x <= wrapWidth) {
            // cabe na mesma linha
            strncpy(line, candidate, sizeof(line) - 1);
        } else {
            // desenha a linha atual e inicia nova linha com a palavra
            if (line[0] != '\0') {
                DrawTextEx(font, line, (Vector2){pos.x, y}, fontSize, spacing, tint);
                y += fontSize * 1.15f;
            }
            // palavra começa a nova linha
            strncpy(line, word, sizeof(line) - 1);
        }

        word = strtok(NULL, " ");
    }

    if (line[0] != '\0') {
        DrawTextEx(font, line, (Vector2){pos.x, y}, fontSize, spacing, tint);
        y += fontSize * 1.15f;
    }

    free(buf);
    return y - pos.y; // altura ocupada
}

int main(void) {
    // resolução  e init da janela
    int screenWidth = 800;
    int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Pula-Catraca");
    SetTraceLogLevel(LOG_WARNING); // Desabilita mensagens de INFO e DEBUG
    SetTargetFPS(60);
    
    // Inicializa sistema de áudio
    InitAudioDevice();

    // Inicializa e carrega ranking salvo (se existir)
    initRanking(&ranking);
    loadRankingAll(&ranking, "ranking_all.txt");
    
    // Carrega sons
    Sound somMenu = LoadSound("assets/sound/scene_inicial.wav");
    Sound somCorrida = LoadSound("assets/sound/corrida.wav");
    Sound somItemBom = LoadSound("assets/sound/item_bom.wav");
    Sound somItemRuim = LoadSound("assets/sound/item_ruim.wav");
    Sound somColisao = LoadSound("assets/sound/ouch.wav");
    Sound somVitoria = LoadSound("assets/sound/vitoria.wav");
    Sound somMusicaVitoria = LoadSound("assets/sound/musica_vitoria.mp3");
    
    // Ajusta volume dos sons (0.0 a 1.0)
    SetSoundVolume(somMenu, 0.3f);
    SetSoundVolume(somCorrida, 0.2f);
    SetSoundVolume(somItemBom, 0.5f);
    SetSoundVolume(somItemRuim, 0.5f);
    SetSoundVolume(somColisao, 0.6f);
    SetSoundVolume(somVitoria, 0.5f);
    SetSoundVolume(somMusicaVitoria, 0.3f);

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
        Image tempImg = GenImageColor(screenWidth, screenHeight, (Color){215, 50, 133, 255}); // #d73285
        background_menu = LoadTextureFromImage(tempImg);
        UnloadImage(tempImg);
    }

    if (fundo_do_jogo.data != NULL) {
        background_jogo = LoadTextureFromImage(fundo_do_jogo);
        UnloadImage(fundo_do_jogo);
    } else {
        // fundo = cor, caso não carregue a imagem do menu
        Image tempImg = GenImageColor(screenWidth, screenHeight, (Color){96, 80, 125, 255}); // #60507d
        background_jogo = LoadTextureFromImage(tempImg);
        UnloadImage(tempImg);
    }

    int estadoJogo = 0; // 0 = menu, 1 = tela nickname, 2 = jogando, 3 = ranking, 4 = como jogar
    char nickname[21] = ""; // Armazena até 20 caracteres + null terminator
    
    while (!WindowShouldClose()) {
        if (estadoJogo == 0) {
            TelaMenu(&estadoJogo, screenWidth, screenHeight, background_menu, somMenu);
        } else if (estadoJogo == 1) {
            TelaNickname(&estadoJogo, screenWidth, screenHeight, background_menu, nickname, somMenu);
        } else if (estadoJogo == 2) {
            TelaJogo(&estadoJogo, screenWidth, screenHeight, background_jogo, nickname, somMenu, somCorrida, somItemBom, somItemRuim, somColisao, somVitoria, somMusicaVitoria);
        } else if (estadoJogo == 3) {
            TelaRanking(&estadoJogo, screenWidth, screenHeight, background_menu);
        } else if (estadoJogo == 4) {
            TelaComoJogar(&estadoJogo, screenWidth, screenHeight, background_menu);
        }
    }
    // salva ranking completo e top10 antes de sair
    saveRankingAll(&ranking, "ranking_all.txt");
    saveTopTXT(&ranking, "ranking_top10.txt", 10);
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

void TelaMenu(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background, Sound somMenu) {
    // Toca som do menu em loop (se não estiver tocando)
    if (!IsSoundPlaying(somMenu)) {
        PlaySound(somMenu);
    }
    
    // fonte texto
    Font titleFont = GetFontDefault();
    // paleta
    Color pink = (Color){255, 102, 196, 255};   // #ff66c4
    Color yellow = (Color){254, 255, 153, 255}; // #feff99
    Color blue = (Color){175, 218, 225, 255};   // #afdae1
    Color green = (Color){87, 183, 33, 255};    // #57b721

    // botões proporcionais à tela
    float btnWidth = screenWidth * 0.25f;
    float btnHeight = screenHeight * 0.08f;

    Rectangle playBtn = {
        screenWidth / 2 - btnWidth / 2,
        screenHeight * 0.55f,
        btnWidth,
        btnHeight
    };

    Rectangle rankingBtn = {
        screenWidth / 2 - btnWidth / 2,
        screenHeight * 0.65f,
        btnWidth,
        btnHeight
    };

    Rectangle comoJogarBtn = {
        screenWidth / 2 - btnWidth / 2,
        screenHeight * 0.75f,
        btnWidth,
        btnHeight
    };

    Vector2 mousePos = GetMousePosition();
    bool hoverPlay = CheckCollisionPointRec(mousePos, playBtn);
    bool hoverRanking = CheckCollisionPointRec(mousePos, rankingBtn);
    bool hoverComoJogar = CheckCollisionPointRec(mousePos, comoJogarBtn);

    // verifica clique no "PLAY"
    if (hoverPlay && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        StopSound(somMenu); // Para música do menu antes de ir para o jogo
        *estadoJogo = 1; // vai p tela nickname
    }
    
    // verifica clique no "RANKING"
    if (hoverRanking && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *estadoJogo = 3; // vai p tela de ranking
    }
    
    // verifica clique no "COMO JOGAR"
    if (hoverComoJogar && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *estadoJogo = 4; // vai p tela de como jogar
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // fundo redimensionado p caber na janela
    if (background.id > 0) {
        Rectangle source = {0, 0, (float)background.width, (float)background.height};
        Rectangle dest = {0, 0, (float)screenWidth, (float)screenHeight};
        DrawTexturePro(background, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    }

    float fontSize = screenWidth * 0.07f;

    // botão "PLAY"
    DrawRectangleRounded(playBtn, 0.3f, 10, hoverPlay ? yellow : blue);
    DrawTextEx(titleFont, "PLAY",
               (Vector2){
                   playBtn.x + btnWidth / 2 - MeasureTextEx(titleFont, "PLAY", fontSize * 0.5f, 2).x / 2,
                   playBtn.y + btnHeight / 2 - fontSize * 0.25f
               },
               fontSize * 0.5f, 2, hoverPlay ? pink : green);

    // botão "RANKING"
    DrawRectangleRounded(rankingBtn, 0.3f, 10, hoverRanking ? yellow : blue);
    DrawTextEx(titleFont, "RANKING",
               (Vector2){
                   rankingBtn.x + btnWidth / 2 - MeasureTextEx(titleFont, "RANKING", fontSize * 0.5f, 2).x / 2,
                   rankingBtn.y + btnHeight / 2 - fontSize * 0.25f
               },
               fontSize * 0.5f, 2, hoverRanking ? pink : green);

    // botão "COMO JOGAR"
    DrawRectangleRounded(comoJogarBtn, 0.3f, 10, hoverComoJogar ? yellow : blue);
    DrawTextEx(titleFont, "COMO JOGAR",
               (Vector2){
                   comoJogarBtn.x + btnWidth / 2 - MeasureTextEx(titleFont, "COMO JOGAR", fontSize * 0.5f, 2).x / 2,
                   comoJogarBtn.y + btnHeight / 2 - fontSize * 0.25f
               },
               fontSize * 0.5f, 2, hoverComoJogar ? pink : green);

    EndDrawing();
}

void TelaNickname(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background, char *nickname, Sound somMenu) {
    if (!IsSoundPlaying(somMenu)) {
        PlaySound(somMenu); // Para música do menu antes de ir para o jogo
    }
    
    // fonte texto
    Font titleFont = GetFontDefault();
    // paleta
    Color pink = (Color){255, 102, 196, 255};   // #ff66c4
    Color yellow = (Color){254, 255, 153, 255}; // #feff99
    Color blue = (Color){175, 218, 225, 255};   // #afdae1
    Color green = (Color){87, 183, 33, 255};    // #57b721
    Color cyan = (Color){102, 255, 255, 255};   // #66FFFF

    // caixa de texto
    float boxWidth = screenWidth * 0.4f;
    float boxHeight = screenHeight * 0.08f;
    Rectangle inputBox = {
        screenWidth / 2 - boxWidth / 2,
        screenHeight * 0.5f,
        boxWidth,
        boxHeight
    };

    // botão confirmar
    float btnWidth = screenWidth * 0.25f;
    float btnHeight = screenHeight * 0.08f;
    Rectangle confirmBtn = {
        screenWidth / 2 - btnWidth / 2,
        screenHeight * 0.65f,
        btnWidth,
        btnHeight
    };

    Vector2 mousePos = GetMousePosition();
    bool hoverConfirm = CheckCollisionPointRec(mousePos, confirmBtn);

    // captura teclas
    int key = GetCharPressed();
    int nicknameLen = strlen(nickname);
    
    while (key > 0) {
        // aceita letras, números, espaço (32-126 ASCII)
        if ((key >= 32) && (key <= 126) && (nicknameLen < 20)) {
            nickname[nicknameLen] = (char)key;
            nickname[nicknameLen + 1] = '\0';
            nicknameLen++;
        }
        key = GetCharPressed();
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

    // fundo
    if (background.id > 0) {
        Rectangle source = {0, 0, (float)background.width, (float)background.height};
        Rectangle dest = {0, 0, (float)screenWidth, (float)screenHeight};
        DrawTexturePro(background, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    }

    float fontSize = screenWidth * 0.06f;

    // título
    const char *title = "Digite seu nickname:";
    float titleWidth = MeasureTextEx(titleFont, title, fontSize * 0.7f, 2).x;
    DrawTextEx(titleFont, title,
               (Vector2){screenWidth / 2 - titleWidth / 2, screenHeight * 0.35f},
               fontSize * 0.7f, 2, pink);

    // caixa de input
    DrawRectangleRounded(inputBox, 0.3f, 10, blue);
    DrawRectangleLinesEx(inputBox, 3.0f, green);

    // texto digitado
    if (nicknameLen > 0) {
        float textWidth = MeasureTextEx(titleFont, nickname, fontSize * 0.5f, 2).x;
        DrawTextEx(titleFont, nickname,
                   (Vector2){
                       inputBox.x + boxWidth / 2 - textWidth / 2,
                       inputBox.y + boxHeight / 2 - fontSize * 0.25f
                   },
                   fontSize * 0.5f, 2, BLACK);
    } else {
        // placeholder
        const char *placeholder = "Seu nome aqui...";
        float phWidth = MeasureTextEx(titleFont, placeholder, fontSize * 0.4f, 2).x;
        DrawTextEx(titleFont, placeholder,
                   (Vector2){
                       inputBox.x + boxWidth / 2 - phWidth / 2,
                       inputBox.y + boxHeight / 2 - fontSize * 0.2f
                   },
                   fontSize * 0.4f, 2, GRAY);
    }

    // cursor piscando
    static float cursorTimer = 0.0f;
    cursorTimer += GetFrameTime();
    if (((int)(cursorTimer * 2)) % 2 == 0 && nicknameLen < 20) {
        float textWidth = MeasureTextEx(titleFont, nickname, fontSize * 0.5f, 2).x;
        DrawRectangle(
            inputBox.x + boxWidth / 2 + textWidth / 2 + 5,
            inputBox.y + boxHeight * 0.25f,
            2,
            boxHeight * 0.5f,
            BLACK
        );
    }

    // botão confirmar (só ativo se tiver texto)
    if (nicknameLen > 0) {
        DrawRectangleRounded(confirmBtn, 0.3f, 10, hoverConfirm ? yellow : blue);
        DrawTextEx(titleFont, "CONFIRMAR",
                   (Vector2){
                       confirmBtn.x + btnWidth / 2 - MeasureTextEx(titleFont, "CONFIRMAR", fontSize * 0.5f, 2).x / 2,
                       confirmBtn.y + btnHeight / 2 - fontSize * 0.25f
                   },
                   fontSize * 0.5f, 2, hoverConfirm ? pink : green);
    } else {
        DrawRectangleRounded(confirmBtn, 0.3f, 10, GRAY);
        DrawTextEx(titleFont, "CONFIRMAR",
                   (Vector2){
                       confirmBtn.x + btnWidth / 2 - MeasureTextEx(titleFont, "CONFIRMAR", fontSize * 0.5f, 2).x / 2,
                       confirmBtn.y + btnHeight / 2 - fontSize * 0.25f
                   },
                   fontSize * 0.5f, 2, DARKGRAY);
    }

    // Botão de voltar
    btnWidth = screenWidth * 0.2f;
    btnHeight = screenHeight * 0.08f;
    Rectangle backBtn = {
        screenWidth / 2 - btnWidth / 2,
        screenHeight * 0.9f,
        btnWidth,
        btnHeight
    };
    
    mousePos = GetMousePosition();
    bool hoverBack = CheckCollisionPointRec(mousePos, backBtn);
    
    if (hoverBack && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *estadoJogo = 0; // Volta ao menu
    }
    
    // Desenha botão voltar
    DrawRectangleRounded(backBtn, 0.3f, 10, hoverBack ? (Color){254, 255, 153, 255} : cyan);
    float btnTextSize = screenWidth * 0.04f;
    const char* backText = "VOLTAR";
    Vector2 backMeasure = MeasureTextEx(GetFontDefault(), backText, btnTextSize, 2);
    DrawTextEx(GetFontDefault(), backText,
               (Vector2){backBtn.x + btnWidth / 2 - backMeasure.x / 2,
                        backBtn.y + btnHeight / 2 - btnTextSize / 2},
               btnTextSize, 2, hoverBack ? pink : (Color){0, 0, 0, 255});
    
    EndDrawing();
}

void TelaJogo(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background_jogo, char *nickname, Sound somMenu, Sound somCorrida, Sound somItemBom, Sound somItemRuim, Sound somColisao, Sound somVitoria, Sound somMusicaVitoria) {
    static Jogador jogador;
    static Obstaculo obstaculos[MAX_OBSTACULOS];
    static ItemColetavel itens[MAX_ITENS];
    static int itensColetados[TIPOS_ITENS]; // contador de cada tipo coletado
    static bool inicializado = false;
    static int frameCount = 0;
    static int frameCountItens = 0;
    static float velocidadeJogo = 3.0f; // Velocidade inicial: 3.0 m/s
    static float velocidadeMaxima = 8.0f; // Velocidade máxima: 8 m/s
    static float intervaloAceleracao = 30.0f; // Acelera a cada 30 segundos
    static float tempoUltimaAceleracao = 0.0f; // Controla quando acelerar
    static float incrementoVelocidade = 1.0f; // Aumenta 1 m/s a cada intervalo
    static float tempoDecorrido = 0.0f; // Tempo em segundos
    static bool gameOver = false;
    static bool vitoria = false;
    static bool rankingInserido = false;
    static bool primeiraVezJogando = true; // Flag para controlar se é a primeira vez jogando (nunca resetou com X)
    static int estadoMorte = 0; // 0 = jogo normal, 1 = mostra morte_1, 2 = mostra morte_2, 3 = tela game over
    static int direcaoJogador = 0; // -1 = esquerda, 0 = centro, 1 = direita
    static float tempoAnimacao = 0.0f; // Timer para animação de sprites
    static bool frameAnimacao = false; // Alterna entre direito(false) e esquerdo(true)
    
    // Sistema progressivo de obstáculos
    static int framesEntreObstaculos = 120; // Começa com 2 segundos (120 frames)
    static int framesMinimos = 40; // Mínimo de 0.66 segundo (~40 frames)
    static float tempoUltimoAumentoFrequencia = 0.0f; // Controla quando aumentar frequência
    static float intervaloAumentoFrequencia = 10.0f; // Aumenta frequência a cada 10 segundos
    
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
    
    // Perspectiva das lanes - ajustadas para coincidir com as faixas do asfalto
    const float horizon_y = 180.0f;          // linha do horizonte onde a estrada começa
    // Medidas calibradas para coincidir com a imagem de fundo (800x600)
    // No topo (horizonte): as 3 lanes ocupam aproximadamente 25% da largura da tela
    // Na base: ocupam mais que a largura da tela para coincidir com as faixas
    float lane_width_top = screenWidth * 0.083f;      // ~66px por lane no topo (3 lanes = 25% da tela)
    float lane_offset_top = screenWidth * 0.385f;     // começa em 38.5% da tela (centralizado)
    float lane_width_bottom = screenWidth * 0.45f;    // ~360px por lane na base (3 lanes = 135% da tela)
    float lane_offset_bottom = -screenWidth * 0.175f; // começa antes da borda esquerda (-17.5%)

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // fundo redimensionado p caber na janela
    if (background_jogo.id > 0) {
        Rectangle source = {0, 0, (float)background_jogo.width, (float)background_jogo.height};
        Rectangle dest = {0, 0, (float)screenWidth, (float)screenHeight};
        DrawTexturePro(background_jogo, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    }
    
    // inicializa jogador 1 vez
    if (!inicializado) {
        float pos_x = screenWidth / 2;
        float pos_y = screenHeight - 100;
        inicializarJogador(&jogador, pos_x, pos_y);
        inicializarObstaculos(obstaculos, MAX_OBSTACULOS);
        inicializarItens(itens, MAX_ITENS);
        
        // zera itens coletados
        for (int i = 0; i < TIPOS_ITENS; i++) {
            itensColetados[i] = 0;
        }
        
        // Cria obstáculos iniciais imediatamente
        int quantidade_inicial = (rand() % 3) + 1; // 1, 2 ou 3
        criarMultiplosObstaculos(obstaculos, MAX_OBSTACULOS, screenHeight, quantidade_inicial, horizon_y);
        
        inicializado = true;
    }
    
    // Carrega texturas dos obstáculos (apenas uma vez)
    if (!spritesCarregadas) {
        spriteOnibusEsquerdo = LoadTexture("assets/images/onibus_esquerdo.png");
        spriteOnibusCentro = LoadTexture("assets/images/onibus.png");
        spriteOnibusDireito = LoadTexture("assets/images/onibus_direito.png");
        spriteCatraca = LoadTexture("assets/images/catraca.png");
        spriteLaranja = LoadTexture("assets/images/laranja.png");
        spritesCarregadas = true;
    }
    
    // Carrega texturas dos itens (apenas uma vez)
    if (!texturasCarregadas) {
        // Itens BONS (tipos 0-4)
        texturasItens[0] = LoadTexture("assets/images/pipoca.png");      // Tipo 0: YELLOW
        texturasItens[1] = LoadTexture("assets/images/moeda.png");       // Tipo 1: SKYBLUE
        texturasItens[2] = LoadTexture("assets/images/VEM.png");         // Tipo 2: PINK
        texturasItens[3] = LoadTexture("assets/images/botao_parada.png"); // Tipo 3: GOLD
        texturasItens[4] = LoadTexture("assets/images/fone.png");        // Tipo 4: GREEN
        // Itens RUINS (tipos 5-7)
        texturasItens[5] = LoadTexture("assets/images/sono.png");        // Tipo 5: Sono (aumenta 5 seg no tempo)
        texturasItens[6] = LoadTexture("assets/images/balaclava.png");   // Tipo 6: Balaclava (perde todos os itens)
        texturasItens[7] = LoadTexture("assets/images/velha.png");       // Tipo 7: Idosa (perde 1 item aleatório)
        
        texturasCarregadas = true;
    }
    
    // Carrega sprites do jogador (apenas uma vez)
    if (!spritesJogadorCarregadas) {
        spriteCorrendoDireita = LoadTexture("assets/images/correndo_direito.png");
        spriteCorrendoEsquerda = LoadTexture("assets/images/correndo_esquerdo.png");
        spritePulandoDireita = LoadTexture("assets/images/pulando_direita.png");
        spritePulandoEsquerda = LoadTexture("assets/images/pulando_esquerda.png");
        spriteDeslizandoDireita = LoadTexture("assets/images/deslizando_direita.png");
        spriteDeslizandoEsquerda = LoadTexture("assets/images/deslizando_esquerda.png");
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

    if (!gameOver) {
        // Para som do menu e toca som de corrida em loop durante o jogo
        static bool somInicializado = false;
        if (!somInicializado) {
            StopSound(somMenu); // Para música do menu
            somInicializado = true;
        }
        if (!IsSoundPlaying(somCorrida)) {
            PlaySound(somCorrida);
        }
        
        // inputs do jogador
        if (IsKeyPressed(KEY_W)) {
            pular(&jogador);
        }
        if (IsKeyPressed(KEY_A)) {
            moverEsquerda(&jogador);
            direcaoJogador = -1;
        }
        if (IsKeyPressed(KEY_D)) {
            moverDireita(&jogador);
            direcaoJogador = 1;
        }
        if (IsKeyPressed(KEY_S)) {
            deslizar(&jogador);
        }

        // atualiza fisica
        atualizarFisica(&jogador);

        // incrementa o tempo (60 FPS = 1/60 segundo por frame)
        tempoDecorrido += 1.0f / 60.0f;

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

        // Timer para animação das sprites do jogador (alterna a cada 0.5s)
        tempoAnimacao += GetFrameTime();
        if (tempoAnimacao >= 0.25f) {
            frameAnimacao = !frameAnimacao;
            tempoAnimacao = 0.0f;
        }
        
        // novos obstaculos a cada 2seg
        frameCount++;
        if (frameCount >= 120) {
            int quantidade = (rand() % 3) + 1; // aleatorio 1, 2 ou 3
            criarMultiplosObstaculos(obstaculos, MAX_ITENS, screenHeight, quantidade, horizon_y);
            frameCount = 0;
        }
        // novos itens a cada 2seg
        frameCountItens++;
        if (frameCountItens >= 120) {
            criarItem(itens, MAX_ITENS, screenHeight, obstaculos, MAX_OBSTACULOS, horizon_y, itensColetados);
            frameCountItens = 0;
        }

        float dt = GetFrameTime();
        atualizarObstaculos(obstaculos, MAX_OBSTACULOS, velocidadeJogo, horizon_y, screenHeight, dt);
        atualizarItens(itens, MAX_ITENS, velocidadeJogo, horizon_y, screenHeight, dt);

        // posição X baseada na lane com perspectiva
        // O jogador está em uma posição Y específica, então precisa interpolar igual aos obstáculos/itens
        
        // Calcula progress baseado na posição Y do jogador (mesma lógica dos obstáculos)
        float player_progress = (jogador.pos_y_real - horizon_y) / (screenHeight - horizon_y);
        if (player_progress < 0) player_progress = 0;
        if (player_progress > 1) player_progress = 1;
        
        // Posição X interpolada entre topo e base
        float x_top = lane_offset_top + lane_width_top * jogador.lane + lane_width_top / 2;
        float x_bottom = lane_offset_bottom + lane_width_bottom * jogador.lane + lane_width_bottom / 2;
        float target_x = x_top + (x_bottom - x_top) * player_progress;
        
        // transição entre lanes
        if (jogador.pos_x_real < target_x) {
            jogador.pos_x_real += 10;
            if (jogador.pos_x_real > target_x) jogador.pos_x_real = target_x;
        } else if (jogador.pos_x_real > target_x) {
            jogador.pos_x_real -= 10;
            if (jogador.pos_x_real < target_x) jogador.pos_x_real = target_x;
        }

        // verifica coleta de itens
        for (int i = 0; i < MAX_ITENS; i++) {
            if (verificarColeta(&jogador, &itens[i], lane_width_bottom, lane_offset_bottom)) {
                int tipo = itens[i].tipo;
                
                // Itens BONS (tipos 0-4)
                if (tipo >= 0 && tipo <= 4) {
                    // incrementa apenas se ainda não atingiu o limite de 5
                    if (itensColetados[tipo] < 5) {
                        itensColetados[tipo]++;
                        PlaySound(somItemBom); // Toca som de item bom
                    }
                }
                // Itens RUINS (tipos 5-7)
                else if (tipo == 5) {
                    // SONO: "você dormiu e perdeu a parada" - aumenta 5 segundos no tempo
                    tempoDecorrido += 5.0f;
                    // Marca como coletado para mostrar mensagem customizada
                    itensColetados[tipo]++;
                    PlaySound(somItemRuim); // Toca som de item ruim
                }
                else if (tipo == 6) {
                    // BALACLAVA: "você foi assaltado e perdeu seus itens" - perde TODOS os itens
                    for (int j = 0; j < 5; j++) { // Apenas itens bons (0-4)
                        itensColetados[j] = 0;
                    }
                    itensColetados[tipo]++;
                    PlaySound(somItemRuim); // Toca som de item ruim
                }
                else if (tipo == 7) {
                    // IDOSA: "você cedeu o assento e ficou em pé" - perde 1 item aleatório
                    // Procura itens que o jogador possui
                    int itensDisponiveis[5];
                    int quantidadeDisponiveis = 0;
                    for (int j = 0; j < 5; j++) {
                        if (itensColetados[j] > 0) {
                            itensDisponiveis[quantidadeDisponiveis] = j;
                            quantidadeDisponiveis++;
                        }
                    }
                    // Se tiver algum item, remove um aleatório
                    if (quantidadeDisponiveis > 0) {
                        int indiceAleatorio = rand() % quantidadeDisponiveis;
                        int itemRemovido = itensDisponiveis[indiceAleatorio];
                        itensColetados[itemRemovido]--;
                    }
                    itensColetados[tipo]++;
                    PlaySound(somItemRuim); // Toca som de item ruim
                }
            }
        }

        // verifica vitoria (pelo menos 1 item de cada tipo BOM - apenas tipos 0-4)
        if (!vitoria) {
            bool ganhou = true;
            for (int i = 0; i < 5; i++) { // Apenas itens bons (0-4)
                if (itensColetados[i] == 0) {
                    ganhou = false;
                    break;
                }
            }
            vitoria = ganhou;
            // Se o jogador acabou de vencer, insere no ranking (apenas uma vez)
            if (vitoria && !rankingInserido && nickname[0] != '\0') {
                insertRanking(&ranking, nickname, tempoDecorrido);
                saveTopTXT(&ranking, "ranking_top10.txt", 10);
                saveRankingAll(&ranking, "ranking_all.txt");
                rankingInserido = true;
                
                // Salva o último tempo e nickname do jogador para exibir no ranking
                ultimoTempoJogador = tempoDecorrido;
                strncpy(ultimoNicknameJogador, nickname, sizeof(ultimoNicknameJogador) - 1);
                ultimoNicknameJogador[sizeof(ultimoNicknameJogador) - 1] = '\0';
                
                gameOver = true; // Termina o jogo
                cenaVitoria = 1; // Inicia sequência de cenas de vitória
                PlaySound(somVitoria); // Toca som de vitória
                StopSound(somCorrida); // Para som de corrida
            }
        }
        // colisões (posição sem perspectiva p cálculo)
        for (int i = 0; i < MAX_OBSTACULOS; i++) {
            if (verificarColisao(&jogador, &obstaculos[i], lane_width_bottom, lane_offset_bottom, horizon_y, screenHeight)) {
                gameOver = true;
                PlaySound(somColisao); // Toca som de colisão
                StopSound(somCorrida); // Para som de corrida
                // Se é primeira vez jogando, inicia sequência de imagens
                if (primeiraVezJogando) {
                    estadoMorte = 1; // Mostra primeira imagem
                } else {
                    estadoMorte = 3; // Vai direto para tela de game over
                }
                break;
            }
        }
    } else {
        // Controle da sequência de imagens de morte (sem fade)
        if (primeiraVezJogando && !vitoria) {
            if (estadoMorte == 1) {
                // Mostrando morte_1
                if (IsKeyPressed(KEY_ENTER)) {
                    estadoMorte = 2; // Vai para morte_2
                }
            } else if (estadoMorte == 2) {
                // Mostrando morte_2
                if (IsKeyPressed(KEY_ENTER)) {
                    estadoMorte = 3; // Vai para game over
                }
            }
        }
        
        // ENTER para avançar entre cenas de vitória
        if (IsKeyPressed(KEY_ENTER) && vitoria && cenaVitoria > 0 && cenaVitoria < 3) {
            cenaVitoria++; // Avança para próxima cena
        }
        
        // "Game Over" ou "Vitória" - R p reiniciar (mantém tempo e itens coletados)
        if (IsKeyPressed(KEY_R)) {
            // Reinicia o jogador, obstáculos e velocidade
            float pos_x = screenWidth / 2;
            float pos_y = screenHeight - 100;
            inicializarJogador(&jogador, pos_x, pos_y);
            inicializarObstaculos(obstaculos, MAX_OBSTACULOS);
            velocidadeJogo = 3.0f; // Reinicia na velocidade inicial
            tempoUltimaAceleracao = tempoDecorrido; // Mantém tempo acumulado
            framesEntreObstaculos = 180; // Reinicia frequência de obstáculos (3 segundos)
            tempoUltimoAumentoFrequencia = tempoDecorrido;
            frameCount = 0; // Reseta contador para criar obstáculos imediatamente
            
            // Cria obstáculos iniciais imediatamente ao reiniciar
            int quantidade_inicial = (rand() % 3) + 1; // 1, 2 ou 3
            criarMultiplosObstaculos(obstaculos, MAX_OBSTACULOS, screenHeight, quantidade_inicial, horizon_y);
            
            gameOver = false;
            vitoria = false;
            estadoMorte = 0; // Reseta estado de morte
            cenaVitoria = 0; // Reseta cenas de vitória
            StopSound(somMusicaVitoria); // Para música de vitória
            // NÃO reseta tempoDecorrido e itensColetados
        }
    }

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
        primeiraVezJogando = false; // Marca que já não é mais a primeira vez (resetou)
    }

    // Tecla ESC para voltar ao menu e resetar tudo (incluindo nickname)
    if (IsKeyPressed(KEY_ESCAPE)) {
        *estadoJogo = 0; // de volta ao menu
        StopSound(somCorrida); // Para som de corrida
        StopSound(somMusicaVitoria); // Para música de vitória
        inicializado = false; // Força reinicialização completa (reseta tempo e itens)
        nickname[0] = '\0'; // Limpa o nickname (fim da run)
        primeiraVezJogando = false; // Marca que já não é mais a primeira vez (resetou)
    }

    // Cores dos itens (declarado aqui para uso em toda a função)
    Color coresItens[TIPOS_ITENS] = {
        YELLOW,    // Tipo 0 - Pipoca (BOM)
        SKYBLUE,   // Tipo 1 - Moeda (BOM)
        PINK,      // Tipo 2 - VEM (BOM)
        GOLD,      // Tipo 3 - Botão parada (BOM)
        GREEN,     // Tipo 4 - Fone (BOM)
        PURPLE,    // Tipo 5 - Sono (RUIM - aumenta 5 seg)
        DARKGRAY,  // Tipo 6 - Balaclava (RUIM - perde todos)
        ORANGE     // Tipo 7 - Idosa (RUIM - perde 1 aleatório)
    };

    BeginDrawing();
    ClearBackground(SKYBLUE);

    // lanes com perspectiva
    // lane esq
    DrawTriangle(
        (Vector2){lane_offset_bottom, screenHeight},
        (Vector2){lane_offset_top, horizon_y},
        (Vector2){lane_offset_top + lane_width_top, horizon_y},
        (Color){100, 100, 100, 100}
    );
    DrawTriangle(
        (Vector2){lane_offset_bottom, screenHeight},
        (Vector2){lane_offset_top + lane_width_top, horizon_y},
        (Vector2){lane_offset_bottom + lane_width_bottom, screenHeight},
        (Color){100, 100, 100, 100}
    );

    // lane meio
    DrawTriangle(
        (Vector2){lane_offset_bottom + lane_width_bottom, screenHeight},      // inferior esquerdo
        (Vector2){lane_offset_top + lane_width_top, horizon_y},               // superior esquerdo
        (Vector2){lane_offset_top + lane_width_top * 2, horizon_y},           // superior direito
        (Color){80, 80, 80, 100}
    );
    DrawTriangle(
        (Vector2){lane_offset_bottom + lane_width_bottom, screenHeight},      // inferior esquerdo
        (Vector2){lane_offset_top + lane_width_top * 2, horizon_y},           // superior direito
        (Vector2){lane_offset_bottom + lane_width_bottom * 2, screenHeight},  // inferior direito
        (Color){80, 80, 80, 100}
    );
    
    // lane dir
    DrawTriangle(
        (Vector2){lane_offset_bottom + lane_width_bottom * 2, screenHeight},  // inferior esquerdo
        (Vector2){lane_offset_top + lane_width_top * 2, horizon_y},           // superior esquerdo
        (Vector2){lane_offset_top + lane_width_top * 3, horizon_y},           // superior direito
        (Color){100, 100, 100, 100}
    );
    DrawTriangle(
        (Vector2){lane_offset_bottom + lane_width_bottom * 2, screenHeight},  // inferior esquerdo
        (Vector2){lane_offset_top + lane_width_top * 3, horizon_y},           // superior direito
        (Vector2){lane_offset_bottom + lane_width_bottom * 3, screenHeight},  // inferior direito
        (Color){100, 100, 100, 100}
    );

    // caminho dos obstaculos com perspectiva
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos[i].ativo) {
            // Progress baseado na distância entre horizonte e fundo da tela
            float progress = (obstaculos[i].pos_y - horizon_y) / (screenHeight - horizon_y);
            if (progress < 0) progress = 0;
            if (progress > 1) progress = 1;
            
            // menor no topo e maior na base
            float scale = 0.3f + (progress * 0.7f); // De 0.3 a 1.0
            
            // largura e altura em escala
            float largura_scaled = obstaculos[i].largura * scale;
            float altura_scaled = obstaculos[i].altura * scale;
            
            // posição X interpolada entre topo e base
            float x_top = lane_offset_top + lane_width_top * obstaculos[i].lane + lane_width_top / 2;
            float x_bottom = lane_offset_bottom + lane_width_bottom * obstaculos[i].lane + lane_width_bottom / 2;
            float obs_x = x_top + (x_bottom - x_top) * progress;
            
            if (obstaculos[i].tipo == 0) { // Ônibus = obstaculo padrão (desviar com A e D)
                // Seleciona a sprite correta baseada na lane
                Texture2D spriteOnibusAtual = spriteOnibusCentro; // Padrão para lane central (= 1)
                if (obstaculos[i].lane == 0) {
                    spriteOnibusAtual = spriteOnibusEsquerdo;
                } else if (obstaculos[i].lane == 2) {
                    spriteOnibusAtual = spriteOnibusDireito;
                }
                
                if (spriteOnibusAtual.id > 0) {
                    // Sprite visual 300px
                    float sprite_largura = 300.0f * scale;
                    float sprite_altura = 300.0f * scale;
                    Rectangle source = {0, 0, (float)spriteOnibusAtual.width, (float)spriteOnibusAtual.height};
                    Rectangle dest = {obs_x - sprite_largura / 2, obstaculos[i].pos_y, sprite_largura, sprite_altura};
                    DrawTexturePro(spriteOnibusAtual, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
                } else {
                    // Fallback se a textura não carregar
                    DrawRectangle(obs_x - largura_scaled / 2, obstaculos[i].pos_y, largura_scaled, altura_scaled, ORANGE);
                }
            } else if (obstaculos[i].tipo == 1) { // catraca = obstaculo baixo (pular com W)
                if (spriteCatraca.id > 0) { // Sprite 80px por 80px
                    float sprite_largura_catraca = 80.0f * scale;
                    float sprite_altura_catraca = 80.0f * scale;
                    Rectangle source = {0, 0, (float)spriteCatraca.width, (float)spriteCatraca.height};
                    Rectangle dest = {obs_x - sprite_largura_catraca / 2, obstaculos[i].pos_y, sprite_largura_catraca, sprite_altura_catraca};
                    DrawTexturePro(spriteCatraca, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
                } else { // Fallback se a textura não carregar
                    DrawRectangle(
                        obs_x - largura_scaled / 2, 
                        obstaculos[i].pos_y, 
                        largura_scaled, 
                        altura_scaled, 
                        GREEN
                    );
                }
            } else { // IGUAL AO SUBWAY SURFERS: cerca de obra alta = obstaculo alto (abaixar com S)
                if (spriteLaranja.id > 0) {
                    float sprite_largura_laranja = 220.0f * scale;
                    float sprite_altura_laranja = 220.0f * scale;
                    Rectangle source = {0, 0, (float)spriteLaranja.width, (float)spriteLaranja.height};
                    Rectangle dest = {obs_x - sprite_largura_laranja / 2, obstaculos[i].pos_y, sprite_largura_laranja, sprite_altura_laranja};
                    DrawTexturePro(spriteLaranja, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
                } else { // Fallback: desenha estrutura vazada se a textura não carregar
                    Color cor = PURPLE;
                    float border = 8 * scale;
                    DrawRectangle(
                        obs_x - largura_scaled / 2, 
                        obstaculos[i].pos_y, 
                        largura_scaled, 
                        border, 
                        cor
                    );
                    DrawRectangle(
                        obs_x - largura_scaled / 2, 
                        obstaculos[i].pos_y, 
                        border, 
                        altura_scaled, 
                        cor
                    );
                    DrawRectangle(
                        obs_x + largura_scaled / 2 - border, 
                        obstaculos[i].pos_y, 
                        border, 
                        altura_scaled, 
                        cor
                    );
                }
            }
        }
    }

    // itens colecionáveis com perspectiva
    for (int i = 0; i < MAX_ITENS; i++) {
        if (itens[i].ativo && !itens[i].coletado) {
            // Progress baseado na distância entre horizonte e fundo da tela
            float progress = (itens[i].pos_y - horizon_y) / (screenHeight - horizon_y);
            if (progress < 0) progress = 0;
            if (progress > 1) progress = 1;
            
            // escala com perspectiva
            float scale = 0.3f + (progress * 0.7f);
            float tamanho_scaled = BASE_ITEM_SIZE * scale; // 120 pixels (varia de 36px no horizonte a 120px na base)
            
            // posição X com perspectiva
            float x_top = lane_offset_top + (itens[i].lane * lane_width_top) + lane_width_top / 2;
            float x_bottom = lane_offset_bottom + (itens[i].lane * lane_width_bottom) + lane_width_bottom / 2;
            float item_x = x_top + (x_bottom - x_top) * progress;
            
            // Desenha sprite do item
            if (texturasItens[itens[i].tipo].id > 0) {
                // Usa a textura se carregada, mantendo proporção original
                Rectangle source = {0, 0, (float)texturasItens[itens[i].tipo].width, (float)texturasItens[itens[i].tipo].height};
                
                // Calcula dimensões mantendo aspect ratio
                float aspectRatio = (float)texturasItens[itens[i].tipo].width / (float)texturasItens[itens[i].tipo].height;
                float item_width = tamanho_scaled;
                float item_height = tamanho_scaled / aspectRatio;
                
                Rectangle dest = {
                    item_x - item_width / 2, 
                    itens[i].pos_y, 
                    item_width, 
                    item_height
                };
                DrawTexturePro(texturasItens[itens[i].tipo], source, dest, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                // Fallback: desenha círculo colorido se a textura não carregar
                DrawCircle(item_x, itens[i].pos_y + tamanho_scaled / 2, tamanho_scaled / 2, coresItens[itens[i].tipo]);
                DrawCircleLines(item_x, itens[i].pos_y + tamanho_scaled / 2, tamanho_scaled / 2, BLACK);
            }
        }
    }

    // desenha o jogador com sprites
    if (jogador.deslizando) { // deslizando - usa direção do movimento
        Texture2D spriteAtual = (direcaoJogador < 0) ? spriteDeslizandoDireita : spriteDeslizandoEsquerda;
        if (spriteAtual.id > 0) {
            Rectangle source = {0, 0, (float)spriteAtual.width, (float)spriteAtual.height};
            // Sprite 150x150 mas hitbox = 40x30
            Rectangle dest = {jogador.pos_x_real - 75, jogador.pos_y_real - 60, 150, 150};
            DrawTexturePro(spriteAtual, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real + 20, 40, 20, RED);
        }
    } else if (jogador.pulando) { // pulando - usa direção do movimento
        Texture2D spriteAtual = (direcaoJogador < 0) ? spritePulandoDireita : spritePulandoEsquerda;
        if (spriteAtual.id > 0) {
            Rectangle source = {0, 0, (float)spriteAtual.width, (float)spriteAtual.height};
            // Sprite 150x150 mas hitbox mantém 40x50
            Rectangle dest = {jogador.pos_x_real - 75, jogador.pos_y_real - 60, 150, 150};
            DrawTexturePro(spriteAtual, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real - 20, 40, 40, RED);
        }
    } else { // correndo - alterna entre direita e esquerda a cada 0.5s
        Texture2D spriteAtual = frameAnimacao ? spriteCorrendoEsquerda : spriteCorrendoDireita;
        if (spriteAtual.id > 0) {
            Rectangle source = {0, 0, (float)spriteAtual.width, (float)spriteAtual.height};
            // Sprite 150x150 mas hitbox mantém 40x50
            Rectangle dest = {jogador.pos_x_real - 75, jogador.pos_y_real - 60, 150, 150};
            DrawTexturePro(spriteAtual, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real, 40, 40, RED);
        }
    }

    if (gameOver) {
        if (vitoria && cenaVitoria == 1 && texturaVitoria1.id > 0) { // Se está na cena de vitória 1
            if (!IsSoundPlaying(somVitoria) && !IsSoundPlaying(somMusicaVitoria)) {
                PlaySound(somMusicaVitoria); // Toca música de vitória em loop após som de vitória acabar
            }
            // Mostra cena de vitória 1
            Rectangle source = {0, 0, (float)texturaVitoria1.width, (float)texturaVitoria1.height};
            Rectangle dest = {0, 0, (float)screenWidth, (float)screenHeight};
            DrawTexturePro(texturaVitoria1, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
            
            const char* instrucao = "Pressione ENTER para continuar...";
            int instrWidth = MeasureText(instrucao, 20);
            DrawText(instrucao, screenWidth/2 - instrWidth/2, screenHeight - 40, 20, WHITE);
        } else { // Tela normal de game over (após as cenas ou se não for vitória)
            // Overlay semi-transparente para melhorar legibilidade dos textos
            DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 100});
            // Offset de 57 pixels (aproximadamente 1.5cm considerando DPI padrão)
            int offsetY = -57;

            // Desenha imagem de fundo conforme o resultado
            if (vitoria) {
                // Usa vitoria_scene2 como fundo da tela de vitória
                if (texturaVitoria2.id > 0) {
                    Rectangle source = {0, 0, (float)texturaVitoria2.width, (float)texturaVitoria2.height};
                    Rectangle dest = {0, 0, (float)screenWidth, (float)screenHeight};
                    DrawTexturePro(texturaVitoria2, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
                } else {
                    // Fallback: overlay escuro se a imagem não carregar
                    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 150});
                }
                
                // Overlay semi-transparente para melhorar legibilidade dos textos
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 100});
                
                const char* titulo = "VOCÊ VENCEU!";
                int tituloWidth = MeasureText(titulo, 70);
                // Sombra do texto para melhor contraste - sobe mais 30 pixels
                DrawText(titulo, screenWidth/2 - tituloWidth/2 + 3, screenHeight/2 - 150 + offsetY, 70, BLACK);
                DrawText(titulo, screenWidth/2 - tituloWidth/2, screenHeight/2 - 147 + offsetY, 70, GREEN);
                
                const char* subtitulo = "Coletou todos os tipos de itens!";
                int subtituloWidth = MeasureText(subtitulo, 25);
                DrawText(subtitulo, screenWidth/2 - subtituloWidth/2 + 2, screenHeight/2 - 77 + offsetY, 25, BLACK);
                DrawText(subtitulo, screenWidth/2 - subtituloWidth/2, screenHeight/2 - 75 + offsetY, 25, WHITE);

            } else {
                // Usa gameOver.png como fundo quando perde
                if (texturaGameOver.id > 0) {
                    Rectangle source = {0, 0, (float)texturaGameOver.width, (float)texturaGameOver.height};
                    Rectangle dest = {0, 0, (float)screenWidth, (float)screenHeight};
                    DrawTexturePro(texturaGameOver, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
                } else {
                    // Fallback: overlay escuro se a imagem não carregar
                    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 150});
                }
                
                // Overlay semi-transparente para melhorar legibilidade dos textos
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 100});
            }
            
            // Tempo centralizado
            int minutos = (int)tempoDecorrido / 60;
            int segundos = (int)tempoDecorrido % 60;
            const char* textoTempo = TextFormat("Tempo: %02d:%02d", minutos, segundos);
            int tempoWidth = MeasureText(textoTempo, 40);
            DrawText(textoTempo, screenWidth/2 - tempoWidth/2 + 2, screenHeight/2 + 12 + offsetY, 40, BLACK);
            DrawText(textoTempo, screenWidth/2 - tempoWidth/2, screenHeight/2 + 10 + offsetY, 40, WHITE);
            
            // Label "Itens coletados" centralizado - desce mais 30 pixels
            const char* labelItens = "Itens coletados:";
            int labelWidth = MeasureText(labelItens, 28);
            DrawText(labelItens, screenWidth/2 - labelWidth/2 + 2, screenHeight/2 + 92 + offsetY, 28, BLACK);
            DrawText(labelItens, screenWidth/2 - labelWidth/2, screenHeight/2 + 90 + offsetY, 28, WHITE);
            
            // Ícones dos itens centralizados - desce mais 30 pixels
            // Cada ícone: 48px de largura, espaçamento de 60px entre centros
            // Total: 5 ícones com 4 espaços de 60px = 240px de espaçamento + 48px/2 em cada ponta
            int totalWidth = (TIPOS_ITENS - 1) * 60 + 48; // Largura total: 4*60 + 48 = 288px
            int startX = screenWidth/2 - totalWidth/2 + 24; // +24 para começar no centro do primeiro ícone
            for (int i = 0; i < TIPOS_ITENS; i++) {
                int icon_x = startX + (i * 60);
                int icon_y = screenHeight/2 + 105 + offsetY;
                if (texturasItens[i].id > 0) {
                    Rectangle source = {0, 0, (float)texturasItens[i].width, (float)texturasItens[i].height};
                    Rectangle dest = {icon_x - 24, icon_y, 48, 48};
                    DrawTexturePro(texturasItens[i], source, dest, (Vector2){0, 0}, 0.0f, WHITE);
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
            DrawText(instrucao1, screenWidth/2 - instr1Width/2 + 2, screenHeight/2 + 202 + offsetY, 25, BLACK);
            DrawText(instrucao1, screenWidth/2 - instr1Width/2, screenHeight/2 + 200 + offsetY, 25, WHITE);
            
            const char* instrucao2 = "P=Pausar | X=Resetar | ESC=Fechar jogo";
            int instr2Width = MeasureText(instrucao2, 25);
            DrawText(instrucao2, screenWidth/2 - instr2Width/2 + 2, screenHeight/2 + 237 + offsetY, 25, BLACK);
            DrawText(instrucao2, screenWidth/2 - instr2Width/2, screenHeight/2 + 235 + offsetY, 25, WHITE);
        }
    } else {
        // HUD do jogo - Caixa estilizada com tempo e itens (canto superior esquerdo)
        // Cores da caixa
        Color cyanBorder = (Color){102, 255, 255, 255};  // Ciano para bordas
        Color darkPink = (Color){180, 50, 120, 220};      // Rosa escuro para fundo da caixa
        Color yellow = (Color){255, 255, 100, 255};       // Amarelo para ícones
        
        // Dimensões da caixa (reduzidas)
        float boxWidth = 350.0f;  // Largura fixa menor
        float boxHeight = 70.0f;  // Altura reduzida
        float boxX = 20.0f;  // 20px da borda esquerda
        float boxY = 20.0f;  // 20px do topo
        
        // Desenha caixa principal com bordas arredondadas
        DrawRectangleRounded((Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.2f, 8, darkPink);
        
        // Bordas externas (múltiplas camadas para efeito 3D)
        DrawRectangleRoundedLines((Rectangle){boxX - 3, boxY - 3, boxWidth + 6, boxHeight + 6}, 0.2f, 8, cyanBorder);
        DrawRectangleRoundedLines((Rectangle){boxX - 1, boxY - 1, boxWidth + 2, boxHeight + 2}, 0.2f, 8, (Color){80, 200, 200, 255});
        
        // Linha divisória vertical no meio
        float middleX = boxX + boxWidth / 2;
        DrawLineEx((Vector2){middleX, boxY + 10}, (Vector2){middleX, boxY + boxHeight - 10}, 3, cyanBorder);
        
        // Listras diagonais decorativas (canto superior direito da caixa)
        for (int i = 0; i < 5; i++) {
            float stripeX = boxX + boxWidth * 0.65f + (i * 12);
            DrawLineEx(
                (Vector2){stripeX, boxY + 5}, 
                (Vector2){stripeX + 25, boxY + 30}, 
                2, (Color){cyanBorder.r, cyanBorder.g, cyanBorder.b, 80}
            );
        }
        
        // SEÇÃO ESQUERDA - TEMPO
        float leftSectionX = boxX + 15;
        float leftSectionY = boxY + boxHeight / 2;
        
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
        float rightSectionY = boxY + boxHeight / 2;
        
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
        DrawText("W=Pular | A=Esq | D=Dir | S=Deslizar", 10, screenHeight - 50, 18, WHITE);
        DrawText("P=Pausar | X=Menu", 10, screenHeight - 28, 18, WHITE);
    }

    EndDrawing();
}

void TelaRanking(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background) {
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
        Rectangle source = {0, 0, (float)background.width, (float)background.height};
        Rectangle dest = {0, 0, (float)screenWidth, (float)screenHeight};
        DrawTexturePro(background, source, dest, (Vector2){0, 0}, 0.0f, (Color){255, 255, 255, 100});
    }
    
    // Dimensões da tabela
    float tableWidth = screenWidth * 0.8f;
    float tableHeight = screenHeight * 0.7f;
    float tableX = (screenWidth - tableWidth) / 2;
    float tableY = screenHeight * 0.15f;
    
    // Borda externa branca/ciano
    DrawRectangleLinesEx((Rectangle){tableX - 5, tableY - 80, tableWidth + 10, tableHeight + 90}, 4, cyanBorder);
    DrawRectangleLinesEx((Rectangle){tableX - 8, tableY - 83, tableWidth + 16, tableHeight + 96}, 2, white);
    
    // Título "RANKING" grande
    float titleSize = screenWidth * 0.12f;
    const char* titleText = "RANKING";
    Vector2 titleMeasure = MeasureTextEx(GetFontDefault(), titleText, titleSize, 4);
    float titleX = screenWidth / 2 - titleMeasure.x / 2;
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
    float headerSize = screenWidth * 0.04f;
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
    float rowSize = screenWidth * 0.03f;
    
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
    float btnWidth = screenWidth * 0.2f;
    float btnHeight = screenHeight * 0.08f;
    Rectangle backBtn = {
        screenWidth / 2 - btnWidth / 2,
        screenHeight * 0.9f,
        btnWidth,
        btnHeight
    };
    
    Vector2 mousePos = GetMousePosition();
    bool hoverBack = CheckCollisionPointRec(mousePos, backBtn);
    
    if (hoverBack && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *estadoJogo = 0; // Volta ao menu
    }
    
    // Desenha botão voltar
    DrawRectangleRounded(backBtn, 0.3f, 10, hoverBack ? (Color){254, 255, 153, 255} : cyan);
    float btnTextSize = screenWidth * 0.04f;
    const char* backText = "VOLTAR";
    Vector2 backMeasure = MeasureTextEx(GetFontDefault(), backText, btnTextSize, 2);
    DrawTextEx(GetFontDefault(), backText,
               (Vector2){backBtn.x + btnWidth / 2 - backMeasure.x / 2,
                        backBtn.y + btnHeight / 2 - btnTextSize / 2},
               btnTextSize, 2, hoverBack ? pink : (Color){0, 0, 0, 255});
    
    EndDrawing();
}

void TelaComoJogar(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background) {
    // Carrega as texturas dos itens colecionáveis (apenas uma vez)
    static Texture2D texturasItensComoJogar[5] = {0};
    static bool texturasCarregadasComoJogar = false;
    
    if (!texturasCarregadasComoJogar) {
        texturasItensComoJogar[0] = LoadTexture("assets/images/pipoca.png");      // Tipo 0: Pipoca
        texturasItensComoJogar[1] = LoadTexture("assets/images/moeda.png");       // Tipo 1: Moeda
        texturasItensComoJogar[2] = LoadTexture("assets/images/VEM.png");         // Tipo 2: VEM
        texturasItensComoJogar[3] = LoadTexture("assets/images/botao_parada.png"); // Tipo 3: Botão de parada
        texturasItensComoJogar[4] = LoadTexture("assets/images/fone.png");        // Tipo 4: Fone
        texturasCarregadasComoJogar = true;
    }
    
    // Paleta de cores (mesma da imagem de referência)
    Color pink = (Color){215, 50, 133, 255};       // #d73285 - fundo rosa/magenta
    Color cyan = (Color){102, 255, 255, 255};      // #66FFFF - azul ciano para título
    Color cyanLight = (Color){150, 255, 255, 255}; // ciano claro para texto
    Color yellow = (Color){254, 255, 153, 255};    // #feff99 - amarelo para botões
    Color green = (Color){150, 255, 100, 255};     // #96FF64 - verde para texto objetivo
    Color white = (Color){255, 255, 255, 255};
    
    BeginDrawing();
    ClearBackground(pink); // Fundo rosa/magenta
    
    // Desenha fundo se existir (mas com overlay rosa)
    if (background.id > 0) {
        Rectangle source = {0, 0, (float)background.width, (float)background.height};
        Rectangle dest = {0, 0, (float)screenWidth, (float)screenHeight};
        DrawTexturePro(background, source, dest, (Vector2){0, 0}, 0.0f, (Color){255, 255, 255, 100});
    }
    
    // Box principal (ajustado: largura menor, altura maior para o texto "Para vencer")
    float boxWidth = screenWidth * 0.78f; // reduzido de 0.85 -> 0.78
    float boxHeight = screenHeight * 0.82f; // aumentado de 0.75 -> 0.82
    float boxX = (screenWidth - boxWidth) / 2;
    float boxY = screenHeight * 0.06f; // movido mais para cima (0.06)
    
    // Borda externa
    DrawRectangleLinesEx((Rectangle){boxX - 5, boxY - 5, boxWidth + 10, boxHeight + 10}, 4, cyanLight);
    DrawRectangleLinesEx((Rectangle){boxX - 8, boxY - 8, boxWidth + 16, boxHeight + 16}, 2, white);
    
    // Fundo do box
    DrawRectangleRounded((Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.02f, 10, pink);
    
    // Título "COMO JOGAR" grande
    float titleSize = screenWidth * 0.08f;
    const char* titleText = "COMO JOGAR";
    Vector2 titleMeasure = MeasureTextEx(GetFontDefault(), titleText, titleSize, 4);
    float titleX = screenWidth / 2 - titleMeasure.x / 2;
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
    DrawLine(boxX + 20, dividerY, boxX + boxWidth - 20, dividerY, white);
    
    // Conteúdo dividido em duas colunas
    float contentY = dividerY + 25;
    float col1X = boxX + 40;
    float col2X = boxX + boxWidth / 2 + 20;
    float textSize = screenWidth * 0.028f;
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
    
    // COLUNA 2: OBJETIVO — usar DrawTextRec para caber todo o conteúdo no box
    DrawTextEx(GetFontDefault(), "OBJETIVO", (Vector2){col2X, contentY}, textSize * 1.2f, 2, yellow);

    // Preparar área e reservas para texto, ícones e aviso
    float iconSize = 50.0f;
    float iconSpacing = 65.0f;
    float iconsStartX = col2X + 20;

    // Não reservar espaço para aviso (removido) — reservar apenas para ícones
    float footerReserve = iconSize + 20.0f; // reduzido para dar mais espaço vertical

    // Área do texto do objetivo (coluna da direita) — movido para esquerda e aumentado
    float objTextSize = textSize * 0.72f; // reduzido para garantir que caiba
    float objBoxPadding = 12.0f; // padding interno do box de objetivo (aumentado)
    float objBoxX = col2X - 15.0f; // movido 15px para a esquerda
    float objBoxY = contentY + lineHeight * 1.5f;
    float objBoxWidth = (boxX + boxWidth - 30) - objBoxX; // aumentado largura (margem reduzida de 40 para 30)
    float objBoxHeight = (boxY + boxHeight) - objBoxY - footerReserve - 5.0f; // aumentado altura (margem reduzida de 10 para 5)
    
    // Desenha o retângulo de fundo para o objetivo
    Rectangle objBgRect = { objBoxX, objBoxY, objBoxWidth, objBoxHeight };
    DrawRectangleRounded(objBgRect, 0.05f, 10, (Color){0, 0, 0, 60}); // fundo escuro transparente
    DrawRectangleLinesEx(objBgRect, 2.0f, cyanLight); // borda ciano
    
    // Área de texto interna (com padding)
    Rectangle objRec = { objBoxX + objBoxPadding, objBoxY + objBoxPadding, objBoxWidth - (objBoxPadding * 2), objBoxHeight - (objBoxPadding * 2) };
    const char* objetivoFull = "Para vencer o guarda e garantir sua liberdade, voce precisa de mais do que apenas velocidade. Sua fuga sera um sucesso apenas se voce coletar pelo menos 1 de CADA ITEM espalhado pelo cenario. Cada moeda, cartao e fone e um passo mais perto da sua vitoria. Se voce nao pegar tudo, a perseguicao nao tera fim!";
    // Desenha com quebra automática dentro de objRec usando helper (retorna altura usada)
    Vector2 objPos = { objRec.x, objRec.y };
    float usedHeight = DrawWrappedText(GetFontDefault(), objetivoFull, objPos, objTextSize, 2, objRec.width, green);

    // Posicao Y para desenhar icones: logo abaixo do box de objetivo
    float iconsY = objBoxY + objBoxHeight + 10.0f;
    
    // Cores dos itens para fallback (se sprites não carregarem)
    Color coresItens[5] = {
        YELLOW,   // Tipo 0 - Pipoca
        SKYBLUE,  // Tipo 1 - Moeda
        PINK,     // Tipo 2 - VEM
        GOLD,     // Tipo 3 - Botão de parada
        GREEN     // Tipo 4 - Fone
    };
    
    for (int i = 0; i < 5; i++) {
        float iconX = iconsStartX + (i * iconSpacing);
        
        // Desenha a textura do item se carregada
        if (texturasItensComoJogar[i].id > 0) {
            Rectangle source = {0, 0, (float)texturasItensComoJogar[i].width, (float)texturasItensComoJogar[i].height};
            Rectangle dest = {iconX, iconsY, iconSize, iconSize};
            DrawTexturePro(texturasItensComoJogar[i], source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            // Fallback: desenha círculo colorido se a textura não carregar
            DrawCircle(iconX + iconSize/2, iconsY + iconSize/2, iconSize/2 - 2, coresItens[i]);
            DrawCircleLines(iconX + iconSize/2, iconsY + iconSize/2, iconSize/2 - 2, (Color){0, 0, 0, 255});
        }
    }
    
    // Aviso removido por pedido do usuario - nada a desenhar aqui
    (void)0; // placeholder para manter lógica
    
    // Botão de voltar
    float btnWidth = screenWidth * 0.2f;
    float btnHeight = screenHeight * 0.08f;
    Rectangle backBtn = {
        screenWidth / 2 - btnWidth / 2,
        boxY + boxHeight + 15,
        btnWidth,
        btnHeight
    };
    
    Vector2 mousePos = GetMousePosition();
    bool hoverBack = CheckCollisionPointRec(mousePos, backBtn);
    
    if (hoverBack && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *estadoJogo = 0; // Volta ao menu
    }
    
    // Desenha botão voltar
    DrawRectangleRounded(backBtn, 0.3f, 10, hoverBack ? yellow : cyan);
    float btnTextSize = screenWidth * 0.04f;
    const char* backText = "VOLTAR";
    Vector2 backMeasure = MeasureTextEx(GetFontDefault(), backText, btnTextSize, 2);
    DrawTextEx(GetFontDefault(), backText,
               (Vector2){backBtn.x + btnWidth / 2 - backMeasure.x / 2,
                        backBtn.y + btnHeight / 2 - btnTextSize / 2},
               btnTextSize, 2, hoverBack ? pink : (Color){0, 0, 0, 255});
    
    EndDrawing();
}