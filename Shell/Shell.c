//  - Descrição:............ Código responsável por criar um shell para o sistema de arquivos EXT2
//  - Autor:................ André Felipe Baretta, João Pedro Inoe
//  - Data de criação:...... 29/05/2025
//  - Datas de atualização:. 29/05/2025, 19/06/2025, 20/06/2025, 21/06/2025, 22/06/2025.

#include "../include/EXT2_Utils.h"
#include "../include/EXT2.h"
#include <sys/wait.h>
#include <sys/stat.h>
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
int cmd_cat(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path) {
    char *resolved_path;
    uint32_t inode_num = resolve_path(file, sb, bgds, current_inode, path, &resolved_path, MAX_PATH_SIZE);
    
    if (inode_num == 0) {
        fprintf(stderr, "cat: Arquivo não encontrado: %s\n", path);
        return -1;
    }

    inode node;
    if (read_inode(file, &node, sb, bgds, inode_num) != 0) {
        fprintf(stderr, "cat: Erro ao ler inode %u\n", inode_num);
        return -1;
    }

    // verifica se é arquivo regular
    if ((node.i_mode & S_IFMT) != S_IFREG) {
        fprintf(stderr, "cat: %s não é um arquivo regular\n", path);
        return -1;
    }

    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint32_t size_left = node.i_size;
    char *buffer = malloc(block_size);

    // Blocos diretos
    for (int i = 0; i < 12 && size_left > 0; i++) {
        uint32_t blk = node.i_block[i];
        if (read_and_print_block(file, blk, buffer, block_size, &size_left) != 0) break;
    }

    // Indireto simples
    if (size_left > 0 && node.i_block[12]) {
        uint32_t *indirect = malloc(block_size);
        read_block(file, indirect, node.i_block[12], block_size);

        for (uint32_t i = 0; i < block_size / 4 && size_left > 0; i++) {
            uint32_t blk = indirect[i];
            if (read_and_print_block(file, blk, buffer, block_size, &size_left) != 0) break;
        }
        free(indirect);
    }

    // Indireto duplo
    if (size_left > 0 && node.i_block[13]) {
        uint32_t *double_indirect = malloc(block_size);
        read_block(file, double_indirect, node.i_block[13], block_size);

        for (uint32_t i = 0; i < block_size / 4 && size_left > 0; i++) {
            if (!double_indirect[i]) continue;
            uint32_t *indirect = malloc(block_size);
            read_block(file, indirect, double_indirect[i], block_size);

            for (uint32_t j = 0; j < block_size / 4 && size_left > 0; j++) {
                uint32_t blk = indirect[j];
                if (read_and_print_block(file, blk, buffer, block_size, &size_left) != 0) break;
            }
            free(indirect);
        }
        free(double_indirect);
    }

    // Indireto triplo
    if (size_left > 0 && node.i_block[14]) {
        uint32_t *triple_indirect = malloc(block_size);
        read_block(file, triple_indirect, node.i_block[14], block_size);

        for (uint32_t i = 0; i < block_size / 4 && size_left > 0; i++) {
            if (!triple_indirect[i]) continue;
            uint32_t *double_indirect = malloc(block_size);
            read_block(file, double_indirect, triple_indirect[i], block_size);

            for (uint32_t j = 0; j < block_size / 4 && size_left > 0; j++) {
                if (!double_indirect[j]) continue;
                uint32_t *indirect = malloc(block_size);
                read_block(file, indirect, double_indirect[j], block_size);

                for (uint32_t k = 0; k < block_size / 4 && size_left > 0; k++) {
                    uint32_t blk = indirect[k];
                    if (read_and_print_block(file, blk, buffer, block_size, &size_left) != 0) break;
                }
                free(indirect);
            }
            free(double_indirect);
        }
        free(triple_indirect);
    }

    free(buffer);
    return 0;
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

    if (inode_num == 0) {
        fprintf(stderr, "cd: O diretorio '%s' nao foi encontrado.\n", path);
        return -1;
    }

    inode target_inode;
    if (read_inode(file, &target_inode, sb, bgds, inode_num) != 0) {
        fprintf(stderr, "cd: Erro critico ao ler o inode %u.\n", inode_num);
        free(resolved_path);
        return -1;
    }

    if (!S_ISDIR(target_inode.i_mode)) {
        fprintf(stderr, "cd: '%s' nao e um diretorio.\n", path);
        free(resolved_path);
        return -1;
    }

    // se todas as verificações passaram, atualize o estado do shell
    *current_inode = inode_num;
    strncpy(current_path, resolved_path, MAX_PATH_SIZE);
    current_path[MAX_PATH_SIZE - 1] = '\0';

    free(resolved_path);
    return 0;
}

