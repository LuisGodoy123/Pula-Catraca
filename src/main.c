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
} Bus;

// Função para desenhar um ônibus estilo pixel art
void DrawBus(Vector2 position, float scale, Color color) {
    int x = (int)position.x;
    int y = (int)position.y;
    int s = (int)(8 * scale); // tamanho do pixel
    
    // Teto
    DrawRectangle(x + s, y, s*8, s, color);
    
    // Corpo principal
    DrawRectangle(x, y + s, s*10, s*4, color);
    
    // Janelas (3 janelas azuis)
    Color windowColor = (Color){150, 200, 255, 255};
    DrawRectangle(x + s, y + s, s*2, s*2, windowColor);
    DrawRectangle(x + s*4, y + s, s*2, s*2, windowColor);
    DrawRectangle(x + s*7, y + s, s*2, s*2, windowColor);
    
    // Parte inferior escura
    Color darkColor = (Color){color.r - 40, color.g - 40, color.b - 40, 255};
    DrawRectangle(x, y + s*3, s*10, s*2, darkColor);
    
    // Rodas (quadradas para pixel art)
    Color wheelColor = (Color){60, 60, 60, 255};
    DrawRectangle(x + s, y + s*5, s*2, s*2, wheelColor);
    DrawRectangle(x + s*7, y + s*5, s*2, s*2, wheelColor);
    
    // Detalhes nas rodas
    DrawRectangle(x + s*1.5f, y + s*5.5f, s, s, GRAY);
    DrawRectangle(x + s*7.5f, y + s*5.5f, s, s, GRAY);
    
    // Farol
    DrawRectangle(x, y + s*2, s, s, YELLOW);
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
        (Color){255, 150, 100, 255},
        (Color){150, 200, 255, 255},
        (Color){255, 200, 100, 255},
        (Color){200, 150, 255, 255},
        (Color){150, 255, 150, 255},
        (Color){255, 180, 180, 255}
    };
    
    for (int i = 0; i < busCount; i++) {
        buses[i].position = (Vector2){rand() % screenWidth, 100 + rand() % 400};
        buses[i].speed = 20 + rand() % 40;
        buses[i].scale = 0.6f + (rand() % 40) / 100.0f;
        buses[i].color = busColors[i];
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
        
        // Desenhar círculos pixelados decorativos no fundo
        for (int i = 0; i < 8; i++) {
            float angle = i * 45;
            int x = 400 + cos(angle + titleBounce * 0.5f) * 300;
            int y = 300 + sin(angle + titleBounce * 0.5f) * 200;
            
            // Círculos pixelados (octógonos)
            DrawPoly((Vector2){x, y}, 8, 80, 0, (Color){220, 75, 135, 80});
            DrawPoly((Vector2){x, y}, 8, 60, 22.5f, (Color){210, 65, 125, 80});
        }
        
        // Desenhar ônibus animados
        for (int i = 0; i < busCount; i++) {
            DrawBus(buses[i].position, buses[i].scale, buses[i].color);
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
            // Tela de jogo (temporário)
            DrawText("JOGANDO...", 250, 250, 50, WHITE);
            DrawText("Pressione ESC para voltar ao menu", 180, 320, 20, WHITE);
            
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