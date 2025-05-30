# Diretórios
INCLUDE_DIR = include
SRC_DIR = src
SHELL_DIR = Shell

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -I$(INCLUDE_DIR) -I$(SHELL_DIR)

# Arquivos-fonte
SRCS = main.c $(SRC_DIR)/EXT2_Utils.c $(SHELL_DIR)/Shell.c

# Arquivos-objeto (com substituição de .c por .o)
OBJS = $(SRCS:.c=.o)

# Nome do executável
TARGET = ext2_shell

# Regra padrão
all: $(TARGET)

# Regra de linkagem
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# Regra para compilar arquivos individualmente (com caminho completo)
main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SHELL_DIR)/%.o: $(SHELL_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza
clean:
	rm -f $(OBJS) $(TARGET)
