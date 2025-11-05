#include <stddef.h>
#include <stdlib.h>
#include "raylib.h"
#include "../include/mecanica_principal.h"

// Protótipos das funções
void TelaMenu(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background);
void TelaJogo(int *estadoJogo, int screenWidth, int screenHeight);

int main(void) {
    // Inicializa janela com tamanho padrão primeiro
    int screenWidth = 800;
    int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "Pula-Catraca");
    SetTargetFPS(60);

    // Tenta carregar a imagem de fundo
    Texture2D background = {0};
    Image bgImage = LoadImage("assets/fundo_menu.png");
    
    if (bgImage.data != NULL) {
        // Se carregou a imagem, ajusta o tamanho da janela
        screenWidth = bgImage.width;
        screenHeight = bgImage.height;
        SetWindowSize(screenWidth, screenHeight);
        background = LoadTextureFromImage(bgImage);
        UnloadImage(bgImage);
    } else {
        // Se não encontrou a imagem, cria uma textura vazia colorida
        Image tempImg = GenImageColor(screenWidth, screenHeight, SKYBLUE);
        background = LoadTextureFromImage(tempImg);
        UnloadImage(tempImg);
    }

    int estadoJogo = 0; // 0 = Menu, 1 = Jogando

    while (!WindowShouldClose()) {
        if (estadoJogo == 0) {
            TelaMenu(&estadoJogo, screenWidth, screenHeight, background);
        } else if (estadoJogo == 1) {
            TelaJogo(&estadoJogo, screenWidth, screenHeight);
        }
    }

    UnloadTexture(background);
    CloseWindow();

    return 0;
}

