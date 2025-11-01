#include "raylib.h"

int main(void)
{
    // Inicialização
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Pula Catraca");

    SetTargetFPS(60);

    // Loop principal do jogo
    while (!WindowShouldClose())
    {
        // Atualização
        

        // Desenho
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Pula Catraca!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
    }

    // Finalização
    CloseWindow();

    return 0;
}