#include "../include/mecanica_principal.h"
#include <stdlib.h>
#include <time.h>

// Inicializa o jogador com posição customizável para Raylib
void inicializarJogador(Jogador *j, float pos_x_inicial, float pos_y_inicial) {
    j->lane = 1; // começa no centro
    j->pulando = 0;
    j->abaixado = 0;
    j->velocidade_pulo = 0;
    j->tempo_abaixado = 0;
    j->pos_x_real = pos_x_inicial;
    j->pos_y_real = pos_y_inicial;
    j->chao_y = pos_y_inicial;
}

void moverEsquerda(Jogador *j) {
    if (j->lane > 0) {
        j->lane--;
    }
}

void moverDireita(Jogador *j) {
    if (j->lane < 2) {
        j->lane++;
    }
}

void pular(Jogador *j) {
    if (!j->pulando && !j->abaixado) {
        j->pulando = 1;
        j->velocidade_pulo = 15; // Reduzido de 17 para 15 (mais 10% de redução)
    }
}

void abaixar(Jogador *j) {
    if (!j->pulando && !j->abaixado) {
        j->abaixado = 1;
        j->tempo_abaixado = 46; // Reduzido de 51 para 46 frames (mais 10% de redução = ~0.77 segundos)
    }
}

void atualizarFisica(Jogador *j) {
    // Física do pulo (gravidade)
    if (j->pulando) {
        j->pos_y_real -= j->velocidade_pulo;
        j->velocidade_pulo -= 1; // gravidade
        
        // Volta ao chão
        if (j->pos_y_real >= j->chao_y) {
            j->pos_y_real = j->chao_y;
            j->pulando = 0;
            j->velocidade_pulo = 0;
        }
    }
    
    // Atualiza estado de abaixado
    if (j->abaixado) {
        j->tempo_abaixado--;
        
        if (j->tempo_abaixado <= 0) {
            j->abaixado = 0;
        }
    }
}

// ========== FUNÇÕES DE OBSTÁCULOS ==========

void inicializarObstaculos(Obstaculo obstaculos[], int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        obstaculos[i].ativo = 0;
        obstaculos[i].pos_x = 0;
        obstaculos[i].pos_y = 0;
        obstaculos[i].lane = 0;
        obstaculos[i].tipo = 0;
        obstaculos[i].largura = 0;
        obstaculos[i].altura = 0;
    }
    srand(time(NULL)); // Inicializa gerador aleatório
}

void criarObstaculo(Obstaculo obstaculos[], int tamanho, float screenHeight, float horizon_y) {
    // Procura um slot inativo
    for (int i = 0; i < tamanho; i++) {
        if (!obstaculos[i].ativo) {
            obstaculos[i].ativo = 1;
            obstaculos[i].lane = rand() % 3; // Lane aleatória (0, 1 ou 2)
            obstaculos[i].pos_y = horizon_y - 50; // Começa no horizonte
            obstaculos[i].tipo = rand() % 3; // 0 = ônibus alto, 1 = baixo, 2 = alto vazado
            
            if (obstaculos[i].tipo == 0) {
                // Ônibus alto (precisa desviar ou abaixar)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 80;
            } else if (obstaculos[i].tipo == 1) {
                // Obstáculo baixo no chão (precisa pular)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 30;
            } else {
                // Obstáculo alto vazado (precisa abaixar para passar por baixo)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 50; // Alto, mas deixa espaço embaixo
            }
            
            break;
        }
    }
}

void criarMultiplosObstaculos(Obstaculo obstaculos[], int tamanho, float screenHeight, int quantidade, float horizon_y) {
    int criados = 0;
    int lanes_usadas[3] = {0, 0, 0}; // Controla quais lanes já têm obstáculo
    int tipos_criados[3] = {0, 0, 0}; // Conta quantos de cada tipo: [laranja, verde, roxo]
    
    for (int i = 0; i < tamanho && criados < quantidade; i++) {
        if (!obstaculos[i].ativo) {
            int lane_tentativa;
            int tentativas = 0;
            
            // Tenta encontrar uma lane que ainda não tem obstáculo
            do {
                lane_tentativa = rand() % 3;
                tentativas++;
                if (tentativas >= 20) break; // Evita loop infinito
            } while (lanes_usadas[lane_tentativa] && tentativas < 20);
            
            // Se não conseguiu achar lane livre, pula este obstáculo
            if (lanes_usadas[lane_tentativa]) {
                continue;
            }
            
            obstaculos[i].ativo = 1;
            obstaculos[i].lane = lane_tentativa;
            obstaculos[i].pos_y = horizon_y - 50; // Começa no horizonte
            
            // Define o tipo, mas evita 3 laranjas
            int tipo_tentativa;
            if (quantidade == 3 && criados == 2 && tipos_criados[0] == 2) {
                // Se já tem 2 laranjas e está criando o terceiro, força outro tipo
                tipo_tentativa = (rand() % 2) + 1; // 1 (verde) ou 2 (roxo)
            } else {
                tipo_tentativa = rand() % 3; // 0, 1 ou 2
            }
            
            obstaculos[i].tipo = tipo_tentativa;
            tipos_criados[tipo_tentativa]++;
            
            if (obstaculos[i].tipo == 0) {
                // Ônibus alto (precisa desviar ou abaixar)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 80;
            } else if (obstaculos[i].tipo == 1) {
                // Obstáculo baixo no chão (precisa pular)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 30;
            } else {
                // Obstáculo alto vazado (precisa abaixar para passar por baixo)
                obstaculos[i].largura = 60;
                obstaculos[i].altura = 50;
            }
            
            lanes_usadas[lane_tentativa] = 1; // Marca a lane como usada
            criados++;
        }
    }
}

