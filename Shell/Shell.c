//  - Descrição:............ Código responsável por criar um shell para o sistema de arquivos EXT2
//  - Autor:................ André Felipe Baretta, João Pedro Inoe
//  - Data de criação:...... 29/05/2025
//  - Datas de atualização:. 29/05/2025, 19/06/2025, 20/06/2025, 21/06/2025, 22/06/2025.

#include "../include/EXT2_Utils.h"
#include "../include/EXT2.h"
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "Shell.h"

#define MAX_INPUT 256
#define MAX_ARGS 10
#define MAX_PATH_SIZE 256

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
int cmd_cd(FILE *file, Superblock *sb, block_group_descriptor *bgds, const char *path, char *current_path, uint32_t *current_inode) {
    char *resolved_path;
    if(resolve_path(current_path, path, &resolved_path, MAX_PATH_SIZE) != 0){
        fprintf(stderr, "Erro ao resolver o caminho");
        return -1;
    }

    uint32_t inode_num = path_to_inode(file, sb, bgds, resolved_path);
    if(inode_num == 0){
        fprintf(stderr, "Diretorio não encontrado: %s", resolved_path);
        return -1;
    }

    if(inode_to_path(file, sb, bgds, inode_num, resolved_path, MAX_PATH_SIZE)){
        fprintf(stderr, "Erro ao resolver inode");
        return -1;
    }

    *current_inode = inode_num;
    strcpy(current_path, resolved_path);

    free(resolved_path);
    return 0;
}

int cmd_ls(FILE *file, Superblock *sb, block_group_descriptor *bgds, const char *path) {
    uint32_t inode_num = path_to_inode(file, sb, bgds, path);
    
    if(inode_num == 0){
        fprintf(stderr, "Erro ao ler o inode");
        return -1;
    }

    inode inode;
    if(read_inode(file, &inode, sb, bgds, inode_num) != 0){
        fprintf(stderr, "Erro ao ler o inode %u\n", inode_num);
        return -1;
    }

    uint32_t offset = 0;

    while(offset < inode.i_size){
        ext2_dir_entry *entry = NULL;
        if(read_directory_entry(file, &entry, sb, &inode, offset) != 0){
            fprintf(stderr, "Erro ao ler diretorio\n");
            return -1;
        }

        print_directory_entry(entry);

        offset += entry->rec_len;

        free(entry);
    }
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

void shell_loop(FILE *file) {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    Superblock sb;
    if(read_superblock(file, &sb) != 0){
        fprintf(stderr, "Erro ao ler o superbloco\n");
    }

    int group_number = (sb.s_inodes_count + sb.s_inodes_per_group - 1) / sb.s_inodes_per_group;

    block_group_descriptor* bgds = malloc(sizeof(block_group_descriptor) * group_number);

    for(int i = 0; i < group_number; i++){
        read_block_group_descriptor(file, &bgds[i], &sb, i);
    }

    uint32_t current_inode = 2;
    char *current_path = malloc(sizeof(char) * MAX_PATH_SIZE);
    strcpy(current_path, "/");


    while (1) {
        printf("[%s] ", current_path);
        if (fgets(input, sizeof(input), stdin) == NULL) 
            break;

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
        } else if (strcmp(args[0], "cd") == 0) {
            char *argument_path;
            if(argc == 1)
                strcpy(argument_path, "/");
            else 
                strcpy(argument_path, args[1]);
            cmd_cd(file, &sb, bgds, argument_path, current_path, &current_inode);
        } else if (strcmp(args[0], "ls") == 0) {
            char *path_arg = malloc(sizeof(char) * MAX_PATH_SIZE);
            if(argc >= 2){
                strcpy(path_arg, args[1]);
            } else{
                path_arg[0] = '\0';
            }
            char *resolved_path;
            if(resolve_path(current_path, path_arg, &resolved_path, MAX_PATH_SIZE) != 0){
                fprintf(stderr, "Erro ao resolver o caminho\n");
                continue;
            }
            
            cmd_ls(file, &sb, bgds, resolved_path);

            free(resolved_path);
            free(path_arg);
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