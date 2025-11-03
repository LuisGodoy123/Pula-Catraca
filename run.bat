@echo off
echo Compilando o jogo...
call build.bat

if %errorlevel% equ 0 (
    echo.
    echo Executando o jogo...
    echo ================================
    .\build\pula-catraca.exe
) else (
    echo.
    echo Nao foi possivel executar devido a erros na compilacao.
    pause
)