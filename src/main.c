#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Estados do jogo
typedef enum {
    MENU,
    PLAYING,
    OPTIONS
} GameState;

// Estrutura para os ônibus animados
typedef struct {
    Vector2 position;
    float speed;
    float scale;
    Color color;
    int type; // 0 = detalhes azuis, 1 = detalhes verdes
} Bus;

// Função para desenhar um ônibus de trás (perspectiva 3D)
void DrawBusBack(Vector2 position, float scale, Color color, int type) {
    int x = (int)position.x;
    int y = (int)position.y;
    int s = (int)(10 * scale);
    
    // Cores de detalhes baseado no tipo
    Color detailColor = type == 0 ? (Color){100, 220, 255, 255} : (Color){100, 255, 150, 255};
    Color darkDetail = type == 0 ? (Color){50, 150, 200, 255} : (Color){50, 180, 100, 255};
    
    // Traseira do ônibus (vista de trás)
    // Parte principal rosa
    DrawRectangle(x, y, s*8, s*7, color);
    
    // Teto
    DrawRectangle(x + s*0.5f, y - s*0.8f, s*7, s*0.8f, color);
    
    // Parte inferior colorida
    DrawRectangle(x, y + s*6, s*8, s, detailColor);
    
    // Janela traseira grande
    DrawRectangle(x + s*1.5f, y + s*1, s*5, s*3, BLACK);
    DrawRectangle(x + s*1.7f, y + s*1.2f, s*4.6f, s*2.6f, (Color){150, 200, 255, 180});
    
    // Divisor da janela
    DrawRectangle(x + s*3.8f, y + s*1, s*0.4f, s*3, color);
    
    // Lanternas traseiras (vermelhas)
    DrawRectangle(x + s*0.5f, y + s*4.5f, s*1.2f, s*1.8f, (Color){255, 0, 0, 255});
    DrawRectangle(x + s*6.3f, y + s*4.5f, s*1.2f, s*1.8f, (Color){255, 0, 0, 255});
    
    // Brilho nas lanternas
    DrawRectangle(x + s*0.7f, y + s*4.7f, s*0.5f, s*0.8f, (Color){255, 100, 100, 255});
    DrawRectangle(x + s*6.5f, y + s*4.7f, s*0.5f, s*0.8f, (Color){255, 100, 100, 255});
    
    // Placa traseira
    DrawRectangle(x + s*2.5f, y + s*6.2f, s*3, s*0.6f, WHITE);
    DrawText("T19", x + s*2.7f, y + s*6.2f, s*0.5f, BLACK);
    
    // Rodas (vistas de trás)
    Color wheelColor = (Color){30, 30, 30, 255};
    DrawRectangle(x + s*0.8f, y + s*6.5f, s*1.5f, s*1.2f, wheelColor);
    DrawRectangle(x + s*5.7f, y + s*6.5f, s*1.5f, s*1.2f, wheelColor);
    
    // Centro das rodas
    DrawRectangle(x + s*1.2f, y + s*6.8f, s*0.7f, s*0.6f, detailColor);
    DrawRectangle(x + s*6.1f, y + s*6.8f, s*0.7f, s*0.6f, detailColor);
    
    // Detalhes na parte inferior
    for (int i = 0; i < 6; i++) {
        DrawRectangle(x + s*1 + i * s*1.2f, y + s*6.3f, s*0.4f, s*0.4f, darkDetail);
    }
}

