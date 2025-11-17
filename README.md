# Pula-Catraca 🚇
Jogo em C estilo endless runner desenvolvido com Raylib 5.5

## 📖 Sobre o Jogo
Pula-Catraca é um jogo endless runner inspirado em Subway Surfers, onde o jogador deve desviar de obstáculos e coletar itens enquanto corre em alta velocidade. O objetivo é coletar todos os 5 tipos de itens diferentes para vencer!

## 🎮 Controles
- **W** - Pular
- **A** - Mover para esquerda
- **D** - Mover para direita
- **S** - Deslizar (abaixar temporário)
- **P** - Pausar (volta ao menu mantendo progresso)
- **X** - Resetar (limpa tempo e itens)
- **R** - Reiniciar (após game over, mantém progresso)
- **ESC** - Fechar jogo / Voltar ao menu (reseta tudo)

## 🎯 Objetivos e Mecânicas

### Itens Colecionáveis
**Itens Bons (colete todos para vencer!):**
- 🍿 **Pipoca** 
- 💰 **Moeda** 
- 🎫 **VEM** 
- 🔘 **Botão de Parada** 
- 🎧 **Fone** 

**Itens Ruins (evite!):**
- 😴 **Sono** - Adiciona 5 segundos ao tempo
- 🎭 **Balaclava** - Perde todos os itens coletados
- 👵 **Idosa** - Perde 1 item aleatório

### Obstáculos
- 🚌 **Ônibus** - Aparece nas 3 lanes (esquerdo, centro, direito)
- 🚪 **Catraca** - Obstáculo no chão
- 🛞 **Pneu** - Obstáculo baixo

### Sistema de Progressão
- Velocidade aumenta gradualmente durante o jogo
- Frequência de obstáculos aumenta com o tempo
- Perspectiva 3D com lanes que se estreitam ao horizonte
- Sistema de ranking que salva os melhores tempos

## 🚀 Como compilar e executar

### Pré-requisitos
- **Raylib 5.5** instalado
- **GCC** (MinGW no Windows)

### No Windows:
```cmd
# Compilar
build.bat

# Executar
.\build\pula-catraca.exe
```

### No Linux:
```bash
# Dar permissão de execução ao script
chmod +x build.sh

# Compilar
./build.sh

# Executar
./build/pula-catraca
```

## 📁 Estrutura do Projeto
```
Pula-Catraca/
├── src/
│   ├── main.c                  # Loop principal, menu e telas
│   ├── mecanica_principal.c    # Mecânica de movimentação e física
│   └── ranking.c               # Sistema de ranking e CSV
├── include/
│   ├── mecanica_principal.h    # Header da mecânica
│   └── ranking.h               # Header do sistema de ranking
├── assets/
│   ├── images/                 # Sprites e backgrounds
│   │   ├── fundo_menu.png
│   │   ├── fundo_do_jogo.png
│   │   ├── gameOver.png
│   │   ├── vitoria_scene1.png
│   │   ├── vitoria_scene2.png
│   │   ├── correndo_*.png
│   │   ├── pulando_*.png
│   │   ├── deslizando_*.png
│   │   ├── onibus*.png
│   │   ├── catraca.png
│   │   ├── pneu.png
│   │   └── [itens].png
│   └── sound/                  # Efeitos sonoros e músicas
│       ├── scene_inicial.wav
│       ├── corrida.wav
│       ├── vitoria.wav
│       ├── musica_vitoria.wav
│       ├── item_bom.wav
│       ├── item_ruim.wav
│       └── ouch.wav
├── build/                      # Arquivos compilados
├── ranking_top10.csv          # Top 10 melhores tempos
├── ranking_all.csv            # Histórico completo
├── build.bat                  # Script de build (Windows)
├── build.sh                   # Script de build (Linux)
├── run.bat                    # Script para executar (Windows)
└── README.md
```

## 🎨 Características Técnicas
- **Engine**: Raylib 5.5
- **Linguagem**: C
- **Gráficos**: OpenGL 3.3
- **Resolução**: 800x600 (HiDPI suportado)
- **Sistema de 3 lanes** com perspectiva 3D
- **Física realista** de pulo com gravidade
- **Sistema de animações** para sprites do jogador
- **Colisão precisa** entre jogador e obstáculos
- **Sistema de áudio** com músicas e efeitos sonoros
- **Ranking persistente** em arquivos CSV
- **Telas de vitória** com sequência de imagens
- **Game over screen** com backgrounds customizados

## 🏆 Sistema de Ranking
O jogo salva automaticamente:
- **ranking_top10.csv**: Os 10 melhores tempos
- **ranking_all.csv**: Histórico completo de todas as partidas

Formato: `Nickname,Tempo (segundos)`

## 🎵 Áudio
- Música ambiente no menu
- Som de corrida durante o jogo
- Efeitos sonoros para colisões e coleta de itens
- Música especial de vitória
- Todos os sons com volume ajustável

## 👥 Equipe
Projeto desenvolvido para a disciplina de Programação Imperativa Funcional

## 📝 Licença
Este projeto é de código aberto para fins educacionais.
