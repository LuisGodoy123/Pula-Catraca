# Pula-Catraca 🚇
Jogo em C estilo endless runner desenvolvido com Raylib 5.5

## 📖 Sobre o Jogo
Pula-Catraca é um jogo endless runner inspirado em Subway Surfers, onde o jogador deve desviar de obstáculos e coletar itens enquanto corre em alta velocidade. O objetivo é coletar todos os 5 tipos de itens diferentes para vencer!

## 🎮 Controles
- **W** - Pular
- **A** - Mover para esquerda
- **D** - Mover para direita
- **S** - Abaixar
- **P** - Pausar (volta ao menu mantendo progresso)
- **X** - Resetar (limpa tempo e itens e volta para o menu)
- **C** - Continuar (mantém progresso de tempo e continua a jogar)
- **ESC** - Fechar jogo (reseta tudo)

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
- 👵 **Idosa** - Perde 1 item aleatório
- 🎭 **Balaclava** - Perde todos os itens coletados

### Obstáculos
- 🚌 **Ônibus** - Aparece nas 3 lanes (esquerdo, centro, direito)
- 🚪 **Catraca** - Obstáculo no chão
- 🛞 **Cerca laranja** - Obstáculo baixo

### Sistema de Progressão
- Velocidade aumenta gradualmente durante o jogo
- Frequência de obstáculos aumenta com o tempo
- Perspectiva 3D com lanes que se estreitam ao horizonte
- Sistema de ranking que salva os melhores tempos

## 🚀 Instruções de compilação e execução

### Pré-requisitos
- **Raylib 5.5** instalado

    Como instalar:
    1. Entre no site oficial:  https://www.raylib.com/
    2. Clique na aba DOWNLOADS
    3. Baixe “raylib 5.5 for Windows (MinGW GCC)” ou “Win64 MinGW”.

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

# Compilar
make

# Executar
./pula_catraca
```

## 🎨 Características Técnicas
- **Engine**: Raylib 5.5
- **Linguagem**: C
- **Resolução**: 800x600
- **Sistema de 3 lanes** com perspectiva de movimento em 3D
- **Física realista** de pulo com gravidade
- **Sistema de animações** para sprites do jogador
- **Colisão precisa** entre jogador, obstáculos e itens
- **Sistema de áudio** com músicas e efeitos sonoros
- **Ranking persistente** em arquivos TXT
- **Game over screen** com backgrounds customizados

## 🏆 Sistema de Ranking
O jogo salva automaticamente:
- **ranking_top5.txt**: Os 5 melhores tempos
- **ranking_all.txt**: Histórico completo de todas as partidas

## 👥 Equipe
Luis Antônio Godoy 
Louise Pessoa
Marília Liz

## 📝 Licença
Este projeto é de código aberto para fins educacionais.
