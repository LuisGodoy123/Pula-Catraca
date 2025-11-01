@echo off
set PATH=C:\raylib\w64devkit\bin;%PATH%
gcc src\main.c -o build\pula-catraca.exe -O1 -Wall -std=c99 -Wno-missing-braces -I C:\raylib\raylib\src -L C:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm
if %errorlevel% equ 0 (
    echo.
    echo Compilacao concluida com sucesso!
    echo Execute: .\build\pula-catraca.exe
) else (
    echo.
    echo Erro na compilacao!
)
pause