int cmd_ls(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path) {
    char *resolved_path;

    uint32_t inode_num = resolve_path(file, sb, bgds, current_inode, path, &resolved_path, MAX_PATH_SIZE);

    if (inode_num == 0) {
        fprintf(stderr, "ls: O diretorio '%s' nao foi encontrado.\n", path);
        return -1;
    }

    inode dir_inode;
    if (read_inode(file, &dir_inode, sb, bgds, inode_num) != 0) {
        fprintf(stderr, "ls: Erro ao ler o inode %u\n", inode_num);
        free(resolved_path);
        return -1;
    }

    free(resolved_path);

    if (!is_inode_dir(&dir_inode)) {
        fprintf(stderr, "ls: '%s' nao e um diretorio.\n", path);
        return -1;
    }

    uint32_t offset = 0;
    while (offset < dir_inode.i_size) {
        ext2_dir_entry *entry = NULL;
        if (read_directory_entry(file, &entry, sb, &dir_inode, offset) != 0) {
            fprintf(stderr, "ls: Erro ao ler entrada de diretorio no offset %u\n", offset);
            return -1;
        }

        if (entry->inode != 0) {
            print_directory_entry(entry);
        }

        offset += entry->rec_len;
        free(entry);
    }

    return 0;
}

// Exibe o diretório corrente (caminho absoluto)
int cmd_pwd(const char *current_path) {
    printf("%s\n", current_path);
    return 0;
}

