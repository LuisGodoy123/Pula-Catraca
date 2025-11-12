#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "../include/mecanica_principal.h"
#define BASE_ITEM_SIZE 120.0f

// Protótipos das funções
void TelaMenu(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background);
void TelaNickname(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background, char *nickname);
void TelaJogo(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background_jogo, char *nickname);

int main(void) {
    // resolução da janela
    int screenWidth = 800;
    int screenHeight = 600;
    
    // inicializa a janela
    InitWindow(screenWidth, screenHeight, "Pula-Catraca");
    SetTraceLogLevel(LOG_WARNING); // Desabilita mensagens de INFO e DEBUG
    SetTargetFPS(60);

    // carrega imagens de fundo
    Texture2D background_menu = {0};
    Texture2D background_jogo = {0};
    Image fundo_menu = LoadImage("assets/images/fundo_menu.png");
    Image fundo_do_jogo = LoadImage("assets/images/fundo_do_jogo.png");

    if (fundo_menu.data != NULL) {
        background_menu = LoadTextureFromImage(fundo_menu);
        UnloadImage(fundo_menu);
    } else {
        // textura vazia com cor caso não carregue a imagem do menu
        Image tempImg = GenImageColor(screenWidth, screenHeight, (Color){215, 50, 133, 255}); // #d73285
        background_menu = LoadTextureFromImage(tempImg);
        UnloadImage(tempImg);
    }

    if (fundo_do_jogo.data != NULL) {
        background_jogo = LoadTextureFromImage(fundo_do_jogo);
        UnloadImage(fundo_do_jogo);
    } else {
        // textura vazia com cor caso não carregue a imagem do jogo
        Image tempImg = GenImageColor(screenWidth, screenHeight, (Color){96, 80, 125, 255}); // #60507d
        background_jogo = LoadTextureFromImage(tempImg);
        UnloadImage(tempImg);
    }

    int estadoJogo = 0; // 0 = menu, 1 = tela nickname, 2 = jogando
    char nickname[21] = ""; // Armazena até 20 caracteres + null terminator
    
    while (!WindowShouldClose()) {
        if (estadoJogo == 0) {
            TelaMenu(&estadoJogo, screenWidth, screenHeight, background_menu);
        } else if (estadoJogo == 1) {
            TelaNickname(&estadoJogo, screenWidth, screenHeight, background_menu, nickname);
        } else if (estadoJogo == 2) {
            TelaJogo(&estadoJogo, screenWidth, screenHeight, background_jogo, nickname);
        }
    }
    UnloadTexture(background_menu);
    UnloadTexture(background_jogo);
    CloseWindow();
    return 0;
}

void TelaMenu(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background) {
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
        screenHeight * 0.65f,
        btnWidth,
        btnHeight
    };

    Rectangle optionsBtn = {
        screenWidth / 2 - btnWidth / 2,
        screenHeight * 0.75f,
        btnWidth,
        btnHeight
    };

    Vector2 mousePos = GetMousePosition();
    bool hoverPlay = CheckCollisionPointRec(mousePos, playBtn);
    bool hoverOptions = CheckCollisionPointRec(mousePos, optionsBtn);

    // verifica clique no "PLAY"
    if (hoverPlay && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *estadoJogo = 1; // vai p tela do jogo
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

    // botão "OPTIONS"
    DrawRectangleRounded(optionsBtn, 0.3f, 10, hoverOptions ? yellow : blue);
    DrawTextEx(titleFont, "OPTIONS",
               (Vector2){
                   optionsBtn.x + btnWidth / 2 - MeasureTextEx(titleFont, "OPTIONS", fontSize * 0.5f, 2).x / 2,
                   optionsBtn.y + btnHeight / 2 - fontSize * 0.25f
               },
               fontSize * 0.5f, 2, hoverOptions ? pink : green);

    EndDrawing();
}

