#include <stddef.h>
#include <stdlib.h>
#include "raylib.h"
#include "../include/mecanica_principal.h"

// Protótipos das funções
void TelaMenu(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background);
void TelaJogo(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background_jogo);

int main(void) {
    // resolução da janela
    int screenWidth = 800;
    int screenHeight = 600;
    
    // inicializa a janela
    InitWindow(screenWidth, screenHeight, "Pula-Catraca");
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

    int estadoJogo = 0; // 0 = menu, 1 = jogando
    while (!WindowShouldClose()) {
        if (estadoJogo == 0) {
            TelaMenu(&estadoJogo, screenWidth, screenHeight, background_menu);
        } else if (estadoJogo == 1) {
            TelaJogo(&estadoJogo, screenWidth, screenHeight, background_jogo);
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

void TelaJogo(int *estadoJogo, int screenWidth, int screenHeight, Texture2D background_jogo) {
    static Jogador jogador;
    static Obstaculo obstaculos[MAX_OBSTACULOS];
    static ItemColetavel itens[MAX_ITENS];
    static int itensColetados[TIPOS_ITENS]; // contador de cada tipo coletado
    static bool inicializado = false;
    static int frameCount = 0;
    static int frameCountItens = 0;
    static float velocidadeJogo = 3.5f; // Velocidade inicial: 3.5 m/s (bem mais lento)
    static float velocidadeMaxima = 45.0f; // Velocidade máxima: 45 m/s (rápido)
    static float intervaloAceleracao = 30.0f; // Acelera a cada 30 segundos
    static float tempoUltimaAceleracao = 0.0f; // Controla quando acelerar
    static float incrementoVelocidade = 4.0f; // Aumenta 4 m/s a cada intervalo
    static float tempoDecorrido = 0.0f; // Tempo em segundos
    static bool gameOver = false;
    static bool vitoria = false;
    static float tempoMensagemAceleracao = 0.0f; // Para mostrar mensagem de aceleração
    static bool mostrarMensagemAceleracao = false;
    
    // Texturas dos itens colecionáveis (carregadas uma vez)
    static Texture2D texturasItens[TIPOS_ITENS] = {0};
    static bool texturasCarregadas = false;
    
    // Constantes de perspectiva
    const float horizon_y = 200.0f;

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
        velocidadeJogo = 3.5f; // Velocidade inicial (lenta)
        velocidadeMaxima = 45.0f;
        intervaloAceleracao = 30.0f;
        tempoUltimaAceleracao = 0.0f;
        incrementoVelocidade = 4.0f;
        tempoDecorrido = 0.0f;
        gameOver = false;
        vitoria = false;
        tempoMensagemAceleracao = 0.0f;
        mostrarMensagemAceleracao = false;
        inicializado = true;
    }
    
    // Carrega texturas dos itens (apenas uma vez)
    if (!texturasCarregadas) {
        texturasItens[0] = LoadTexture("assets/images/pipoca.png");      // Tipo 0: YELLOW
        texturasItens[1] = LoadTexture("assets/images/moeda.png");       // Tipo 1: SKYBLUE
        texturasItens[2] = LoadTexture("assets/images/VEM.png");         // Tipo 2: PINK
        texturasItens[3] = LoadTexture("assets/images/botao_parada.png"); // Tipo 3: GOLD
        texturasItens[4] = LoadTexture("assets/images/fone.png");        // Tipo 4: GREEN
        texturasCarregadas = true;
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

        // novos obstaculos a 60fps
        frameCount++;
        if (frameCount >= 60) {
            // Escolhe aleatoriamente: 1, 2 ou 3 obstáculos
            int quantidade = (rand() % 3) + 1; // 1, 2 ou 3
            criarMultiplosObstaculos(obstaculos, MAX_OBSTACULOS, screenHeight, quantidade, horizon_y);
            frameCount = 0;
        }

        // atualiza obstáculos
        atualizarObstaculos(obstaculos, MAX_OBSTACULOS, velocidadeJogo);

        // gerar itens colecionáveis a cada 60 frames (1 seg)
        frameCountItens++;
        if (frameCountItens >= 60) {
            criarItem(itens, MAX_ITENS, screenHeight, obstaculos, MAX_OBSTACULOS, horizon_y);
            frameCountItens = 0;
        }

        // atualiza itens
        atualizarItens(itens, MAX_ITENS, velocidadeJogo);

        // posição X baseada na lane com perspectiva
        // O jogador está em uma posição Y específica, então precisa interpolar igual aos obstáculos/itens
        float lane_width_top = screenWidth / 10.0f;
        float lane_offset_top = (screenWidth - lane_width_top * 3) / 2.0f;
        float lane_width_bottom = screenWidth / 2.5f;
        float lane_offset_bottom = (screenWidth - lane_width_bottom * 3) / 2.0f;
        
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
                // incrementa apenas se ainda não atingiu o limite de 5
                if (itensColetados[itens[i].tipo] < 5) {
                    itensColetados[itens[i].tipo]++;
                }
            }
        }

        // verifica vitoria (pelo menos 1 item de cada tipo)
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
            if (verificarColisao(&jogador, &obstaculos[i], lane_width_bottom, lane_offset_bottom)) {
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
            velocidadeJogo = 3.5f; // Reinicia na velocidade inicial (lenta)
            tempoUltimaAceleracao = tempoDecorrido; // Mantém tempo acumulado
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

    // Tecla ESC para voltar ao menu e resetar tudo
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
    float lane_width_bottom = screenWidth / 2.5f; // mais largo embaixo
    float lane_width_top = screenWidth / 10.0f; // mais estreito em cima
    float lane_offset_top = (screenWidth - lane_width_top * 3) / 2.0f;
    float lane_offset_bottom = (screenWidth - lane_width_bottom * 3) / 2.0f; // Centraliza as lanes na base
    
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

    // obstáculos com perspectiva
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
            
            // posição X com perspectiva
            float lane_width_top = screenWidth / 10.0f; // Perspectiva mais acentuada
            float lane_offset_top = (screenWidth - lane_width_top * 3) / 2.0f;
            float lane_width_bottom = screenWidth / 2.5f;
            float lane_offset_bottom = (screenWidth - lane_width_bottom * 3) / 2.0f;
            
            // posição X interpolada entre topo e base
            float x_top = lane_offset_top + lane_width_top * obstaculos[i].lane + lane_width_top / 2;
            float x_bottom = lane_offset_bottom + lane_width_bottom * obstaculos[i].lane + lane_width_bottom / 2;
            float obs_x = x_top + (x_bottom - x_top) * progress;
            
            // cor do obstaculo dependendo do tipo ------------ ADD SPRITES AQUI DEPOIS
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

    // itens colecionáveis com perspectiva
    for (int i = 0; i < MAX_ITENS; i++) {
        if (itens[i].ativo && !itens[i].coletado) {
            // Progress baseado na distância entre horizonte e fundo da tela
            float progress = (itens[i].pos_y - horizon_y) / (screenHeight - horizon_y);
            if (progress < 0) progress = 0;
            if (progress > 1) progress = 1;
            
            // escala com perspectiva
            float scale = 0.3f + (progress * 0.7f);
            float tamanho_scaled = 120 * scale; // 120 pixels (varia de 36px no horizonte a 120px na base)
            
            // posição X com perspectiva
            float lane_width_top = screenWidth / 10.0f; // Perspectiva mais acentuada
            float lane_offset_top = (screenWidth - lane_width_top * 3) / 2.0f;
            float lane_width_bottom = screenWidth / 2.5f;
            float lane_offset_bottom = (screenWidth - lane_width_bottom * 3) / 2.0f;
            
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

    // desenha o jogador ------------------ ADD SPRITES AQUI DEPOIS
    Color playerColor = gameOver ? GRAY : RED;
    if (jogador.abaixado) {
        // abaixado
        DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real + 20, 40, 20, playerColor);
    } else if (jogador.pulando){
        // pulando
        DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real - 20, 40, 40, playerColor);
    } else {
        DrawRectangle(jogador.pos_x_real - 20, jogador.pos_y_real, 40, 40, playerColor);
    }

    if (gameOver) {
        // game over
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 150});
        
        if (vitoria) {
            DrawText("VOCÊ VENCEU!", screenWidth/2 - 150, screenHeight/2 - 80, 50, GREEN);
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
            int icon_x = screenWidth/2 - 100 + (i * 40);
            int icon_y = screenHeight/2 + 75;
            if (texturasItens[i].id > 0) {
                Rectangle source = {0, 0, (float)texturasItens[i].width, (float)texturasItens[i].height};
                Rectangle dest = {icon_x - 24, icon_y, 48, 48}; // Aumentado de 24x24 para 48x48
                DrawTexturePro(texturasItens[i], source, dest, (Vector2){0, 0}, 0.0f, WHITE);
            } else {
                DrawCircle(icon_x, icon_y + 10, 12, coresItens[i]);
            }
            DrawText(TextFormat("%d", itensColetados[i]), screenWidth/2 - 95 + (i * 40), screenHeight/2 + 95, 15, WHITE);
        }
        
        DrawText("Pressione R para reiniciar", screenWidth/2 - 150, screenHeight/2 + 125, 20, WHITE);
        DrawText("P=Pausar | X=Resetar | ESC=Fechar jogo", screenWidth/2 - 180, screenHeight/2 + 155, 20, WHITE);
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