// Cria um arquivo vazio com o nome file
int cmd_touch(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path) {
    char *full_path = malloc(MAX_PATH_SIZE);
    if (!full_path) {
        perror("touch: Falha ao alocar memoria");
        return -1;
    }

    if (path[0] == '/') {
        strncpy(full_path, path, MAX_PATH_SIZE);
    } else {
        char current_path[MAX_PATH_SIZE];
        if (inode_to_path(file, sb, bgds, current_inode, current_path, MAX_PATH_SIZE) != 0) {
            if (current_inode == 2) {
                strcpy(current_path, "/");
            } else {
                fprintf(stderr, "touch: Nao foi possivel determinar o caminho atual.\n");
                free(full_path);
                return -1;
            }
        }
        if (strcmp(current_path, "/") == 0) {
            snprintf(full_path, MAX_PATH_SIZE, "/%s", path);
        } else {
            snprintf(full_path, MAX_PATH_SIZE, "%s/%s", current_path, path);
        }
    }

    uint32_t target_inode_num = path_to_inode(file, sb, bgds, full_path, 2);
    if (target_inode_num != 0) {
        fprintf(stderr, "touch: Arquivo ou diretorio ja existe: %s\n", full_path);
        free(full_path);
        return -1;
    }

    char parent_path[MAX_PATH_SIZE];
    const char *filename_ptr;
    char *last_slash = strrchr(full_path, '/');

    if (last_slash == NULL || strlen(filename_ptr = last_slash + 1) == 0 || strlen(filename_ptr) > 255) {
        fprintf(stderr, "touch: Caminho ou nome de arquivo invalido.\n");
        free(full_path);
        return -1;
    }

    char filename[256];
    strncpy(filename, filename_ptr, 255);
    filename[255] = '\0';

    if (last_slash == full_path) {
        strncpy(parent_path, "/", MAX_PATH_SIZE);
    } else {
        *last_slash = '\0';
        strncpy(parent_path, full_path, MAX_PATH_SIZE);
    }
    
    // agora podemos liberar full_path com segurança, pois já copiamos o nome do arquivo
    free(full_path);

    uint32_t parent_inode_num = path_to_inode(file, sb, bgds, parent_path, 2);
    if (parent_inode_num == 0) {
        fprintf(stderr, "touch: O diretorio pai '%s' nao existe.\n", parent_path);
        return -1;
    }

    inode parent_inode;
    read_inode(file, &parent_inode, sb, bgds, parent_inode_num);
    if (!is_inode_dir(&parent_inode)) {
        fprintf(stderr, "touch: O caminho '%s' nao e um diretorio.\n", parent_path);
        return -1;
    }

    uint32_t group = (parent_inode_num - 1) / sb->s_inodes_per_group;
    uint8_t inode_bitmap[sb->s_inodes_per_group / 8];
    read_inode_bitmap(file, inode_bitmap, sb, bgds, group);

    uint32_t new_inode_num = 0;
    for (uint32_t i = 0; i < sb->s_inodes_per_group; i++) {
        uint32_t potential_inode_num = group * sb->s_inodes_per_group + i + 1;
        if (potential_inode_num > sb->s_first_ino && !is_inode_used(inode_bitmap, i + 1, sb->s_inodes_per_group)) {
            inode_bitmap[i / 8] |= (1 << (i % 8));
            new_inode_num = potential_inode_num;
            break;
        }
    }

    if (new_inode_num == 0) {
        fprintf(stderr, "touch: Nao ha inodes livres no grupo do diretorio pai.\n");
        return -1;
    }

    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint32_t offset = 0;
    while (offset < parent_inode.i_size) {
        uint32_t entry_offset = offset;
        ext2_dir_entry *entry;
        if(read_directory_entry(file, &entry, sb, &parent_inode, offset) != 0) {
            break;
        }

        uint16_t ideal_len_entry = 8 + ((entry->name_len + 3) & ~3);
        uint16_t required_len_new = 8 + ((strlen(filename) + 3) & ~3);

        if (entry->inode != 0 && entry->rec_len >= ideal_len_entry + required_len_new) {
            uint16_t original_rec_len = entry->rec_len;
            
            entry->rec_len = ideal_len_entry;
            write_directory_entry(file, entry, sb, &parent_inode, entry_offset);
            
            uint32_t new_entry_offset = entry_offset + ideal_len_entry;
            uint16_t new_entry_rec_len = original_rec_len - ideal_len_entry;
            
            ext2_dir_entry *new_file_entry = calloc(1, new_entry_rec_len);
            if (!new_file_entry) {
                // simplificação: não fazemos um rollback
                free(entry);
                return -1;
            }
            
            new_file_entry->inode = new_inode_num;
            new_file_entry->rec_len = new_entry_rec_len;
            new_file_entry->name_len = strlen(filename);
            new_file_entry->file_type = 1; // EXT2_FT_REG_FILE
            memcpy(new_file_entry->name, filename, new_file_entry->name_len);

            write_directory_entry(file, new_file_entry, sb, &parent_inode, new_entry_offset);
            free(new_file_entry);
            free(entry);

            inode new_file_inode = {0};
            new_file_inode.i_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
            new_file_inode.i_links_count = 1;
            new_file_inode.i_size = 0; // arquivo vazio
            new_file_inode.i_blocks = 0; // não ocupa blocos de dados ainda.
            new_file_inode.i_ctime = new_file_inode.i_mtime = new_file_inode.i_atime = time(NULL);
            write_inode(file, &new_file_inode, sb, bgds, new_inode_num);

            write_inode_bitmap(file, inode_bitmap, sb, bgds, group);
            
            parent_inode.i_mtime = time(NULL);
            write_inode(file, &parent_inode, sb, bgds, parent_inode_num);
            
            bgds[group].bg_free_inodes_count--;
            write_block_group_descriptor(file, &bgds[group], sb, group);
            
            sb->s_free_inodes_count--;
            write_superblock(file, sb);
            
            printf("Arquivo '%s' criado.\n", filename);
            return 0;
        }
        
        offset += entry->rec_len;
        free(entry);
    }
    
    fprintf(stderr, "touch: Nao ha espaco nos blocos de dados do diretorio '%s' para criar novo arquivo.\n", parent_path);
    return -1;
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
        inode_to_path(file, &sb, bgds, current_inode, current_path, MAX_PATH_SIZE);
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
            cmd_cat(file, &sb, bgds, current_inode, args[1]);
        } else if (strcmp(args[0], "attr") == 0 && argc == 2) {
            cmd_attr(file, &sb, bgds, current_inode, args[1]);
        } else if (strcmp(args[0], "cd") == 0) {
            char *path_arg = (argc > 1) ? args[1] : "";
            cmd_cd(file, &sb, bgds, path_arg, current_path, &current_inode);
        
        } else if (strcmp(args[0], "ls") == 0) {
            char *path_arg = (argc > 1) ? args[1] : "";
            cmd_ls(file, &sb, bgds, current_inode, path_arg);
        } else if (strcmp(args[0], "pwd") == 0) {
            cmd_pwd(current_path);
        } else if (strcmp(args[0], "touch") == 0 && argc == 2) {
            cmd_touch(file, &sb, bgds, current_inode, args[1]);
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