void TelaMenu(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background) {
    // Fonte padrão
    Font titleFont = GetFontDefault();

    // Cores da paleta
    Color pink = (Color){255, 102, 196, 255};   // #ff66c4
    Color yellow = (Color){254, 255, 153, 255}; // #feff99
    Color blue = (Color){175, 218, 225, 255};   // #afdae1
    Color green = (Color){87, 183, 33, 255};    // #57b721

    // Botões proporcionais ao tamanho da tela
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

    // Verifica clique no botão PLAY
    if (hoverPlay && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *estadoJogo = 1; // Vai para a tela do jogo
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Desenha o fundo (seja a imagem carregada ou a cor padrão)
    DrawTexture(background, 0, 0, WHITE);

    float fontSize = screenWidth * 0.07f;

    // Botão PLAY
    DrawRectangleRounded(playBtn, 0.3f, 10, hoverPlay ? yellow : blue);
    DrawTextEx(titleFont, "PLAY",
               (Vector2){
                   playBtn.x + btnWidth / 2 - MeasureTextEx(titleFont, "PLAY", fontSize * 0.5f, 2).x / 2,
                   playBtn.y + btnHeight / 2 - fontSize * 0.25f
               },
               fontSize * 0.5f, 2, hoverPlay ? pink : green);

    // Botão OPTIONS
    DrawRectangleRounded(optionsBtn, 0.3f, 10, hoverOptions ? yellow : blue);
    DrawTextEx(titleFont, "OPTIONS",
               (Vector2){
                   optionsBtn.x + btnWidth / 2 - MeasureTextEx(titleFont, "OPTIONS", fontSize * 0.5f, 2).x / 2,
                   optionsBtn.y + btnHeight / 2 - fontSize * 0.25f
               },
               fontSize * 0.5f, 2, hoverOptions ? pink : green);

    EndDrawing();
}

void TelaJogo(int *estadoJogo, int screenWidth, int screenHeight) {
    static Jogador jogador;
    static Obstaculo obstaculos[MAX_OBSTACULOS];
    static bool inicializado = false;
    static int frameCount = 0;
    static float velocidadeJogo = 5.0f;
    static int pontuacao = 0;
    static bool gameOver = false;
    
    // Inicializa o jogador apenas uma vez
    if (!inicializado) {
        float pos_x = screenWidth / 2;
        float pos_y = screenHeight - 100;
        inicializarJogador(&jogador, pos_x, pos_y);
        inicializarObstaculos(obstaculos, MAX_OBSTACULOS);
        frameCount = 0;
        velocidadeJogo = 5.0f;
        pontuacao = 0;
        gameOver = false;
        inicializado = true;
    }

    if (!gameOver) {
        // Entrada do jogador
        if (IsKeyPressed(KEY_W)) {
            pular(&jogador);
        }
        if (IsKeyPressed(KEY_A)) {
            moverEsquerda(&jogador);
        }
        if (IsKeyPressed(KEY_D)) {
            moverDireita(&jogador);
        }
        if (IsKeyPressed(KEY_S)) {
            abaixar(&jogador);
        }

        // Atualiza física
        atualizarFisica(&jogador);

        // Cria novos obstáculos a cada 60 frames (1 segundo)
        frameCount++;
        if (frameCount >= 60) {
            // Escolhe aleatoriamente: 1, 2 ou 3 obstáculos
            int quantidade = (rand() % 3) + 1; // 1, 2 ou 3
            criarMultiplosObstaculos(obstaculos, MAX_OBSTACULOS, screenHeight, quantidade);
            frameCount = 0;
            pontuacao += 10; // Aumenta pontuação
            
            // Aumenta dificuldade gradualmente
            if (velocidadeJogo < 12.0f) {
                velocidadeJogo += 0.2f;
            }
        }

        // Atualiza obstáculos
        atualizarObstaculos(obstaculos, MAX_OBSTACULOS, velocidadeJogo);

        // Calcula posição X baseada na lane
        float lane_width = screenWidth / 3.0f;
        float target_x = lane_width * jogador.lane + lane_width / 2;
        
        // Transição suave entre lanes
        if (jogador.pos_x_real < target_x) {
            jogador.pos_x_real += 10;
            if (jogador.pos_x_real > target_x) jogador.pos_x_real = target_x;
        } else if (jogador.pos_x_real > target_x) {
            jogador.pos_x_real -= 10;
            if (jogador.pos_x_real < target_x) jogador.pos_x_real = target_x;
        }

        // Verifica colisões
        for (int i = 0; i < MAX_OBSTACULOS; i++) {
            if (verificarColisao(&jogador, &obstaculos[i], lane_width)) {
                gameOver = true;
                break;
            }
        }
    } else {
        // Game Over - reinicia ao pressionar R
        if (IsKeyPressed(KEY_R)) {
            inicializado = false;
            gameOver = false;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        *estadoJogo = 0; // Volta ao menu
        inicializado = false;
    }

    // Desenha tudo
    BeginDrawing();
    ClearBackground(SKYBLUE);

    // Desenha as lanes
    float lane_width = screenWidth / 3.0f;
    DrawLine(lane_width, 0, lane_width, screenHeight, DARKGRAY);
    DrawLine(lane_width * 2, 0, lane_width * 2, screenHeight, DARKGRAY);

    // Desenha obstáculos
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos[i].ativo) {
            float obs_x = lane_width * obstaculos[i].lane + lane_width / 2;
            
            // Cor baseada no tipo
            Color cor;
            
            if (obstaculos[i].tipo == 0) {
                // Ônibus alto cheio (laranja)
                cor = ORANGE;
                DrawRectangle(
                    obs_x - obstaculos[i].largura / 2, 
                    obstaculos[i].pos_y, 
                    obstaculos[i].largura, 
                    obstaculos[i].altura, 
                    cor
                );
            } else if (obstaculos[i].tipo == 1) {
                // Obstáculo baixo (verde)
                cor = GREEN;
                DrawRectangle(
                    obs_x - obstaculos[i].largura / 2, 
                    obstaculos[i].pos_y, 
                    obstaculos[i].largura, 
                    obstaculos[i].altura, 
                    cor
                );
            } else {
                // Obstáculo alto vazado (roxo) - desenha só as bordas
                cor = PURPLE;
                // Borda superior
                DrawRectangle(
                    obs_x - obstaculos[i].largura / 2, 
                    obstaculos[i].pos_y, 
                    obstaculos[i].largura, 
                    10, 
                    cor
                );
                // Bordas laterais
                DrawRectangle(
                    obs_x - obstaculos[i].largura / 2, 
                    obstaculos[i].pos_y, 
                    8, 
                    obstaculos[i].altura, 
                    cor
                );
                DrawRectangle(
                    obs_x + obstaculos[i].largura / 2 - 8, 
                    obstaculos[i].pos_y, 
                    8, 
                    obstaculos[i].altura, 
                    cor
                );
            }
        }
    }

    // Desenha o jogador
    Color playerColor = gameOver ? GRAY : RED;
    if (jogador.abaixado) {
        // Desenha abaixado (retângulo mais baixo)
        DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real + 20, 40, 20, playerColor);
    } else {
        // Desenha normal
        DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real, 40, 40, playerColor);
    }

    if (gameOver) {
        // Tela de Game Over
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 150});
        DrawText("GAME OVER!", screenWidth/2 - 150, screenHeight/2 - 50, 50, RED);
        DrawText(TextFormat("Pontuacao: %d", pontuacao), screenWidth/2 - 120, screenHeight/2 + 20, 30, WHITE);
        DrawText("Pressione R para reiniciar", screenWidth/2 - 150, screenHeight/2 + 70, 20, WHITE);
        DrawText("Pressione ESC para o menu", screenWidth/2 - 150, screenHeight/2 + 100, 20, WHITE);
    } else {
        // Informações de debug e HUD
        DrawText(TextFormat("Pontos: %d", pontuacao), 10, 10, 30, BLACK);
        DrawText(TextFormat("Velocidade: %.1f", velocidadeJogo), 10, 45, 20, BLACK);
        DrawText(TextFormat("Lane: %d", jogador.lane), 10, 70, 20, BLACK);
        DrawText("W=Pular | A=Esq | D=Dir | S=Abaixar | ESC=Menu", 10, screenHeight - 30, 20, BLACK);
    }

    EndDrawing();
}
