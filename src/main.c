#include <stddef.h>
#include <stdlib.h>
#include "raylib.h"
#include "../include/mecanica_principal.h"

// Protótipos das funções
void TelaMenu(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background);
void TelaJogo(int *estadoJogo, int screenWidth, int screenHeight);

int main(void) {
    // resolução da janela
    int screenWidth = 800;
    int screenHeight = 600;
    
    // inicializa a janela
    InitWindow(screenWidth, screenHeight, "Pula-Catraca");
    SetTargetFPS(60);

    // carrega imagem de fundo
    Texture2D background = {0};
    Image bgImage = LoadImage("assets/images/fundo_menu.png");
    
    if (bgImage.data != NULL) {
        background = LoadTextureFromImage(bgImage);
        UnloadImage(bgImage);
    } else {
        // cria uma textura vazia colorida caso não carregue a imagem
        Image tempImg = GenImageColor(screenWidth, screenHeight, SKYBLUE);
        background = LoadTextureFromImage(tempImg);
        UnloadImage(tempImg);
    }

    int estadoJogo = 0; // 0 = menu, 1 = jogando

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

void TelaJogo(int *estadoJogo, int screenWidth, int screenHeight) {
    static Jogador jogador;
    static Obstaculo obstaculos[MAX_OBSTACULOS];
    static ItemColetavel itens[MAX_ITENS];
    static int itensColetados[TIPOS_ITENS]; // contador de cada tipo coletado
    static bool inicializado = false;
    static int frameCount = 0;
    static int frameCountItens = 0;
    static float velocidadeJogo = 4.5f;
    static float tempoDecorrido = 0.0f; // Tempo em segundos
    static bool gameOver = false;
    static bool vitoria = false;
    
    // inicializa jogador 1 vez
    if (!inicializado) {
        float pos_x = screenWidth / 2;
        float pos_y = screenHeight - 100;
        inicializarJogador(&jogador, pos_x, pos_y);
        inicializarObstaculos(obstaculos, MAX_OBSTACULOS);
        inicializarItens(itens, MAX_ITENS);
        
        // Zera contador de itens coletados
        for (int i = 0; i < TIPOS_ITENS; i++) {
            itensColetados[i] = 0;
        }
        
        frameCount = 0;
        frameCountItens = 0;
        velocidadeJogo = 4.5f;
        tempoDecorrido = 0.0f;
        gameOver = false;
        vitoria = false;
        inicializado = true;
    }

    if (!gameOver) {
        // inputs do jogador
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

        // atualiza fisica
        atualizarFisica(&jogador);

        // Incrementa o tempo (60 FPS = 1/60 segundo por frame)
        tempoDecorrido += 1.0f / 60.0f;

        // novos obstaculos a 60fps
        frameCount++;
        if (frameCount >= 60) {
            // Escolhe aleatoriamente: 1, 2 ou 3 obstáculos
            int quantidade = (rand() % 3) + 1; // 1, 2 ou 3
            criarMultiplosObstaculos(obstaculos, MAX_OBSTACULOS, screenHeight, quantidade);
            frameCount = 0;
            
            // dificuldade aumentando gradualmente
            if (velocidadeJogo < 10.8f) {
                velocidadeJogo += 0.18f;
            }
        }

        // atualiza obstáculos
        atualizarObstaculos(obstaculos, MAX_OBSTACULOS, velocidadeJogo);

        // Criar itens colecionáveis periodicamente (a cada 90 frames = 1.5 segundos)
        frameCountItens++;
        if (frameCountItens >= 90) {
            criarItem(itens, MAX_ITENS, screenHeight, obstaculos, MAX_OBSTACULOS);
            frameCountItens = 0;
        }

        // Atualiza itens
        atualizarItens(itens, MAX_ITENS, velocidadeJogo);

        // posição X baseada na lane com perspectiva
        float lane_width = screenWidth / 3.0f;
        float target_x = lane_width * jogador.lane + lane_width / 2;
        
        // transição entre lanes
        if (jogador.pos_x_real < target_x) {
            jogador.pos_x_real += 10;
            if (jogador.pos_x_real > target_x) jogador.pos_x_real = target_x;
        } else if (jogador.pos_x_real > target_x) {
            jogador.pos_x_real -= 10;
            if (jogador.pos_x_real < target_x) jogador.pos_x_real = target_x;
        }

        // Verifica coleta de itens
        for (int i = 0; i < MAX_ITENS; i++) {
            if (verificarColeta(&jogador, &itens[i], lane_width)) {
                // Incrementa apenas se ainda não atingiu o limite de 5
                if (itensColetados[itens[i].tipo] < 5) {
                    itensColetados[itens[i].tipo]++;
                }
            }
        }

        // Verifica se ganhou o jogo (pelo menos 1 de cada tipo)
        if (!vitoria) {
            bool ganhou = true;
            for (int i = 0; i < TIPOS_ITENS; i++) {
                if (itensColetados[i] == 0) {
                    ganhou = false;
                    break;
                }
            }
            vitoria = ganhou;
        }

        // colisões (posição sem perspectiva p cálculo)
        for (int i = 0; i < MAX_OBSTACULOS; i++) {
            if (verificarColisao(&jogador, &obstaculos[i], lane_width)) {
                gameOver = true;
                break;
            }
        }
    } else {
        // "Game Over" ou "Vitória" - R p reiniciar (mantém tempo e itens coletados)
        if (IsKeyPressed(KEY_R)) {
            // Reinicia apenas o jogador, obstáculos e velocidade
            float pos_x = screenWidth / 2;
            float pos_y = screenHeight - 100;
            inicializarJogador(&jogador, pos_x, pos_y);
            inicializarObstaculos(obstaculos, MAX_OBSTACULOS);
            velocidadeJogo = 4.5f;
            gameOver = false;
            vitoria = false;
            // NÃO reseta tempoDecorrido e itensColetados
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        *estadoJogo = 0; // de volta ao menu
        inicializado = false; // Força reinicialização completa (reseta tempo e itens)
    }

    // Cores dos itens (declarado aqui para uso em toda a função)
    Color coresItens[TIPOS_ITENS] = {
        YELLOW,    // Tipo 0
        SKYBLUE,   // Tipo 1
        PINK,      // Tipo 2
        GOLD,      // Tipo 3
        GREEN      // Tipo 4
    };

    BeginDrawing();
    ClearBackground(SKYBLUE);

    // lanes com perspectiva
    float lane_width_bottom = screenWidth / 3.0f;
    float lane_width_top = screenWidth / 5.0f;
    float lane_offset_top = (screenWidth - lane_width_top * 3) / 2.0f;
    
    // lane esq
    DrawTriangle(
        (Vector2){0, screenHeight},
        (Vector2){lane_offset_top, 0},
        (Vector2){lane_offset_top + lane_width_top, 0},
        (Color){100, 100, 100, 100}
    );
    DrawTriangle(
        (Vector2){0, screenHeight},
        (Vector2){lane_offset_top + lane_width_top, 0},
        (Vector2){lane_width_bottom, screenHeight},
        (Color){100, 100, 100, 100}
    );

    // lane meio
    DrawTriangle(
        (Vector2){lane_width_bottom, screenHeight},                            // inferior esquerdo
        (Vector2){lane_offset_top + lane_width_top, 0},                       // superior esquerdo
        (Vector2){lane_offset_top + lane_width_top * 2, 0},                   // superior direito
        (Color){80, 80, 80, 100}
    );
    DrawTriangle(
        (Vector2){lane_width_bottom, screenHeight},                            // inferior esquerdo
        (Vector2){lane_offset_top + lane_width_top * 2, 0},                   // superior direito
        (Vector2){lane_width_bottom * 2, screenHeight},                       // inferior direito
        (Color){80, 80, 80, 100}
    );
    
    // lane dir
    DrawTriangle(
        (Vector2){lane_width_bottom * 2, screenHeight},                        // inferior esquerdo
        (Vector2){lane_offset_top + lane_width_top * 2, 0},                   // superior esquerdo
        (Vector2){lane_offset_top + lane_width_top * 3, 0},                   // superior direito
        (Color){100, 100, 100, 100}
    );
    DrawTriangle(
        (Vector2){lane_width_bottom * 2, screenHeight},                        // inferior esquerdo
        (Vector2){lane_offset_top + lane_width_top * 3, 0},                   // superior direito
        (Vector2){screenWidth, screenHeight},                                  // inferior direito
        (Color){100, 100, 100, 100}
    );
    
    // divisórias das lanes
    DrawLine(lane_width_bottom, screenHeight, lane_offset_top + lane_width_top, 0, DARKGRAY);
    DrawLine(lane_width_bottom * 2, screenHeight, lane_offset_top + lane_width_top * 2, 0, DARKGRAY);

    // obstáculos com perspectiva
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos[i].ativo) {
            float progress = (obstaculos[i].pos_y + 100) / (screenHeight + 100);
            if (progress < 0) progress = 0;
            if (progress > 1) progress = 1;
            
            // menor no topo e maior na base
            float scale = 0.3f + (progress * 0.7f); // De 0.3 a 1.0
            
            // largura e altura em escala
            float largura_scaled = obstaculos[i].largura * scale;
            float altura_scaled = obstaculos[i].altura * scale;
            
            // posição X com perspectiva
            float lane_width_top = screenWidth / 5.0f;
            float lane_offset_top = (screenWidth - lane_width_top * 3) / 2.0f;
            float lane_width_bottom = screenWidth / 3.0f;
            
            // posição X interpolada entre topo e base
            float x_top = lane_offset_top + lane_width_top * obstaculos[i].lane + lane_width_top / 2;
            float x_bottom = lane_width_bottom * obstaculos[i].lane + lane_width_bottom / 2;
            float obs_x = x_top + (x_bottom - x_top) * progress;
            
            // cor do obstaculo dependendo do tipo
            Color cor;
            
            if (obstaculos[i].tipo == 0) {
                // Ônibus alto cheio = obstaculo padrão (desviar com A e D)
                cor = ORANGE;
                DrawRectangle(
                    obs_x - largura_scaled / 2, 
                    obstaculos[i].pos_y, 
                    largura_scaled, 
                    altura_scaled, 
                    cor
                );
            } else if (obstaculos[i].tipo == 1) {
                // catraca de onibus (apenas parte inferior) = obstaculo baixo (pular com W)
                cor = GREEN;
                DrawRectangle(
                    obs_x - largura_scaled / 2, 
                    obstaculos[i].pos_y, 
                    largura_scaled, 
                    altura_scaled, 
                    cor
                );
            } else {
                // parada de onibus com teto = obstaculo alto vazado (abaixar com S)
                cor = PURPLE;
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

    // Desenha itens colecionáveis com perspectiva
    for (int i = 0; i < MAX_ITENS; i++) {
        if (itens[i].ativo && !itens[i].coletado) {
            float progress = (itens[i].pos_y + 100) / (screenHeight + 100);
            if (progress < 0) progress = 0;
            if (progress > 1) progress = 1;
            
            // Escala com perspectiva
            float scale = 0.3f + (progress * 0.7f);
            float tamanho_scaled = 30 * scale;
            
            // Posição X com perspectiva
            float lane_width_top = screenWidth / 5.0f;
            float lane_offset_top = (screenWidth - lane_width_top * 3) / 2.0f;
            float lane_width_bottom = screenWidth / 3.0f;
            
            float x_top = lane_offset_top + (itens[i].lane * lane_width_top) + lane_width_top / 2;
            float x_bottom = (itens[i].lane * lane_width_bottom) + lane_width_bottom / 2;
            float item_x = x_top + (x_bottom - x_top) * progress;
            
            // Desenha item como círculo
            DrawCircle(item_x, itens[i].pos_y + tamanho_scaled / 2, tamanho_scaled / 2, coresItens[itens[i].tipo]);
            DrawCircleLines(item_x, itens[i].pos_y + tamanho_scaled / 2, tamanho_scaled / 2, BLACK);
        }
    }

    // desenha o jogador
    Color playerColor = gameOver ? GRAY : RED;
    if (jogador.abaixado) {
        // abaixado
        DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real + 20, 40, 20, playerColor);
    } else {
        // default
        DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real, 40, 40, playerColor);
    }

    if (gameOver) {
        // game over
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 150});
        
        if (vitoria) {
            DrawText("VOCE VENCEU!", screenWidth/2 - 150, screenHeight/2 - 80, 50, GREEN);
            DrawText("Coletou todos os tipos de itens!", screenWidth/2 - 180, screenHeight/2 - 30, 20, WHITE);
        } else {
            DrawText("GAME OVER!", screenWidth/2 - 150, screenHeight/2 - 80, 50, RED);
        }
        
        // Mostra tempo em minutos:segundos
        int minutos = (int)tempoDecorrido / 60;
        int segundos = (int)tempoDecorrido % 60;
        DrawText(TextFormat("Tempo: %02d:%02d", minutos, segundos), screenWidth/2 - 100, screenHeight/2 + 10, 30, WHITE);
        
        // Mostra contagem de itens coletados
        DrawText("Itens coletados:", screenWidth/2 - 100, screenHeight/2 + 50, 20, WHITE);
        for (int i = 0; i < TIPOS_ITENS; i++) {
            DrawCircle(screenWidth/2 - 100 + (i * 40), screenHeight/2 + 85, 12, coresItens[i]);
            DrawText(TextFormat("%d", itensColetados[i]), screenWidth/2 - 95 + (i * 40), screenHeight/2 + 95, 15, WHITE);
        }
        
        DrawText("Pressione R para reiniciar", screenWidth/2 - 150, screenHeight/2 + 125, 20, WHITE);
        DrawText("Pressione ESC para o menu", screenWidth/2 - 150, screenHeight/2 + 155, 20, WHITE);
    } else {
        // debug e HUD
        // Mostra tempo em minutos:segundos
        int minutos = (int)tempoDecorrido / 60;
        int segundos = (int)tempoDecorrido % 60;
        DrawText(TextFormat("Tempo: %02d:%02d", minutos, segundos), 10, 10, 30, BLACK);
        DrawText(TextFormat("Velocidade: %.1f", velocidadeJogo), 10, 45, 20, BLACK);
        DrawText(TextFormat("Lane: %d", jogador.lane), 10, 70, 20, BLACK);
        
        // Mostra itens coletados durante o jogo
        DrawText("Itens:", 10, 100, 20, BLACK);
        for (int i = 0; i < TIPOS_ITENS; i++) {
            DrawCircle(20 + (i * 35), 130, 12, coresItens[i]);
            DrawText(TextFormat("%d", itensColetados[i]), 15 + (i * 35), 145, 15, itensColetados[i] > 0 ? GREEN : RED);
        }
        
        DrawText("W=Pular | A=Esq | D=Dir | S=Abaixar | ESC=Menu", 10, screenHeight - 30, 20, BLACK);
    }

    EndDrawing();
}
