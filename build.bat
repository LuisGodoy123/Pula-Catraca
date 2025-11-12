@echo off
set PATH=C:\raylib\w64devkit\bin;%PATH%

REM Criar pasta build se nao existir
if not exist build mkdir build

gcc src\main.c src\mecanica_principal.c src\ranking.c -o build\pula-catraca.exe -O1 -Wall -std=c99 -Wno-missing-braces -I include -I C:\raylib\raylib\src -L C:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm
if %errorlevel% equ 0 (
    echo.
    echo Compilacao concluida com sucesso!
    echo Execute: .\build\pula-catraca.exe
) else (
    echo.
    echo Erro na compilacao!
    pause
    exit /b 1
)
