//  - Descrição:............ Código responsável por criar um shell para o sistema de arquivos EXT2
//  - Autor:................ André Felipe Baretta, João Pedro Inoe
//  - Data de criação:...... 29/05/2025
//  - Datas de atualização:. 29/05/2025, 19/06/2025, 20/06/2025, 21/06/2025.

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "Shell.h"

#define MAX_INPUT 256
#define MAX_ARGS 10

// Exibe informações do disco e do sistema de arquivos
void cmd_info(void) {
    printf("Comando info chamado\n");
    // TODO: implementar leitura e exibição das informações do superbloco e outros dados do disco
}

// Exibe o conteúdo de um arquivo texto
void cmd_cat(const char *filename) {
    printf("Comando cat chamado para arquivo: %s\n", filename);
    // TODO: implementar leitura do conteúdo do arquivo e exibição no terminal
}

// Exibe os atributos de um arquivo ou diretório
void cmd_attr(const char *path) {
    printf("Comando attr chamado para: %s\n", path);
    // TODO: implementar leitura dos atributos do inode (permissões, tamanho, datas etc.)
}

// Altera o diretório corrente para o definido em path
int cmd_cd(const char *path) {
    printf("Comando cd chamado para: %s\n", path);
    // TODO: alterar diretório corrente (mudar inode atual para o inode do path)
    return 0;
}

// Lista arquivos e diretórios do diretório corrente
void cmd_ls(void) {
    printf("Comando ls chamado\n");
    // TODO: listar entradas do diretório corrente
}

// Exibe o diretório corrente (caminho absoluto)
void cmd_pwd(void) {
    printf("Comando pwd chamado\n");
    // TODO: imprimir caminho absoluto do diretório atual
}

// Cria um arquivo vazio com o nome file
int cmd_touch(const char *filename) {
    printf("Comando touch chamado para arquivo: %s\n", filename);
    // TODO: criar arquivo vazio
    return 0;
}

// Cria um diretório vazio com o nome dir
int cmd_mkdir(const char *dirname) {
    printf("Comando mkdir chamado para diretório: %s\n", dirname);
    // TODO: criar diretório vazio
    return 0;
}

// Remove o arquivo especificado
int cmd_rm(const char *filename) {
    printf("Comando rm chamado para arquivo: %s\n", filename);
    // TODO: remover arquivo
    return 0;
}

// Remove o diretório especificado, se estiver vazio
int cmd_rmdir(const char *dirname) {
    printf("Comando rmdir chamado para diretório: %s\n", dirname);
    // TODO: remover diretório (se vazio)
    return 0;
}

// Renomeia um arquivo de file para newfilename
int cmd_rename(const char *file, const char *newfilename) {
    printf("Comando rename chamado para arquivo: %s -> %s\n", file, newfilename);
    // TODO: renomear arquivo
    return 0;
}

// Copia arquivo source_path para target_path
int cmd_cp(const char *source_path, const char *target_path) {
    printf("Comando cp chamado de: %s para %s\n", source_path, target_path);
    // TODO: copiar arquivo
    return 0;
}

// Move arquivo source_path para target_path
int cmd_mv(const char *source_path, const char *target_path) {
    printf("Comando mv chamado de: %s para %s\n", source_path, target_path);
    // TODO: mover arquivo
    return 0;
}

void shell_loop() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1) {
        printf("ext2sh> ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;

        // Remover \n
        input[strcspn(input, "\n")] = '\0';

        // Separar comando e argumentos
        int argc = 0;
        char *token = strtok(input, " ");
        while (token && argc < MAX_ARGS - 1) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;

        if (argc == 0) continue;

        // Mapeamento dos comandos
        if (strcmp(args[0], "info") == 0) {
            cmd_info();
        } else if (strcmp(args[0], "cat") == 0 && argc == 2) {
            cmd_cat(args[1]);
        } else if (strcmp(args[0], "attr") == 0 && argc == 2) {
            cmd_attr(args[1]);
        } else if (strcmp(args[0], "cd") == 0 && argc == 2) {
            cmd_cd(args[1]);
        } else if (strcmp(args[0], "ls") == 0) {
            cmd_ls();
        } else if (strcmp(args[0], "pwd") == 0) {
            cmd_pwd();
        } else if (strcmp(args[0], "touch") == 0 && argc == 2) {
            cmd_touch(args[1]);
        } else if (strcmp(args[0], "mkdir") == 0 && argc == 2) {
            cmd_mkdir(args[1]);
        } else if (strcmp(args[0], "rm") == 0 && argc == 2) {
            cmd_rm(args[1]);
        } else if (strcmp(args[0], "rmdir") == 0 && argc == 2) {
            cmd_rmdir(args[1]);
        } else if (strcmp(args[0], "rename") == 0 && argc == 3) {
            cmd_rename(args[1], args[2]);
        } else if (strcmp(args[0], "cp") == 0 && argc == 3) {
            cmd_cp(args[1], args[2]);
        } else if (strcmp(args[0], "mv") == 0 && argc == 3) {
            cmd_mv(args[1], args[2]);
        } else if (strcmp(args[0], "exit") == 0) {
            break;
        } else {
            printf("Comando não reconhecido: %s\n", args[0]);
        }
    }
}