void TelaNickname(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background, char *nickname) {
    // fonte texto
    Font titleFont = GetFontDefault();
    // paleta
    Color pink = (Color){255, 102, 196, 255};   // #ff66c4
    Color yellow = (Color){254, 255, 153, 255}; // #feff99
    Color blue = (Color){175, 218, 225, 255};   // #afdae1
    Color green = (Color){87, 183, 33, 255};    // #57b721

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

    EndDrawing();
}

void TelaJogo(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background_jogo, char *nickname) {
    static Jogador jogador;
    static Obstaculo obstaculos[MAX_OBSTACULOS];
    static ItemColetavel itens[MAX_ITENS];
    static int itensColetados[TIPOS_ITENS]; // contador de cada tipo coletado
    static bool inicializado = false;
    static int frameCount = 0;
    static int frameCountItens = 0;
    static float velocidadeJogo = 3.0f; // Velocidade inicial: 3.0 m/s
    static float velocidadeMaxima = 30.0f; // Velocidade máxima: 30 m/s
    static float intervaloAceleracao = 30.0f; // Acelera a cada 30 segundos
    static float tempoUltimaAceleracao = 0.0f; // Controla quando acelerar
    static float incrementoVelocidade = 2.0f; // Aumenta 2 m/s a cada intervalo
    static float tempoDecorrido = 0.0f; // Tempo em segundos
    static bool gameOver = false;
    static bool vitoria = false;
    static float tempoMensagemAceleracao = 0.0f; // Para mostrar mensagem de aceleração
    static bool mostrarMensagemAceleracao = false;
    static int direcaoJogador = 0; // -1 = esquerda, 0 = centro, 1 = direita
    static float tempoAnimacao = 0.0f; // Timer para animação de sprites
    static bool frameAnimacao = false; // Alterna entre direito(false) e esquerdo(true)
    
    // Sistema progressivo de obstáculos
    static int framesEntreObstaculos = 180; // Começa com 3 segundos (180 frames)
    static int framesMinimos = 40; // Mínimo de 0.66 segundo (~40 frames)
    static float tempoUltimoAumentoFrequencia = 0.0f; // Controla quando aumentar frequência
    static float intervaloAumentoFrequencia = 10.0f; // Aumenta frequência a cada 10 segundos
    
    // Texturas dos obstáculos (carregadas uma vez)
    static Texture2D spriteOnibusEsquerdo = {0};
    static Texture2D spriteOnibusCentro = {0};
    static Texture2D spriteOnibusDireito = {0};
    static Texture2D spriteCatraca = {0};
    static Texture2D spriteParada = {0};
    static bool spritesCarregadas = false;
    
    // Texturas dos itens colecionáveis (carregadas uma vez)
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
    
    // Perspectiva das lanes
    const float horizon_y = 200.0f;          // linha do horizonte (igual a imagem de fundo)
    float lane_top_divisor = 20.0f;          // maior = topo mais estreito (aumenta angulação)
    float lane_bottom_divisor = 2.0f;        // menor = base mais larga (aumenta angulação)
    float lane_top_extra_offset = 0.25f;      // desloca lanes no topo (positivo move pra direita)
    float lane_bottom_extra_offset = 0.5f;    // desloca lanes na base (positivo move pra direita)

    float lane_width_top = screenWidth / lane_top_divisor;
    float lane_offset_top = (screenWidth - lane_width_top * 3) / 2.0f + lane_top_extra_offset;
    float lane_width_bottom = screenWidth / lane_bottom_divisor;
    float lane_offset_bottom = (screenWidth - lane_width_bottom * 3) / 2.0f + lane_bottom_extra_offset;

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
        
        frameCount = 0;
        frameCountItens = 0;
        velocidadeJogo = 3.0f; // Velocidade inicial
        velocidadeMaxima = 30.0f;
        intervaloAceleracao = 30.0f;
        tempoUltimaAceleracao = 0.0f;
        incrementoVelocidade = 2.0f;
        tempoDecorrido = 0.0f;
        gameOver = false;
        vitoria = false;
        tempoMensagemAceleracao = 0.0f;
        mostrarMensagemAceleracao = false;
        
        // Inicializa sistema progressivo de obstáculos
        framesEntreObstaculos = 180; // Começa com 3 segundos
        tempoUltimoAumentoFrequencia = 0.0f;
        
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

    if (!gameOver) {
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
            abaixar(&jogador);
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
                mostrarMensagemAceleracao = true;
                tempoMensagemAceleracao = 0.0f;
            }
        }

        // Atualiza temporizador da mensagem de aceleração
        if (mostrarMensagemAceleracao) {
            tempoMensagemAceleracao += 1.0f / 60.0f;
            if (tempoMensagemAceleracao >= 2.0f) { // Mostra por 2 segundos
                mostrarMensagemAceleracao = false;
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

        // novos obstaculos com frequencia progressiva
        frameCount++;
        if (frameCount >= framesEntreObstaculos) {
            // Escolhe aleatoriamente: 1, 2 ou 3 obstáculos
            int quantidade = (rand() % 3) + 1; // 1, 2 ou 3
            criarMultiplosObstaculos(obstaculos, MAX_OBSTACULOS, screenHeight, quantidade, horizon_y);
            frameCount = 0;
        }

        // Timer para animação das sprites do jogador (alterna a cada 0.5s)
        tempoAnimacao += GetFrameTime();
        if (tempoAnimacao >= 0.25f) {
            frameAnimacao = !frameAnimacao;
            tempoAnimacao = 0.0f;
        }

        

        // gerar itens colecionáveis a cada 180 frames (3 seg)
        frameCountItens++;
        if (frameCountItens >= 180) {
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
                    }
                }
                // Itens RUINS (tipos 5-7)
                else if (tipo == 5) {
                    // SONO: "você dormiu e perdeu a parada" - aumenta 5 segundos no tempo
                    tempoDecorrido += 5.0f;
                    mostrarMensagemAceleracao = true;
                    tempoMensagemAceleracao = 0.0f;
                    // Marca como coletado para mostrar mensagem customizada
                    itensColetados[tipo]++;
                }
                else if (tipo == 6) {
                    // BALACLAVA: "você foi assaltado e perdeu seus itens" - perde TODOS os itens
                    for (int j = 0; j < 5; j++) { // Apenas itens bons (0-4)
                        itensColetados[j] = 0;
                    }
                    mostrarMensagemAceleracao = true;
                    tempoMensagemAceleracao = 0.0f;
                    itensColetados[tipo]++;
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
                    mostrarMensagemAceleracao = true;
                    tempoMensagemAceleracao = 0.0f;
                    itensColetados[tipo]++;
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
        }

        // colisões (posição sem perspectiva p cálculo)
        for (int i = 0; i < MAX_OBSTACULOS; i++) {
            if (verificarColisao(&jogador, &obstaculos[i], lane_width_bottom, lane_offset_bottom, horizon_y, screenHeight)) {
                gameOver = true;
                break;
            }
        }
    } else {
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
            // NÃO reseta tempoDecorrido e itensColetados
        }
    }

    // Tecla P para voltar ao menu sem resetar progresso (pausa)
    if (IsKeyPressed(KEY_P)) {
        *estadoJogo = 0; // de volta ao menu
        // NÃO define inicializado = false, então mantém tempo e itens coletados
    }

    // Tecla X para resetar a pontuação e voltar ao menu
    if (IsKeyPressed(KEY_X)) {
        *estadoJogo = 0; // de volta ao menu
        inicializado = false; // Força reinicialização completa (reseta tempo e itens)
    }

    // Tecla ESC para voltar ao menu e resetar tudo (incluindo nickname)
    if (IsKeyPressed(KEY_ESCAPE)) {
        *estadoJogo = 0; // de volta ao menu
        inicializado = false; // Força reinicialização completa (reseta tempo e itens)
        nickname[0] = '\0'; // Limpa o nickname (fim da run)
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
    
    // divisórias das lanes
    DrawLine(lane_offset_bottom + lane_width_bottom, screenHeight, lane_offset_top + lane_width_top, horizon_y, DARKGRAY);
    DrawLine(lane_offset_bottom + lane_width_bottom * 2, screenHeight, lane_offset_top + lane_width_top * 2, horizon_y, DARKGRAY);

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
            
            if (obstaculos[i].tipo == 0) {
                // Ônibus alto cheio = obstaculo padrão (desviar com A e D)
                // Seleciona a sprite correta baseada na lane
                Texture2D spriteOnibusAtual = spriteOnibusCentro; // Padrão para lane central (1)
                if (obstaculos[i].lane == 0) {
                    spriteOnibusAtual = spriteOnibusEsquerdo;
                } else if (obstaculos[i].lane == 2) {
                    spriteOnibusAtual = spriteOnibusDireito;
                }
                
                if (spriteOnibusAtual.id > 0) {
                    // Sprite visual 300px (aumentado de 150px base)
                    float sprite_largura = 300.0f * scale;
                    float sprite_altura = 300.0f * scale;
                    Rectangle source = {0, 0, (float)spriteOnibusAtual.width, (float)spriteOnibusAtual.height};
                    Rectangle dest = {obs_x - sprite_largura / 2, obstaculos[i].pos_y, sprite_largura, sprite_altura};
                    DrawTexturePro(spriteOnibusAtual, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
                } else {
                    // Fallback se a textura não carregar
                    DrawRectangle(obs_x - largura_scaled / 2, obstaculos[i].pos_y, largura_scaled, altura_scaled, ORANGE);
                }
            } else if (obstaculos[i].tipo == 1) {
                // catraca de onibus (apenas parte inferior) = obstaculo baixo (pular com W)
                if (spriteCatraca.id > 0) {
                    // Sprite visual de 80px
                    float sprite_largura_catraca = 80.0f * scale;
                    float sprite_altura_catraca = 80.0f * scale;
                    Rectangle source = {0, 0, (float)spriteCatraca.width, (float)spriteCatraca.height};
                    Rectangle dest = {obs_x - sprite_largura_catraca / 2, obstaculos[i].pos_y, sprite_largura_catraca, sprite_altura_catraca};
                    DrawTexturePro(spriteCatraca, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
                } else {
                    // Fallback se a textura não carregar
                    DrawRectangle(
                        obs_x - largura_scaled / 2, 
                        obstaculos[i].pos_y, 
                        largura_scaled, 
                        altura_scaled, 
                        GREEN
                    );
                }
            } else {
                // parada de onibus com teto = obstaculo alto vazado (abaixar com S)
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
                // Usa a textura se carregada
                Rectangle source = {0, 0, (float)texturasItens[itens[i].tipo].width, (float)texturasItens[itens[i].tipo].height};
                Rectangle dest = {
                    item_x - tamanho_scaled / 2, 
                    itens[i].pos_y, 
                    tamanho_scaled, 
                    tamanho_scaled
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
    if (jogador.abaixado) {
        // abaixado - usa sprite correndo mas menor, alterna animação
        Texture2D spriteAtual = frameAnimacao ? spriteCorrendoEsquerda : spriteCorrendoDireita;
        if (spriteAtual.id > 0) {
            Rectangle source = {0, 0, (float)spriteAtual.width, (float)spriteAtual.height};
            // Sprite 150x75 mas hitbox mantém 40x30
            Rectangle dest = {jogador.pos_x_real - 75, jogador.pos_y_real - 12.5f, 150, 75};
            DrawTexturePro(spriteAtual, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real + 20, 40, 20, RED);
        }
    } else if (jogador.pulando) {
        // pulando - usa direção do movimento
        Texture2D spriteAtual = (direcaoJogador < 0) ? spritePulandoEsquerda : spritePulandoDireita;
        if (spriteAtual.id > 0) {
            Rectangle source = {0, 0, (float)spriteAtual.width, (float)spriteAtual.height};
            // Sprite 150x150 mas hitbox mantém 40x50
            Rectangle dest = {jogador.pos_x_real - 75, jogador.pos_y_real - 60, 150, 150};
            DrawTexturePro(spriteAtual, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real - 20, 40, 40, RED);
        }
    } else {
        // correndo - alterna entre direita e esquerda a cada 0.5s
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
        // game over
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 150});
        
        // Título centralizado
        if (vitoria) {
            const char* titulo = "VOCÊ VENCEU!";
            int tituloWidth = MeasureText(titulo, 50);
            DrawText(titulo, screenWidth/2 - tituloWidth/2, screenHeight/2 - 80, 50, GREEN);
            
            const char* subtitulo = "Coletou todos os tipos de itens!";
            int subtituloWidth = MeasureText(subtitulo, 20);
            DrawText(subtitulo, screenWidth/2 - subtituloWidth/2, screenHeight/2 - 30, 20, WHITE);
        } else {
            const char* titulo = "GAME OVER!";
            int tituloWidth = MeasureText(titulo, 50);
            DrawText(titulo, screenWidth/2 - tituloWidth/2, screenHeight/2 - 80, 50, RED);
        }
        
        // Tempo centralizado
        int minutos = (int)tempoDecorrido / 60;
        int segundos = (int)tempoDecorrido % 60;
        const char* textoTempo = TextFormat("Tempo: %02d:%02d", minutos, segundos);
        int tempoWidth = MeasureText(textoTempo, 30);
        DrawText(textoTempo, screenWidth/2 - tempoWidth/2, screenHeight/2 + 10, 30, WHITE);
        
        // Label "Itens coletados" centralizado
        const char* labelItens = "Itens coletados:";
        int labelWidth = MeasureText(labelItens, 20);
        DrawText(labelItens, screenWidth/2 - labelWidth/2, screenHeight/2 + 50, 20, WHITE);
        
        // Ícones dos itens centralizados
        // Cada ícone: 48px de largura, espaçamento de 60px entre centros
        // Total: 5 ícones com 4 espaços de 60px = 240px de espaçamento + 48px/2 em cada ponta
        int totalWidth = (TIPOS_ITENS - 1) * 60 + 48; // Largura total: 4*60 + 48 = 288px
        int startX = screenWidth/2 - totalWidth/2 + 24; // +24 para começar no centro do primeiro ícone
        for (int i = 0; i < TIPOS_ITENS; i++) {
            int icon_x = startX + (i * 60);
            int icon_y = screenHeight/2 + 75;
            if (texturasItens[i].id > 0) {
                Rectangle source = {0, 0, (float)texturasItens[i].width, (float)texturasItens[i].height};
                Rectangle dest = {icon_x - 24, icon_y, 48, 48};
                DrawTexturePro(texturasItens[i], source, dest, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                DrawCircle(icon_x, icon_y + 24, 12, coresItens[i]);
            }
            // Quantidade centralizada abaixo da imagem
            const char* texto = TextFormat("%d", itensColetados[i]);
            int textWidth = MeasureText(texto, 25);
            DrawText(texto, icon_x - textWidth/2, icon_y + 55, 25, WHITE);
        }
        
        // Instruções centralizadas
        const char* instrucao1 = "Pressione R para reiniciar";
        int instr1Width = MeasureText(instrucao1, 20);
        DrawText(instrucao1, screenWidth/2 - instr1Width/2, screenHeight/2 + 165, 20, WHITE);
        
        const char* instrucao2 = "P=Pausar | X=Resetar | ESC=Fechar jogo";
        int instr2Width = MeasureText(instrucao2, 20);
        DrawText(instrucao2, screenWidth/2 - instr2Width/2, screenHeight/2 + 195, 20, WHITE);
    } else {
        // debug e HUD
        // Mostra tempo em minutos:segundos
        int minutos = (int)tempoDecorrido / 60;
        int segundos = (int)tempoDecorrido % 60;
        DrawText(TextFormat("Tempo: %02d:%02d", minutos, segundos), 10, 10, 30, BLACK);
        DrawText(TextFormat("Velocidade: %.1f m/s", velocidadeJogo), 10, 45, 20, BLACK);
        
        // Barra de progresso para próxima aceleração
        if (velocidadeJogo < velocidadeMaxima) {
            float tempoDesdeUltimaAceleracao = tempoDecorrido - tempoUltimaAceleracao;
            float progressoAceleracao = tempoDesdeUltimaAceleracao / intervaloAceleracao;
            if (progressoAceleracao > 1.0f) progressoAceleracao = 1.0f;
            
            int barWidth = 200;
            int barHeight = 15;
            int barX = 10;
            int barY = 72;
            
            // Fundo da barra
            DrawRectangle(barX, barY, barWidth, barHeight, (Color){50, 50, 50, 200});
            // Progresso
            DrawRectangle(barX, barY, (int)(barWidth * progressoAceleracao), barHeight, (Color){255, 200, 0, 255});
            // Borda
            DrawRectangleLines(barX, barY, barWidth, barHeight, BLACK);
            
            // Tempo restante
            int tempoRestante = (int)(intervaloAceleracao - tempoDesdeUltimaAceleracao);
            DrawText(TextFormat("Próxima aceleração: %ds", tempoRestante), barX + barWidth + 10, barY, 15, BLACK);
        } else {
            DrawText("VELOCIDADE MÁXIMA ATINGIDA!", 10, 72, 15, RED);
        }
        
        DrawText(TextFormat("Lane: %d", jogador.lane), 10, 95, 20, BLACK);
        
        // Mostra itens coletados durante o jogo
        DrawText("Itens:", 10, 120, 20, BLACK);
        for (int i = 0; i < TIPOS_ITENS; i++) {
            int icon_x = 20 + (i * 35);
            int icon_y = 140;
            if (texturasItens[i].id > 0) {
                Rectangle source = {0, 0, (float)texturasItens[i].width, (float)texturasItens[i].height};
                Rectangle dest = {icon_x - 24, icon_y, 48, 48}; // Aumentado de 24x24 para 48x48
                DrawTexturePro(texturasItens[i], source, dest, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                DrawCircle(icon_x, icon_y + 10, 12, coresItens[i]);
            }
            DrawText(TextFormat("%d", itensColetados[i]), 15 + (i * 35), 165, 15, itensColetados[i] > 0 ? GREEN : RED);
        }
        
        // Mensagem de aceleração
        if (mostrarMensagemAceleracao) {
            int msgX = screenWidth / 2 - 150;
            int msgY = screenHeight / 2 - 50;
            
            // Fundo semi-transparente
            DrawRectangle(msgX - 20, msgY - 10, 320, 80, (Color){0, 0, 0, 150});
            
            // Texto de aceleração com efeito pulsante
            float alpha = 1.0f - (tempoMensagemAceleracao / 2.0f); // Fade out gradual
            Color textColor = (Color){255, 200, 0, (unsigned char)(255 * alpha)};
            
            if (velocidadeJogo >= velocidadeMaxima) {
                DrawText("VELOCIDADE MÁXIMA!", msgX, msgY, 30, textColor);
                DrawText(TextFormat("%.0f m/s", velocidadeJogo), msgX + 60, msgY + 40, 25, (Color){255, 255, 255, (unsigned char)(255 * alpha)});
            } else {
                DrawText("ACELERANDO!", msgX + 30, msgY, 35, textColor);
                DrawText(TextFormat("Nova velocidade: %.0f m/s", velocidadeJogo), msgX + 10, msgY + 40, 20, (Color){255, 255, 255, (unsigned char)(255 * alpha)});
            }
        }
        
        DrawText("W=Pular | A=Esq | D=Dir | S=Abaixar", 10, screenHeight - 50, 18, BLACK);
        DrawText("P=Pausar | X=Menu", 10, screenHeight - 28, 18, BLACK);
    }

    EndDrawing();
}
