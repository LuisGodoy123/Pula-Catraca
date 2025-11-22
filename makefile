# Compilador
CC = gcc

# Diretórios
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Arquivos fonte e objetos
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))

# Nome do executável
TARGET = pula_catraca

# Flags
CFLAGS = -Wall -std=c99 -I$(INC_DIR)

# Bibliotecas do raylib
LIBS = -lraylib -lm -lpthread -ldl -lrt -lX11

# Regra padrão
all: $(BUILD_DIR) $(TARGET)

# Linkagem final
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

# Compilação dos .c -> .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Criar diretório build/ se não existir
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Limpeza
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
