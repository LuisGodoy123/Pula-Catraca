# Pula-Catraca
Programação Imperativa Funcional - Jogo em C estilo Subway Surfers

## 🎮 Controles
- **W** - Pular
- **A** - Mover para esquerda
- **D** - Mover para direita
- **S** - Abaixar (temporário)
- **ESC** - Voltar ao menu

## 🚀 Como compilar e executar

### No Linux:
```bash
# Dar permissão de execução ao script
chmod +x build.sh

# Compilar
./build.sh

# Executar
./build/pula-catraca
```

### No Windows:
```cmd
# Compilar
build.bat

# Executar
.\build\pula-catraca.exe
```

## 📁 Estrutura do Projeto
```
Pula-Catraca/
├── src/
│   ├── main.c                  # Loop principal e menu
│   └── mecanica_principal.c    # Mecânica de movimentação
├── include/
│   └── mecanica_principal.h    # Header da mecânica
├── assets/
│   ├── images/
│   └── audio/
├── build/                      # Arquivos compilados
├── build.bat                   # Script de build (Windows)
└── build.sh                    # Script de build (Linux)
```

## 🎯 Características
- Sistema de 3 lanes (esquerda, centro, direita)
- Física de pulo com gravidade
- Abaixamento temporário (não segura)
- Menu inicial com botões
- Gráficos usando Raylib