void atualizarObstaculos(Obstaculo obstaculos[], int tamanho, float velocidade) {
    for (int i = 0; i < tamanho; i++) {
        if (obstaculos[i].ativo) {
            obstaculos[i].pos_y += velocidade; // Move para baixo
            
            // Desativa se sair da tela
            if (obstaculos[i].pos_y > 700) {
                obstaculos[i].ativo = 0;
            }
        }
    }
}

int verificarColisao(Jogador *j, Obstaculo *obs, float lane_width, float lane_offset, float horizon_y, float screenHeight) {
    if (!obs->ativo) return 0;
    
    // Se é um obstáculo baixo e o jogador está pulando, não colide
    if (obs->tipo == 1 && j->pulando) {
        return 0;
    }
    
    // Se é um obstáculo alto vazado e o jogador está abaixado, não colide
    if (obs->tipo == 2 && j->abaixado) {
        return 0;
    }
    
    // Calcula o scale do obstáculo baseado na perspectiva
    float progress = (obs->pos_y - horizon_y) / (screenHeight - horizon_y);
    if (progress < 0) progress = 0;
    if (progress > 1) progress = 1;
    float scale = 0.3f + (progress * 0.7f); // De 0.3 a 1.0
    
    // Calcula posição X do obstáculo com perspectiva (igual ao renderização)
    float lane_width_top = 800.0f / 10.0f; // Usa screenWidth hardcoded (800)
    float lane_offset_top = (800.0f - lane_width_top * 3) / 2.0f;
    float lane_width_bottom = 800.0f / 2.5f;
    float lane_offset_bottom = (800.0f - lane_width_bottom * 3) / 2.0f;
    
    float x_top = lane_offset_top + lane_width_top * obs->lane + lane_width_top / 2;
    float x_bottom = lane_offset_bottom + lane_width_bottom * obs->lane + lane_width_bottom / 2;
    float obs_x = x_top + (x_bottom - x_top) * progress;
    
    // Hitbox do jogador em X (largura reduzida para 30px ao invés de 40px)
    float player_left = j->pos_x_real - 15;
    float player_right = j->pos_x_real + 15;
    
    // Hitbox do jogador em Y (expandida 10px para cima)
    float player_top = j->abaixado ? j->pos_y_real + 10 : j->pos_y_real - 10;
    float player_bottom = j->abaixado ? j->pos_y_real + 40 : j->pos_y_real + 40;
    
    // Hitbox do obstáculo tipo 0 (ônibus): ajustada proporcionalmente de 150px para 300px
    // Fator de escala: 300/150 = 2x
    // Hitbox anterior: 70% da largura visual
    // Nova hitbox: 70% * 2 = 140% = 1.4x (mantendo proporcional)
    float hitbox_factor = (obs->tipo == 0) ? 1.4f : 0.7f; // 140% para ônibus, 70% para outros
    float largura_scaled = obs->largura * scale * hitbox_factor;
    float obs_left = obs_x - largura_scaled / 2;
    float obs_right = obs_x + largura_scaled / 2;
    
    // Ajuste de altura da hitbox
    float altura_ajustada = obs->altura * scale;
    if (obs->tipo == 1) {
        // Catraca: reduz 20px da altura da hitbox
        altura_ajustada = (obs->altura * scale) - 20.0f;
        if (altura_ajustada < 0) altura_ajustada = 0; // Garante que não fique negativo
    }
    
    float obs_top = obs->pos_y + (altura_ajustada * 0.15f); // 15% de margem no topo
    float obs_bottom = obs->pos_y + (altura_ajustada * 0.85f); // 85% da altura
    
    // Verifica colisão em X e Y (AABB - Axis-Aligned Bounding Box)
    if (player_right > obs_left && player_left < obs_right &&
        player_bottom > obs_top && player_top < obs_bottom) {
        return 1; // Colidiu!
    }
    
    return 0; // Não colidiu!
}

// ============= FUNÇÕES DE ITENS COLECIONÁVEIS =============

void inicializarItens(ItemColetavel itens[], int tamanho) {
    for (int i = 0; i < tamanho; i++) {
        itens[i].ativo = 0;
        itens[i].coletado = 0;
        itens[i].pos_x = 0;
        itens[i].pos_y = 0;
        itens[i].lane = 0;
        itens[i].tipo = 0;
        itens[i].largura = 30;
        itens[i].altura = 30;
    }
}

