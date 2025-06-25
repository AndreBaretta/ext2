# Definindo variáveis para os compiladores e flags
CC = gcc
CFLAGS = -Wall -Werror -Iinclude
LDFLAGS =

# Definindo os arquivos fontes
SRC = main.c src/EXT2_Utils.c Shell/Shell.c
OBJ = $(SRC:.c=.o)

# Nome do executável final
TARGET = run

# Regras do Makefile

# Regra padrão
all: $(TARGET)

# Como gerar o executável
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Como compilar os arquivos .c em arquivos .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpar arquivos gerados
clean:
	rm -f $(OBJ) $(TARGET)

# Gerar o arquivo de imagem (caso necessário)
image: $(TARGET)
	./$(TARGET) myext2image.img

.PHONY: all clean image
