#!/bin/bash

# Criar pasta build se não existir
mkdir -p build

# Compilar o jogo
gcc src/main.c src/mecanica_principal.c -o build/pula-catraca -O1 -Wall -std=c99 -Wno-missing-braces -I include -I /usr/local/include -L /usr/local/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

if [ $? -eq 0 ]; then
    echo ""
    echo "Compilação concluída com sucesso!"
    echo "Execute: ./build/pula-catraca"
else
    echo ""
    echo "Erro na compilação!"
    exit 1
fi