void criarItem(ItemColetavel itens[], int tamanho, float screenHeight, Obstaculo obstaculos[], int tamanhoObstaculos, float horizon_y, int itensColetados[]) {
    // Procura um slot vazio
    for (int i = 0; i < tamanho; i++) {
        if (!itens[i].ativo) {
            // Tenta criar o item em uma lane válida (sem obstáculos laranja tipo 0)
            int tentativas = 0;
            int lane_escolhida;
            int lane_valida = 0;
            
            // Tenta até 10 vezes encontrar uma lane sem obstáculo laranja (tipo 0)
            while (tentativas < 10 && !lane_valida) {
                lane_escolhida = rand() % 3;
                lane_valida = 1; // Assume que é válida
                
                // Verifica se há obstáculo laranja (tipo 0) nesta lane
                for (int j = 0; j < tamanhoObstaculos; j++) {
                    if (obstaculos[j].ativo && 
                        obstaculos[j].tipo == 0 && 
                        obstaculos[j].lane == lane_escolhida &&
                        obstaculos[j].pos_y >= horizon_y - 100 && obstaculos[j].pos_y <= horizon_y + 100) {
                        // Há um obstáculo laranja recente nesta lane
                        lane_valida = 0;
                        break;
                    }
                }
                tentativas++;
            }
            
            // Se não encontrou lane válida após tentativas, não cria o item
            if (!lane_valida) {
                return;
            }
            
            itens[i].ativo = 1;
            itens[i].coletado = 0;
            itens[i].pos_y = horizon_y - 50; // Começa no horizonte (igual aos obstáculos)
            itens[i].lane = lane_escolhida;
            
            // Verifica quantos itens bons o jogador tem (tipos 0-4)
            int total_itens_bons = 0;
            for (int k = 0; k < 5; k++) {
                total_itens_bons += itensColetados[k];
            }
            
            int tem_pelo_menos_1 = (total_itens_bons >= 1);
            int tem_pelo_menos_2 = (total_itens_bons >= 2);
            
            // Define o tipo do item com probabilidades diferentes
            // 70% chance de item BOM (tipos 0-4)
            // 30% chance de item RUIM (tipos 5-7)
            int chance = rand() % 100;
            
            if (chance < 70) {
                // Item BOM (tipos 0-4)
                itens[i].tipo = rand() % 5;
            } else {
                // Item RUIM (tipos 5-7)
                int item_ruim = rand() % 100;
                if (item_ruim < 60) {
                    itens[i].tipo = 5; // SONO (mais comum) - 18% do total
                } else if (item_ruim < 90 && tem_pelo_menos_1) {
                    // IDOSA só aparece se jogador tiver pelo menos 1 item bom
                    itens[i].tipo = 7; // IDOSA (comum) - 9% do total
                } else if (item_ruim >= 90 && tem_pelo_menos_2) {
                    // BALACLAVA só aparece se jogador tiver pelo menos 2 itens bons
                    itens[i].tipo = 6; // BALACLAVA (mais raro!) - 3% do total
                } else {
                    // Se era para ser IDOSA/BALACLAVA mas jogador não tem itens suficientes, vira SONO
                    itens[i].tipo = 5; // SONO
                }
            }
            
            itens[i].largura = 30;
            itens[i].altura = 30;
            break;
        }
    }
}

void atualizarItens(ItemColetavel itens[], int tamanho, float velocidade) {
    for (int i = 0; i < tamanho; i++) {
        if (itens[i].ativo && !itens[i].coletado) {
            itens[i].pos_y += velocidade; // Move para baixo (igual aos obstáculos)
            
            // Desativa item se saiu da tela (passou do fundo)
            if (itens[i].pos_y > 700) { // Ajuste conforme a altura da tela
                itens[i].ativo = 0;
            }
        }
    }
}

int verificarColeta(Jogador *j, ItemColetavel *item, float lane_width, float lane_offset) {
    // Se item já foi coletado ou não está ativo, ignora
    if (item->coletado || !item->ativo) {
        return 0;
    }
    
    // Verifica se estão na mesma lane
    if (j->lane != item->lane) {
        return 0;
    }
    
    // Hitbox do jogador (posição real em Y, expandida 10px para cima)
    float player_top = j->abaixado ? j->pos_y_real + 10 : j->pos_y_real - 10;
    float player_bottom = j->abaixado ? j->pos_y_real + 40 : j->pos_y_real + 40;
    
    // Hitbox do item em Y
    float item_top = item->pos_y;
    float item_bottom = item->pos_y + item->altura;
    
    // Verifica colisão em Y (se estão na mesma lane, basta checar Y)
    if (player_bottom > item_top && player_top < item_bottom) {
        item->coletado = 1;
        return 1; // Coletou!
    }
    
    return 0; // Não coletou
}
