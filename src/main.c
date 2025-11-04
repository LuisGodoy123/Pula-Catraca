#include "raylib.h"

int main(void) {
    // Carrega a imagem primeiro para descobrir o tamanho real dela
    Image bgImage = LoadImage("assets/fundo_menu.png");
    int screenWidth = bgImage.width;
    int screenHeight = bgImage.height;

    // Cria a janela com o mesmo tamanho da imagem
    InitWindow(screenWidth, screenHeight, "Pula-Catraca - Menu");
    SetTargetFPS(60);

    // Converte a imagem para textura (renderizável)
    Texture2D background = LoadTextureFromImage(bgImage);
    UnloadImage(bgImage);

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

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        bool hoverPlay = CheckCollisionPointRec(mousePos, playBtn);
        bool hoverOptions = CheckCollisionPointRec(mousePos, optionsBtn);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Fundo exatamente no tamanho da janela
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

    UnloadTexture(background);
    CloseWindow();

    return 0;
}
