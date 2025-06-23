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
#include <time.h>
#include "Shell.h"

#define MAX_INPUT 256
#define MAX_ARGS 10
#define MAX_PATH_SIZE 256

// Exibe informações do disco e do sistema de arquivos
int cmd_info(Superblock *sb) {
    if(sb == NULL)
        return -1;

    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint32_t inode_size = sb->s_inode_size;
    uint32_t blocks_count = sb->s_blocks_count;
    uint32_t free_blocks = sb->s_free_blocks_count;
    uint32_t free_inodes = sb->s_free_inodes_count;
    uint32_t inodes_per_group = sb->s_inodes_per_group;
    uint32_t blocks_per_group = sb->s_blocks_per_group;
    uint32_t group_count = (blocks_count + blocks_per_group - 1) / blocks_per_group;
    uint32_t inode_table_blocks = (inodes_per_group * inode_size) / block_size;

    printf("Volume name.....: %.16s\n", sb->s_volume_name);
    printf("Image size......: %u bytes\n", blocks_count * block_size);
    printf("Free space......: %u KiB\n", (free_blocks * block_size) / 1024);
    printf("Free inodes.....: %u\n", free_inodes);
    printf("Free blocks.....: %u\n", free_blocks);
    printf("Block size......: %u bytes\n", block_size);
    printf("Inode size......: %u bytes\n", inode_size);
    printf("Groups count....: %u\n", group_count);
    printf("Groups size.....: %u blocks\n", blocks_per_group);
    printf("Groups inodes...: %u inodes\n", inodes_per_group);
    printf("Inodetable size.: %u blocks\n", inode_table_blocks);

    return 0;
}

// Exibe o conteúdo de um arquivo texto
void cmd_cat(const char *filename) {
    printf("Comando cat chamado para arquivo: %s\n", filename);
    // TODO: implementar leitura do conteúdo do arquivo e exibição no terminal
}

// Exibe os atributos de um arquivo ou diretório
int cmd_attr(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path) {
    char *resolved_path;

    uint32_t inode_num = resolve_path(file, sb, bgds, current_inode, path, &resolved_path, MAX_PATH_SIZE);
    
    if (inode_num == 0) {
        fprintf(stderr, "Arquivo ou diretório não encontrado: %s\n", path);
        return -1;
    }

    inode node;
    if (read_inode(file, &node, sb, bgds, inode_num) != 0) {
        fprintf(stderr, "Erro ao ler o inode %u\n", inode_num);
        return -1;
    }

    char perms[11];
    format_permissions(node.i_mode, perms);

    char size_str[12];
    format_size(node.i_size, size_str, sizeof(size_str));

    printf("Inode: %u\n", inode_num);

    printf("Permissões: %s\n", perms);
    printf("UID: %u\n", node.i_uid);
    printf("GID: %u\n", node.i_gid);
    printf("Tamanho: %s\n", size_str);
    printf("Blocos usados: %u\n", node.i_blocks);
    printf("Links: %u\n", node.i_links_count);

    // Datas
    time_t ctime_time = (time_t)node.i_ctime;
    time_t mtime_time = (time_t)node.i_mtime;
    time_t atime_time = (time_t)node.i_atime;

    printf("Criado em: %s", ctime(&ctime_time));
    printf("Modificado em: %s", ctime(&mtime_time));
    printf("Acessado em: %s", ctime(&atime_time));

    free(resolved_path);
    return 0;
}

// Altera o diretório corrente para o definido em path
int cmd_cd(FILE *file, Superblock *sb, block_group_descriptor *bgds, const char *path, char *current_path, uint32_t *current_inode) {
    char *resolved_path;

    uint32_t inode_num = resolve_path(file, sb, bgds, *current_inode, path, &resolved_path, MAX_PATH_SIZE);
    if(inode_num == 0){
        fprintf(stderr, "Diretorio nao encontrado");
    }

    *current_inode = inode_num;
    strcpy(current_path, resolved_path);

    free(resolved_path);
    return 0;
}

int cmd_ls(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path) {
    char *resolved_path;

    uint32_t inode_num = resolve_path(file, sb, bgds, current_inode, path, &resolved_path, MAX_PATH_SIZE);
    if(inode_num == 0){
        fprintf(stderr, "Diretorio nao encontrado");
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

    free(resolved_path);
}

// Exibe o diretório corrente (caminho absoluto)
int cmd_pwd(const char *current_path) {
    printf("%s\n", current_path);
    return 0;
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
            cmd_info(&sb);
        } else if (strcmp(args[0], "cat") == 0 && argc == 2) {
            cmd_cat(args[1]);
        } else if (strcmp(args[0], "attr") == 0 && argc == 2) {
            cmd_attr(file, &sb, bgds, current_inode, args[1]);
        } else if (strcmp(args[0], "cd") == 0) {
            char *argument_path = malloc(sizeof(char) * MAX_PATH_SIZE);
            if(argc == 1){
                strcpy(argument_path, "/");
            } else{
                strcpy(argument_path, args[1]);
            } 
            cmd_cd(file, &sb, bgds, argument_path, current_path, &current_inode);
            free(argument_path);
        } else if (strcmp(args[0], "ls") == 0) {
            char *path_arg = malloc(sizeof(char) * MAX_PATH_SIZE);
            if(argc >= 2){
                strcpy(path_arg, args[1]);
            } else{
                path_arg[0] = '\0';
            }
            
            cmd_ls(file, &sb, bgds, current_inode, path_arg);

            free(path_arg);
        } else if (strcmp(args[0], "pwd") == 0) {
            cmd_pwd(current_path);
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