// Função para desenhar um ônibus estilo pixel art (visão lateral)
void DrawBus(Vector2 position, float scale, Color color, int type) {
    int x = (int)position.x;
    int y = (int)position.y;
    int s = (int)(8 * scale); // tamanho do pixel
    
    // Cores de detalhes baseado no tipo (0=azul, 1=verde)
    Color detailColor = type == 0 ? (Color){100, 220, 255, 255} : (Color){100, 255, 150, 255};
    Color darkDetail = type == 0 ? (Color){50, 150, 200, 255} : (Color){50, 180, 100, 255};
    
    // Cor mais escura do rosa para sombras
    Color darkColor = (Color){200, 50, 120, 255};
    
    // Teto do ônibus
    DrawRectangle(x + s, y, s*8, s, color);
    
    // Corpo principal rosa
    DrawRectangle(x, y + s, s*10, s*4, color);
    
    // Listras horizontais coloridas (azul ou verde)
    DrawRectangle(x, y + s*3, s*10, s*0.5f, detailColor);
    DrawRectangle(x, y + s*3.8f, s*10, s*0.5f, detailColor);
    
    // Janelas com borda preta (mais definidas)
    DrawRectangle(x + s, y + s, s*2, s*2, BLACK);
    DrawRectangle(x + s*1.2f, y + s*1.2f, s*1.6f, s*1.6f, (Color){150, 200, 255, 255});
    
    DrawRectangle(x + s*4, y + s, s*2, s*2, BLACK);
    DrawRectangle(x + s*4.2f, y + s*1.2f, s*1.6f, s*1.6f, (Color){150, 200, 255, 255});
    
    DrawRectangle(x + s*7, y + s, s*2, s*2, BLACK);
    DrawRectangle(x + s*7.2f, y + s*1.2f, s*1.6f, s*1.6f, (Color){150, 200, 255, 255});
    
    // Janela da frente (motorista)
    DrawRectangle(x, y + s*1.5f, s*0.8f, s*1.5f, BLACK);
    DrawRectangle(x + s*0.1f, y + s*1.6f, s*0.6f, s*1.3f, (Color){150, 200, 255, 255});
    
    // Parte inferior colorida (azul ou verde)
    DrawRectangle(x, y + s*4, s*10, s*1, detailColor);
    
    // Detalhes na parte inferior
    DrawRectangle(x + s*0.5f, y + s*4.3f, s, s*0.4f, darkDetail);
    DrawRectangle(x + s*2, y + s*4.3f, s, s*0.4f, darkDetail);
    DrawRectangle(x + s*3.5f, y + s*4.3f, s, s*0.4f, darkDetail);
    DrawRectangle(x + s*5, y + s*4.3f, s, s*0.4f, darkDetail);
    DrawRectangle(x + s*6.5f, y + s*4.3f, s, s*0.4f, darkDetail);
    DrawRectangle(x + s*8, y + s*4.3f, s, s*0.4f, darkDetail);
    
    // Grade/para-choque frontal colorido
    DrawRectangle(x, y + s*2.5f, s*0.6f, s*1.2f, detailColor);
    DrawRectangle(x + s*0.1f, y + s*2.6f, s*0.3f, s*0.3f, darkDetail);
    DrawRectangle(x + s*0.1f, y + s*3.2f, s*0.3f, s*0.3f, darkDetail);
    
    // Rodas pretas
    Color wheelColor = (Color){30, 30, 30, 255};
    DrawRectangle(x + s*1.5f, y + s*5, s*2, s*2, wheelColor);
    DrawRectangle(x + s*6.5f, y + s*5, s*2, s*2, wheelColor);
    
    // Centro das rodas colorido
    DrawRectangle(x + s*2, y + s*5.5f, s, s, detailColor);
    DrawRectangle(x + s*7, y + s*5.5f, s, s, detailColor);
    
    // Faróis amarelos brilhantes
    DrawRectangle(x, y + s*2, s*0.8f, s*0.8f, (Color){255, 255, 100, 255});
    DrawRectangle(x, y + s*4, s*0.8f, s*0.6f, (Color){255, 80, 0, 255});
    
    // Detalhes laterais (retângulos coloridos)
    DrawRectangle(x + s*3, y + s*3.3f, s*0.4f, s*0.4f, WHITE);
    DrawRectangle(x + s*5, y + s*3.3f, s*0.4f, s*0.4f, WHITE);
    DrawRectangle(x + s*7, y + s*3.3f, s*0.4f, s*0.4f, WHITE);
    
    // Porta (detalhes na lateral)
    DrawRectangle(x + s*0.3f, y + s*1.2f, s*0.3f, s*2.5f, darkColor);
}

