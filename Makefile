# Diretórios
INCLUDE_DIR = include
SRC_DIR = src
SHELL_DIR = Shell

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -I$(INCLUDE_DIR)

# Arquivos-fonte e objeto
SRCS = main.c $(SRC_DIR)/EXT2_Utils.c $(SHELL_DIR)/Shell.c
OBJS = $(SRCS:.c=.o)

# Nome do executável
TARGET = ext2_shell

# Regra padrão
all: $(TARGET)

# Linkagem
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# Compilação
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza
clean:
	rm -f $(OBJS) $(TARGET)