// Função para desenhar um botão estilo pixel art
bool DrawButton(const char* text, Rectangle bounds, Color color, Color hoverColor) {
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, bounds);
    bool isClicked = isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    Color currentColor = isHovered ? hoverColor : color;
    
    // Sombra pixelada
    DrawRectangle(bounds.x + 6, bounds.y + 6, bounds.width, bounds.height, (Color){0, 0, 0, 120});
    
    // Botão principal
    DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, currentColor);
    
    // Borda pixelada (linha grossa)
    DrawRectangleLines(bounds.x, bounds.y, bounds.width, bounds.height, WHITE);
    DrawRectangleLines(bounds.x + 1, bounds.y + 1, bounds.width - 2, bounds.height - 2, WHITE);
    DrawRectangleLines(bounds.x + 2, bounds.y + 2, bounds.width - 4, bounds.height - 4, WHITE);
    
    // Texto centralizado
    int textWidth = MeasureText(text, 40);
    DrawText(text, bounds.x + (bounds.width - textWidth)/2, bounds.y + (bounds.height - 40)/2, 40, WHITE);
    
    return isClicked;
}

int main(void)
{
    // Inicialização
    const int screenWidth = 800;
    const int screenHeight = 600;

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "Pula Catraca");
    SetTargetFPS(60);
    
    // Desabilitar suavização para efeito pixel art
    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);
    
    srand(time(NULL));
    
    GameState gameState = MENU;
    
    // Criando ônibus animados
    const int busCount = 6;
    Bus buses[6];
    
    Color busColors[] = {
        (Color){255, 80, 150, 255},     // Rosa (corpo)
        (Color){255, 80, 150, 255},     // Rosa (corpo)
        (Color){255, 80, 150, 255},     // Rosa (corpo)
        (Color){255, 80, 150, 255},     // Rosa (corpo)
        (Color){255, 80, 150, 255},     // Rosa (corpo)
        (Color){255, 80, 150, 255}      // Rosa (corpo)
    };
    
    for (int i = 0; i < busCount; i++) {
        buses[i].position = (Vector2){rand() % screenWidth, 100 + rand() % 400};
        buses[i].speed = 20 + rand() % 40;
        buses[i].scale = 0.6f + (rand() % 40) / 100.0f;
        buses[i].color = busColors[i];
        buses[i].type = i % 2; // Alterna entre tipo 0 (azul) e tipo 1 (verde)
    }
    
    // Variáveis para animação
    float titleBounce = 0.0f;
    
    // Loop principal do jogo
    while (!WindowShouldClose())
    {
        // Atualização
        float deltaTime = GetFrameTime();
        titleBounce += deltaTime * 2;
        
        // Atualizar posição dos ônibus
        for (int i = 0; i < busCount; i++) {
            buses[i].position.x += buses[i].speed * deltaTime;
            
            // Reiniciar posição quando sair da tela
            if (buses[i].position.x > screenWidth + 100) {
                buses[i].position.x = -100;
                buses[i].position.y = 100 + rand() % 400;
            }
        }
        
        // Desenho
        BeginDrawing();
        
        // Fundo rosa com padrão
        ClearBackground((Color){235, 90, 150, 255});
        
        // Desenhar espirais concêntricas no fundo (tipo alvo)
        for (int i = 0; i < 12; i++) {
            // Posições distribuídas pela tela
            int baseX = (i % 4) * 250 + 50;
            int baseY = (i / 4) * 220 + 50;
            float offsetAngle = titleBounce * 0.2f + i * 0.8f;
            int x = baseX + cos(offsetAngle) * 40;
            int y = baseY + sin(offsetAngle) * 40;
            
            // Cores da espiral
            Color spiralColors[] = {
                (Color){220, 75, 135, 100},
                (Color){200, 60, 120, 100},
                (Color){180, 50, 110, 100},
                (Color){160, 40, 100, 100}
            };
            
            // Desenhar círculos concêntricos (de fora para dentro)
            for (int ring = 5; ring >= 0; ring--) {
                int radius = 70 - ring * 12;
                Color ringColor = spiralColors[ring % 4];
                
                // Desenhar octógono (círculo pixelado)
                DrawPoly((Vector2){x, y}, 8, radius, titleBounce * 10 + ring * 5, ringColor);
            }
        }
        
        // Desenhar ônibus animados
        for (int i = 0; i < busCount; i++) {
            DrawBus(buses[i].position, buses[i].scale, buses[i].color, buses[i].type);
        }
        
        // Desenhar estrelinhas pixeladas decorativas
        for (int i = 0; i < 20; i++) {
            int x = (i * 123) % screenWidth;
            int y = (i * 234) % screenHeight;
            int size = 3 + (i % 2) * 2;
            float twinkle = sin(titleBounce * 3 + i) * 0.5f + 0.5f;
            Color starColor = (Color){255, 255, 150, (unsigned char)(220 * twinkle)};
            
            // Estrela pixelada (cruz + diagonal)
            DrawRectangle(x - size, y, size*3, size, starColor);
            DrawRectangle(x, y - size, size, size*3, starColor);
            DrawRectangle(x - size/2, y - size/2, size*2, size*2, starColor);
        }
        
        if (gameState == MENU) {
            // Título com animação
            float bounce = sin(titleBounce) * 10;
            const char* title = "Pula Catraca";
            int titleSize = 80;
            int titleWidth = MeasureText(title, titleSize);
            
            // Sombra do título
            DrawText(title, (screenWidth - titleWidth)/2 + 5, 60 + bounce + 5, titleSize, (Color){0, 0, 0, 150});
            // Título
            DrawText(title, (screenWidth - titleWidth)/2, 60 + bounce, titleSize, (Color){255, 240, 180, 255});
            
            // Botões
            Rectangle playButton = {300, 250, 200, 60};
            Rectangle optionsButton = {300, 330, 200, 60};
            Rectangle quitButton = {300, 410, 200, 60};
            
            if (DrawButton("PLAY", playButton, (Color){100, 200, 255, 255}, (Color){120, 220, 255, 255})) {
                gameState = PLAYING;
            }
            
            if (DrawButton("OPTIONS", optionsButton, (Color){150, 255, 100, 255}, (Color){170, 255, 120, 255})) {
                gameState = OPTIONS;
            }
            
            if (DrawButton("QUIT", quitButton, (Color){255, 100, 100, 255}, (Color){255, 120, 120, 255})) {
                break;
            }
            
            // Nome e pontuação no canto
            DrawText("Nome: NUNU", 20, screenHeight - 80, 25, WHITE);
            DrawText("Mortes: 95782", 20, screenHeight - 50, 25, WHITE);
            
        } else if (gameState == PLAYING) {
            // === TELA DE JOGO COM PERSPECTIVA 3D ===
            
            // Céu degradê (rosa para roxo claro no horizonte)
            DrawRectangleGradientV(0, 0, screenWidth, screenHeight * 0.4f, (Color){255, 150, 180, 255}, (Color){220, 120, 200, 255});
            
            // Nuvens pixeladas no céu
            for (int i = 0; i < 6; i++) {
                int cloudX = (i * 180 + (int)(titleBounce * 10)) % (screenWidth + 120) - 60;
                int cloudY = 30 + (i % 3) * 25;
                int cloudSize = 40 + (i % 2) * 20;
                // Nuvem estilo pixel art
                DrawRectangle(cloudX + 10, cloudY, cloudSize, 10, (Color){255, 255, 255, 180});
                DrawRectangle(cloudX, cloudY + 4, cloudSize + 20, 10, (Color){255, 255, 255, 180});
                DrawRectangle(cloudX + 5, cloudY + 8, cloudSize + 10, 8, (Color){255, 255, 255, 180});
            }
            
            // Skyline de prédios ao fundo (perspectiva)
            Color buildingColors[] = {
                (Color){180, 80, 150, 255},
                (Color){150, 70, 180, 255},
                (Color){200, 90, 140, 255},
                (Color){160, 85, 170, 255},
                (Color){190, 75, 160, 255}
            };
            
            // Prédios mais distantes (menor no horizonte)
            int horizonY = screenHeight * 0.35f;
            for (int i = 0; i < 12; i++) {
                int bx = i * 75;
                int bh = 60 + (i * 13) % 50;
                Color bColor = buildingColors[i % 5];
                DrawRectangle(bx, horizonY - bh, 70, bh, bColor);
                
                // Janelas iluminadas
                for (int row = 0; row < bh/15; row++) {
                    for (int col = 0; col < 2; col++) {
                        if ((i + row + col) % 3 > 0) {
                            DrawRectangle(bx + 15 + col * 25, horizonY - bh + 8 + row * 15, 10, 10, (Color){100, 220, 255, 255});
                        }
                    }
                }
            }
            
            // Chão/Estrada com perspectiva (trapézio)
            int roadStartY = horizonY;
            int roadStartWidth = 180; // Largura no horizonte
            int roadEndWidth = 650; // Largura na parte de baixo
            int roadCenterX = screenWidth / 2;
            
            // Calçada esquerda
            DrawTriangle(
                (Vector2){0, roadStartY},
                (Vector2){roadCenterX - roadStartWidth/2, roadStartY},
                (Vector2){roadCenterX - roadEndWidth/2, screenHeight},
                (Color){120, 70, 130, 255}
            );
            DrawTriangle(
                (Vector2){0, roadStartY},
                (Vector2){roadCenterX - roadEndWidth/2, screenHeight},
                (Vector2){0, screenHeight},
                (Color){120, 70, 130, 255}
            );
            
            // Asfalto roxo escuro (estrada principal)
            DrawTriangle(
                (Vector2){roadCenterX - roadStartWidth/2, roadStartY},
                (Vector2){roadCenterX + roadStartWidth/2, roadStartY},
                (Vector2){roadCenterX + roadEndWidth/2, screenHeight},
                (Color){90, 70, 110, 255}
            );
            DrawTriangle(
                (Vector2){roadCenterX - roadStartWidth/2, roadStartY},
                (Vector2){roadCenterX + roadEndWidth/2, screenHeight},
                (Vector2){roadCenterX - roadEndWidth/2, screenHeight},
                (Color){90, 70, 110, 255}
            );
            
            // Calçada direita
            DrawTriangle(
                (Vector2){roadCenterX + roadStartWidth/2, roadStartY},
                (Vector2){screenWidth, roadStartY},
                (Vector2){screenWidth, screenHeight},
                (Color){120, 70, 130, 255}
            );
            DrawTriangle(
                (Vector2){roadCenterX + roadStartWidth/2, roadStartY},
                (Vector2){screenWidth, screenHeight},
                (Vector2){roadCenterX + roadEndWidth/2, screenHeight},
                (Color){120, 70, 130, 255}
            );
            
            // Árvores/arbustos rosas nas laterais (com perspectiva)
            for (int i = 0; i < 8; i++) {
                float depth = (i + (titleBounce * 0.3f)) / 8.0f;
                depth = depth - (int)depth;
                int ty = roadStartY + depth * (screenHeight - roadStartY);
                float treeScale = 0.2f + depth * 1.0f;
                
                int currentRoadWidth = roadStartWidth + depth * (roadEndWidth - roadStartWidth);
                
                // Arbusto esquerdo
                int leftX = roadCenterX - currentRoadWidth/2 - 60 * treeScale;
                DrawRectangle(leftX, ty - 20 * treeScale, 15 * treeScale, 30 * treeScale, (Color){180, 60, 120, 255});
                DrawCircle(leftX + 7 * treeScale, ty - 10 * treeScale, 20 * treeScale, (Color){255, 100, 150, 255});
                DrawCircle(leftX + 5 * treeScale, ty - 5 * treeScale, 15 * treeScale, (Color){255, 120, 160, 255});
                
                // Arbusto direito
                int rightX = roadCenterX + currentRoadWidth/2 + 40 * treeScale;
                DrawRectangle(rightX, ty - 20 * treeScale, 15 * treeScale, 30 * treeScale, (Color){180, 60, 120, 255});
                DrawCircle(rightX + 7 * treeScale, ty - 10 * treeScale, 20 * treeScale, (Color){255, 100, 150, 255});
                DrawCircle(rightX + 5 * treeScale, ty - 5 * treeScale, 15 * treeScale, (Color){255, 120, 160, 255});
            }
            
            // Linhas da estrada com perspectiva (3 pistas)
            for (int i = 0; i < 25; i++) {
                float depth = (i + (titleBounce * 3)) / 25.0f;
                depth = depth - (int)depth; // Loop infinito
                
                int y = roadStartY + depth * (screenHeight - roadStartY);
                int currentRoadWidth = roadStartWidth + depth * (roadEndWidth - roadStartWidth);
                float lineScale = 0.5f + depth * 2.0f;
                
                // Linha amarela esquerda (divisória de pista 1-2)
                int leftLine = roadCenterX - currentRoadWidth/6;
                DrawRectangle(leftLine - 2 * lineScale, y, 4 * lineScale, 20 * lineScale, YELLOW);
                
                // Linha amarela direita (divisória de pista 2-3)
                int rightLine = roadCenterX + currentRoadWidth/6;
                DrawRectangle(rightLine - 2 * lineScale, y, 4 * lineScale, 20 * lineScale, YELLOW);
                
                // Linhas brancas laterais (borda da estrada)
                int leftEdge = roadCenterX - currentRoadWidth/2;
                int rightEdge = roadCenterX + currentRoadWidth/2;
                DrawRectangle(leftEdge, y, 3 * lineScale, 15 * lineScale, WHITE);
                DrawRectangle(rightEdge - 3 * lineScale, y, 3 * lineScale, 15 * lineScale, WHITE);
            }
            
            // Ônibus vindo de trás com perspectiva (3 pistas)
            for (int i = 0; i < 5; i++) {
                float busDepth = 0.15f + (i * 0.18f + titleBounce * 0.25f);
                busDepth = busDepth - (int)busDepth;
                
                int busY = roadStartY + busDepth * (screenHeight - roadStartY - 120);
                int currentRoadWidth = roadStartWidth + busDepth * (roadEndWidth - roadStartWidth);
                float busScale = 0.2f + busDepth * 0.9f;
                
                // Posição em uma das 3 pistas
                int lane = i % 3; // 0=esquerda, 1=meio, 2=direita
                int busX = roadCenterX - currentRoadWidth/3 + lane * currentRoadWidth/3;
                
                // Desenhar ônibus visto de trás
                DrawBusBack((Vector2){busX - 40 * busScale, busY}, busScale, (Color){255, 80, 150, 255}, lane % 2);
            }
            
            // Moedas coletáveis na pista (com perspectiva)
            for (int i = 0; i < 8; i++) {
                float coinDepth = 0.1f + (i * 0.11f + titleBounce * 0.35f);
                coinDepth = coinDepth - (int)coinDepth;
                
                int coinY = roadStartY + coinDepth * (screenHeight - roadStartY - 150);
                int currentRoadWidth = roadStartWidth + coinDepth * (roadEndWidth - roadStartWidth);
                float coinScale = 0.3f + coinDepth * 1.0f;
                int coinSize = 15 * coinScale;
                
                int lane = (i + 1) % 3;
                int coinX = roadCenterX - currentRoadWidth/3 + lane * currentRoadWidth/3;
                
                // Moeda dourada com brilho
                DrawCircle(coinX, coinY, coinSize, YELLOW);
                DrawCircle(coinX, coinY, coinSize * 0.7f, GOLD);
                DrawCircle(coinX - coinSize * 0.2f, coinY - coinSize * 0.2f, coinSize * 0.3f, (Color){255, 255, 200, 200});
                
                // Símbolo de moeda
                if (coinSize > 10) {
                    DrawText("$", coinX - coinSize * 0.35f, coinY - coinSize * 0.4f, coinSize * 1.2f, (Color){255, 200, 0, 255});
                }
            }
            
            // Personagem (na frente, centro-inferior)
            int playerX = screenWidth / 2 - 25;
            int playerY = screenHeight - 150;
            
            // Corpo do personagem pixel art
            DrawRectangle(playerX, playerY + 20, 50, 60, (Color){100, 220, 255, 255}); // Corpo azul
            DrawRectangle(playerX + 10, playerY, 30, 30, (Color){255, 200, 150, 255}); // Cabeça
            DrawRectangle(playerX + 5, playerY + 80, 18, 30, (Color){200, 150, 100, 255}); // Perna esquerda
            DrawRectangle(playerX + 27, playerY + 80, 18, 30, (Color){200, 150, 100, 255}); // Perna direita
            DrawRectangle(playerX - 5, playerY + 25, 15, 35, (Color){100, 220, 255, 255}); // Braço esquerdo
            DrawRectangle(playerX + 40, playerY + 25, 15, 35, (Color){100, 220, 255, 255}); // Braço direito
            // Olhos
            DrawRectangle(playerX + 15, playerY + 10, 6, 6, BLACK);
            DrawRectangle(playerX + 29, playerY + 10, 6, 6, BLACK);
            // Cabelo
            DrawRectangle(playerX + 10, playerY - 5, 30, 10, (Color){80, 50, 40, 255});
            
            // UI do jogo
            // Score no canto superior esquerdo
            DrawRectangle(10, 10, 220, 45, (Color){0, 0, 0, 150});
            DrawText("SCORE: 00987", 20, 18, 30, WHITE);
            
            // Moedas/vidas no canto superior direito
            DrawCircle(screenWidth - 100, 30, 20, YELLOW);
            DrawCircle(screenWidth - 100, 30, 14, GOLD);
            DrawText("$", screenWidth - 108, 16, 28, (Color){255, 200, 0, 255});
            DrawRectangle(screenWidth - 65, 15, 50, 35, (Color){0, 0, 0, 150});
            DrawText("11", screenWidth - 58, 20, 28, WHITE);
            
            // Botão pause (estilo pixel art)
            DrawRectangle(screenWidth - 55, 70, 40, 40, (Color){100, 220, 255, 255});
            DrawRectangle(screenWidth - 48, 78, 8, 24, WHITE);
            DrawRectangle(screenWidth - 32, 78, 8, 24, WHITE);
            
            // Estrelinhas decorativas
            DrawPoly((Vector2){screenWidth - 30, screenHeight - 40}, 4, 15, 45, WHITE);
            
            if (IsKeyPressed(KEY_ESCAPE)) {
                gameState = MENU;
            }
            
        } else if (gameState == OPTIONS) {
            // Tela de opções (temporário)
            DrawText("OPCOES", 280, 250, 50, WHITE);
            DrawText("Pressione ESC para voltar ao menu", 180, 320, 20, WHITE);
            
            if (IsKeyPressed(KEY_ESCAPE)) {
                gameState = MENU;
            }
        }

        EndDrawing();
    }

    // Finalização
    CloseWindow();

    return 0;
}