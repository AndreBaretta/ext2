//  - Descrição:............ Código responsável por criar um shell para o sistema de arquivos EXT2
//  - Autor:................ André Felipe Baretta, João Pedro Inoe
//  - Data de criação:...... 29/05/2025
//  - Datas de atualização:. 29/05/2025, 19/06/2025, 20/06/2025, 21/06/2025, 22/06/2025, 23/06/2025, 24/06/2025, 27/06/2026, 28/06/2025.

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

    // Extrai e calcula informações do superbloco para exibição.
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
    // Resolve o caminho para obter o inode do arquivo.
    char *resolved_path;
    uint32_t inode_num = resolve_path(file, sb, bgds, current_inode, path, &resolved_path, MAX_PATH_SIZE);
    
    if (inode_num == 0) {
        fprintf(stderr, "cat: Arquivo não encontrado: %s\n", path);
        return -1;
    }

    // Lê o inode.    
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
    uint8_t *buffer = malloc(block_size);

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
    // Resolve o caminho.
    char *resolved_path;
    uint32_t inode_num = resolve_path(file, sb, bgds, current_inode, path, &resolved_path, MAX_PATH_SIZE);    
    if (inode_num == 0) {
        fprintf(stderr, "Arquivo ou diretório não encontrado: %s\n", path);
        return -1;
    }
    
    // Lê o inode.
    inode node;
    if (read_inode(file, &node, sb, bgds, inode_num) != 0) {
        fprintf(stderr, "Erro ao ler o inode %u\n", inode_num);
        return -1;
    }

    // Formata e exibe os atributos como permissões, tamanho, datas, etc.
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

    // Lê o inode de destino.
    inode target_inode;
    if (read_inode(file, &target_inode, sb, bgds, inode_num) != 0) {
        fprintf(stderr, "cd: Erro critico ao ler o inode %u.\n", inode_num);
        free(resolved_path);
        return -1;
    }
    // Verifica se o inode é um diretório.
    if (!S_ISDIR(target_inode.i_mode)) {
        fprintf(stderr, "cd: '%s' nao e um diretorio.\n", path);
        free(resolved_path);
        return -1;
    }

    // Se todas as verificações passaram, atualiza o estado do shell
    *current_inode = inode_num;
    strncpy(current_path, resolved_path, MAX_PATH_SIZE);
    current_path[MAX_PATH_SIZE - 1] = '\0';

    free(resolved_path);
    return 0;
}

int cmd_ls(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path) {
    // Resolve o caminho
    char *resolved_path;
    uint32_t inode_num = resolve_path(file, sb, bgds, current_inode, path, &resolved_path, MAX_PATH_SIZE);
    if (inode_num == 0) {
        fprintf(stderr, "ls: O diretorio '%s' nao foi encontrado.\n", path);
        return -1;
    }

    // Lê o inode.
    inode dir_inode;
    if (read_inode(file, &dir_inode, sb, bgds, inode_num) != 0) {
        fprintf(stderr, "ls: Erro ao ler o inode %u\n", inode_num);
        free(resolved_path);
        return -1;
    }
    free(resolved_path);

    // Verifica se o inode é um arquivo.
    if (!is_inode_dir(&dir_inode)) {
        fprintf(stderr, "ls: '%s' nao e um diretorio.\n", path);
        return -1;
    }

    // Itera sobre as entradas do diretório usando um offset.
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
    // Resolve o caminho
    char *full_path = malloc(MAX_PATH_SIZE);
    if (!full_path) {
        perror("touch: Falha ao alocar memoria");
        return -1;
    }

    // --- 1. Resolução do Caminho Absoluto ---
    // Se o caminho já for absoluto (começa com '/'), apenas copia.
    if (path[0] == '/') {
        strncpy(full_path, path, MAX_PATH_SIZE);
    } else {
        // Se o caminho for relativo, obtém o caminho do diretório atual.
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
        // Concatena o caminho atual com o caminho relativo para formar o caminho absoluto.
        int required_len;
        if (strcmp(current_path, "/") == 0) {
            required_len = snprintf(full_path, MAX_PATH_SIZE, "/%s", path);
        } else {
            required_len = snprintf(full_path, MAX_PATH_SIZE, "%s/%s", current_path, path);
        }

        // Verifica se o caminho resultante não é maior que o buffer.
        if (required_len >= MAX_PATH_SIZE) {
            fprintf(stderr, "touch: O caminho resultante e muito longo.\n");
            free(full_path); // libera a memória alocada antes de sair
            return -1;
        }
    }

    // --- 2. Validação e Extração do Nome do Arquivo ---
    // Verifica se já existe um arquivo ou diretório com o mesmo nome.
    uint32_t target_inode_num = path_to_inode(file, sb, bgds, full_path, 2);
    if (target_inode_num != 0) {
        fprintf(stderr, "touch: Arquivo ou diretorio ja existe: %s\n", full_path);
        free(full_path);
        return -1;
    }

    // Divide o caminho em caminho do diretório pai e nome do novo arquivo.
    char parent_path[MAX_PATH_SIZE];
    const char *filename_ptr;
    char *last_slash = strrchr(full_path, '/');

    // Valida o nome do arquivo.
    if (last_slash == NULL || strlen(filename_ptr = last_slash + 1) == 0 || strlen(filename_ptr) > 255) {
        fprintf(stderr, "touch: Caminho ou nome de arquivo invalido.\n");
        free(full_path);
        return -1;
    }

    // Extrai o caminho do diretório pai.
    char filename[256];
    strncpy(filename, filename_ptr, 255);
    filename[255] = '\0';

    if (last_slash == full_path) {
        strncpy(parent_path, "/", MAX_PATH_SIZE);
    } else {
        *last_slash = '\0';
        strncpy(parent_path, full_path, MAX_PATH_SIZE);
    }
    
    // Agora podemos liberar full_path com segurança, pois já copiamos o nome do arquivo.
    free(full_path);

    // --- 3. Alocação de um Inode Livre ---
    // Encontra o inode do diretório pai.
    uint32_t parent_inode_num = path_to_inode(file, sb, bgds, parent_path, 2);
    if (parent_inode_num == 0) {
        fprintf(stderr, "touch: O diretorio pai '%s' nao existe.\n", parent_path);
        return -1;
    }
    
    // Lê o inode do pai e verifica se é um diretório.
    inode parent_inode;
    read_inode(file, &parent_inode, sb, bgds, parent_inode_num);
    if (!is_inode_dir(&parent_inode)) {
        fprintf(stderr, "touch: O caminho '%s' nao e um diretorio.\n", parent_path);
        return -1;
    }

    // Determina o grupo do inode pai para procurar um inode livre no mesmo grupo.
    uint32_t group = (parent_inode_num - 1) / sb->s_inodes_per_group;
    uint8_t inode_bitmap[sb->s_inodes_per_group / 8];
    read_inode_bitmap(file, inode_bitmap, sb, bgds, group);

    // Procura um inode livre para o novo arquivo.
    uint32_t new_inode_num = 0;
    for (uint32_t i = 0; i < sb->s_inodes_per_group; i++) {
        uint32_t potential_inode_num = group * sb->s_inodes_per_group + i + 1;
        // Verifica se o inode não está em uso e não é um inode reservado.
        if (potential_inode_num > sb->s_first_ino && !is_inode_used(inode_bitmap, i + 1, sb->s_inodes_per_group)) {
            inode_bitmap[i / 8] |= (1 << (i % 8));
            new_inode_num = potential_inode_num;
            break;
        }
    }

    // Valida se existe algum espaço para inodes
    if (new_inode_num == 0) {
        fprintf(stderr, "touch: Nao ha inodes livres no grupo do diretorio pai.\n");
        return -1;
    }

    // --- 4. Adição da Nova Entrada no Diretório Pai ---
    uint32_t offset = 0;
    while (offset < parent_inode.i_size) {
        uint32_t entry_offset = offset;
        ext2_dir_entry *entry;
        if(read_directory_entry(file, &entry, sb, &parent_inode, offset) != 0) {
            break;
        }

        // Calcula o tamanho ideal que a entrada atual precisa e o tamanho necessário para a nova entrada.
        // O tamanho é sempre alinhado para múltiplos de 4.
        uint16_t ideal_len_entry = 8 + ((entry->name_len + 3) & ~3);
        uint16_t required_len_new = 8 + ((strlen(filename) + 3) & ~3);

        // Verifica se a entrada atual tem espaço sobrando (rec_len > ideal_len) para acomodar a nova entrada.
        if (entry->inode != 0 && entry->rec_len >= ideal_len_entry + required_len_new) {
            uint16_t original_rec_len = entry->rec_len;
            
            // "Encolhe" a entrada atual para seu tamanho ideal.
            entry->rec_len = ideal_len_entry;
            write_directory_entry(file, entry, sb, &parent_inode, entry_offset);
            
            // O espaço restante será usado para a nova entrada.
            uint32_t new_entry_offset = entry_offset + ideal_len_entry;
            uint16_t new_entry_rec_len = original_rec_len - ideal_len_entry;
            
            // Aloca e preenche a nova entrada de diretório.
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

            // Escreve a nova entrada no disco.
            write_directory_entry(file, new_file_entry, sb, &parent_inode, new_entry_offset);
            free(new_file_entry);
            free(entry);

            // --- 5. Criação do Inode e Atualização de Metadados ---
            // Cria a estrutura do inode para o novo arquivo.
            inode new_file_inode = {0};
            new_file_inode.i_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
            new_file_inode.i_links_count = 1;
            new_file_inode.i_size = 0; // arquivo vazio
            new_file_inode.i_blocks = 0; // não ocupa blocos de dados ainda.
            new_file_inode.i_ctime = new_file_inode.i_mtime = new_file_inode.i_atime = time(NULL);
            write_inode(file, &new_file_inode, sb, bgds, new_inode_num);

            // Grava o bitmap de inodes modificado.
            write_inode_bitmap(file, inode_bitmap, sb, bgds, group);
            
            // Atualiza o tempo de modificação do diretório pai.
            parent_inode.i_mtime = time(NULL);
            write_inode(file, &parent_inode, sb, bgds, parent_inode_num);
            
            // Decrementa contadores de inodes livres e grava as estruturas de metadados.
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
int cmd_mkdir(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path) {
    // --- 1. Resolução e Validação do Caminho ---
    // A lógica para resolver o caminho absoluto e validar a existência é idêntica à de cmd_touch.
    char full_path[MAX_PATH_SIZE];
    if (path[0] == '/') {
        strncpy(full_path, path, MAX_PATH_SIZE);
    } else {
        char current_path_str[MAX_PATH_SIZE];
        inode_to_path(file, sb, bgds, current_inode, current_path_str, MAX_PATH_SIZE);
        if (strcmp(current_path_str, "/") == 0){ 
            snprintf(full_path, MAX_PATH_SIZE, "/%s", path);
        } else { 
            int required_len = snprintf(full_path, MAX_PATH_SIZE, "%s/%s", current_path_str, path);
            if (required_len >= MAX_PATH_SIZE) {
                fprintf(stderr, "mkdir: O caminho resultante e muito longo.\n");
                return -1;
            }
        }
    }

    if (path_to_inode(file, sb, bgds, full_path, 2) != 0) {
        fprintf(stderr, "mkdir: Nao foi possivel criar o diretorio '%s': Arquivo ou diretorio ja existe\n", path);
        return -1;
    }

    // Separa o nome do novo diretório e o caminho do pai.
    char parent_path[MAX_PATH_SIZE];
    char new_dir_name[256];
    char *last_slash = strrchr(full_path, '/');
    if (last_slash == NULL || strlen(last_slash + 1) == 0) {
        fprintf(stderr, "mkdir: Caminho invalido\n");
        return -1;
    }
    strncpy(new_dir_name, last_slash + 1, 255);
    new_dir_name[255] = '\0';
    if (last_slash == full_path) 
        strcpy(parent_path, "/");
    else { 
        *last_slash = '\0'; 
        strcpy(parent_path, full_path); 
    }

    uint32_t parent_inode_num = path_to_inode(file, sb, bgds, parent_path, 2);
    inode parent_inode;
    read_inode(file, &parent_inode, sb, bgds, parent_inode_num);

    // --- 2. Alocação de Recursos (Inode e Bloco de Dados) ---
    uint32_t group = (parent_inode_num - 1) / sb->s_inodes_per_group;
    
    // Aloca um inode livre.
    uint8_t inode_bitmap[sb->s_inodes_per_group / 8];
    read_inode_bitmap(file, inode_bitmap, sb, bgds, group);
    uint32_t new_inode_num = 0;
    for (uint32_t i = 0; i < sb->s_inodes_per_group; i++) {
        if (!is_inode_used(inode_bitmap, i + 1, sb->s_inodes_per_group)) {
            inode_bitmap[i / 8] |= (1 << (i % 8));
            new_inode_num = group * sb->s_inodes_per_group + i + 1;
            break;
        }
    }
    if (new_inode_num == 0) { 
        fprintf(stderr, "mkdir: Nao ha inodes livres\n"); 
        return -1; 
    }

    // Aloca um bloco de dados livre para o novo diretório.
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint8_t block_bitmap[block_size];
    read_block_bitmap(file, block_bitmap, sb, bgds, group);
    uint32_t new_block_num = 0;
    for (uint32_t i = 0; i < sb->s_blocks_per_group; i++) {
        if (!is_block_used(block_bitmap, i + 1, sb->s_blocks_per_group)) {
            block_bitmap[i / 8] |= (1 << (i % 8));
            new_block_num = group * sb->s_blocks_per_group + i + sb->s_first_data_block;
            break;
        }
    }
    if (new_block_num == 0) { 
        fprintf(stderr, "mkdir: Nao ha blocos de dados livres\n"); 
        return -1; 
    }


    // --- 3. Criação do Inode e do Bloco de Dados do Novo Diretório ---
    // Preenche a estrutura do inode para o novo diretório.
    inode new_dir_inode = {0};
    new_dir_inode.i_mode = S_IFDIR | 0755; // wxr-xr-x
    new_dir_inode.i_links_count = 2;       // entradas . e ..
    new_dir_inode.i_size = block_size;
    new_dir_inode.i_blocks = block_size / 512;
    new_dir_inode.i_atime = new_dir_inode.i_ctime = new_dir_inode.i_mtime = time(NULL);
    new_dir_inode.i_block[0] = new_block_num;

    // Preenche o bloco de dados com as entradas "." e "..".
    char *block_buffer = calloc(1, block_size);
    // Cria a entrada para "."
    ext2_dir_entry *self_entry = (ext2_dir_entry*)block_buffer;
    self_entry->inode = new_inode_num;
    self_entry->name_len = 1;
    self_entry->file_type = 2; // EXT2_FT_DIR
    self_entry->rec_len = 12; // Tamanho ideal para esta entrada
    strcpy(self_entry->name, ".");
    
    // Cria a entrada para ".."
    ext2_dir_entry *parent_entry = (ext2_dir_entry*)(block_buffer + self_entry->rec_len);
    parent_entry->inode = parent_inode_num;
    parent_entry->name_len = 2;
    parent_entry->file_type = 2; // EXT2_FT_DIR
    parent_entry->rec_len = block_size - self_entry->rec_len; // preenche o resto do bloco
    strcpy(parent_entry->name, "..");
    
    // Escreve o bloco de dados inicializado no disco.
    write_block(file, block_buffer, new_block_num, block_size);
    free(block_buffer);

    // --- 4. Adição da Entrada no Diretório Pai e Atualização de Metadados ---
    // Adiciona a entrada para o novo diretório no diretório pai (lógica idêntica à de cmd_touch).
    uint32_t offset = 0;
    int space_found = 0;
    while (offset < parent_inode.i_size) {
        ext2_dir_entry *entry;
        read_directory_entry(file, &entry, sb, &parent_inode, offset);
        uint16_t ideal_len_entry = 8 + ((entry->name_len + 3) & ~3);
        uint16_t required_len_new = 8 + ((strlen(new_dir_name) + 3) & ~3);

        if (entry->inode != 0 && entry->rec_len >= ideal_len_entry + required_len_new) {
            uint16_t original_rec_len = entry->rec_len;
            entry->rec_len = ideal_len_entry;
            write_directory_entry(file, entry, sb, &parent_inode, offset);
            
            uint16_t new_entry_rec_len = original_rec_len - ideal_len_entry;
            ext2_dir_entry *new_dir_entry = calloc(1, new_entry_rec_len);
            new_dir_entry->inode = new_inode_num;
            new_dir_entry->rec_len = new_entry_rec_len;
            new_dir_entry->name_len = strlen(new_dir_name);
            new_dir_entry->file_type = 2; // EXT2_FT_DIR
            memcpy(new_dir_entry->name, new_dir_name, new_dir_entry->name_len);
            write_directory_entry(file, new_dir_entry, sb, &parent_inode, offset + ideal_len_entry);
            
            free(new_dir_entry);
            space_found = 1;
            break;
        }
        offset += entry->rec_len;
        free(entry);
    }
    if (!space_found) { 
        fprintf(stderr, "mkdir: Nao ha espaco no diretorio pai\n");
        return -1; 
    }

    // O link count do pai aumenta, pois a entrada ".." do novo diretório aponta para ele.
    parent_inode.i_links_count++;
    parent_inode.i_mtime = time(NULL);
    write_inode(file, &parent_inode, sb, bgds, parent_inode_num);

    // Escreve todas as novas estruturas e metadados modificados no disco.
    write_inode(file, &new_dir_inode, sb, bgds, new_inode_num);
    write_inode_bitmap(file, inode_bitmap, sb, bgds, group);
    write_block_bitmap(file, block_bitmap, sb, bgds, group);

    sb->s_free_inodes_count--;
    sb->s_free_blocks_count--;
    write_superblock(file, sb);
    
    bgds[group].bg_free_inodes_count--;
    bgds[group].bg_free_blocks_count--;
    bgds[group].bg_used_dirs_count++;
    write_block_group_descriptor(file, &bgds[group], sb, group);

    printf("Diretorio '%s' criado.\n", new_dir_name);
    return 0;
}

// Imprime os comandos
int cmd_help() {
    printf("Comandos disponiveis no EXT2 Shell:\n\n");
    printf("  info                               - Mostra informacoes do sistema de arquivos.\n");
    printf("  print inode <numero_do_inode>      - Mostra informacoes do inode escolhido.\n");
    printf("  print superblock                   - Mostra informacoes do superblock.\n");
    printf("  print groups                       - Mostra informacoes do sistema de arquivos.\n");
    printf("  ls <caminho>                       - Lista o conteudo de um diretorio.\n");
    printf("  cd <caminho>                       - Altera o diretorio atual.\n");
    printf("  pwd                                - Mostra o caminho do diretorio atual.\n");
    printf("  cat <arquivo>                      - Exibe o conteudo de um arquivo de texto.\n");
    printf("  attr <arquivo_ou_dir>              - Exibe os atributos de um arquivo ou diretorio.\n");
    printf("  touch <arquivo>                    - Cria um arquivo vazio.\n");
    printf("  mkdir <diretorio>                  - Cria um novo diretorio.\n");
    printf("  rm <arquivo>                       - Remove um arquivo.\n");
    printf("  rmdir <diretorio>                  - Remove um diretorio vazio.\n");
    printf("  rename <nome_antigo> <nome_novo>   - Renomeia um arquivo ou diretorio.\n");
    printf("  cp <origem_na_imagem> <destino>    - Copia um arquivo da imagem para o seu sistema.\n");
    printf("  help                               - Mostra esta mensagem de ajuda.\n");
    printf("  exit                               - Sai do shell.\n\n");
    printf("Use aspas (\"\") para caminhos com espacos. Ex: cat \"meu arquivo.txt\"\n");

    return 0;
}

// Remove o arquivo especificado
int cmd_rm(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path) {
    // Resolve o caminho.
    char full_path[MAX_PATH_SIZE];
    if (path[0] == '/') {
        strncpy(full_path, path, MAX_PATH_SIZE);
    } else {
        char current_path_str[MAX_PATH_SIZE];
        if (inode_to_path(file, sb, bgds, current_inode, current_path_str, MAX_PATH_SIZE) != 0) {
            if (current_inode == 2) strcpy(current_path_str, "/");
            else {
                fprintf(stderr, "rm: nao foi possivel determinar o caminho atual.\n");
                return -1;
            }
        }
        if (strcmp(current_path_str, "/") == 0) {
            snprintf(full_path, MAX_PATH_SIZE, "/%s", path);
        } else {
            int required_len = snprintf(full_path, MAX_PATH_SIZE, "%s/%s", current_path_str, path);
            if (required_len >= MAX_PATH_SIZE) {
                fprintf(stderr, "rm: O caminho resultante e muito longo.\n");
                return -1;
            }
        }
    }

    // Encontra o número do inode.
    uint32_t inode_num = path_to_inode(file, sb, bgds, full_path, 2);
    if (inode_num == 0) {
        fprintf(stderr, "rm: nao foi possivel remover '%s': Arquivo ou diretorio nao encontrado\n", path);
        return -1;
    }

    // Lê o inode.
    inode target_inode;
    read_inode(file, &target_inode, sb, bgds, inode_num);
    if (is_inode_dir(&target_inode)) {
        fprintf(stderr, "rm: nao foi possivel remover '%s': E um diretorio. Use 'rmdir'.\n", path);
        return -1;
    }

    // Extrai o nome do arquivo e o caminho do diretório pai a partir do caminho completo.
    char parent_path[MAX_PATH_SIZE];
    char filename[256];
    char *last_slash = strrchr(full_path, '/');

    if (last_slash == NULL) {
        fprintf(stderr, "rm: caminho invalido '%s'\n", full_path);
        return -1;
    }

    strncpy(filename, last_slash + 1, sizeof(filename) - 1);
    filename[sizeof(filename) - 1] = '\0';

    if (last_slash == full_path) {
        strcpy(parent_path, "/");
    } else {
        *last_slash = '\0';
        strcpy(parent_path, full_path);
    }

    // Encontra e lê o inode pai (diretório pai).
    uint32_t parent_inode_num = path_to_inode(file, sb, bgds, parent_path, 2);
    inode parent_inode;
    read_inode(file, &parent_inode, sb, bgds, parent_inode_num);

    // Remove a entrada do diretório pai.
    uint32_t offset = 0;
    ext2_dir_entry *prev_entry = NULL;
    uint32_t prev_entry_offset = 0;
    int entry_removed = 0;

    while (offset < parent_inode.i_size) {
        ext2_dir_entry *entry = NULL;
        if (read_directory_entry(file, &entry, sb, &parent_inode, offset) != 0) {
            break;
        }

        if (entry->inode != 0) {
            char name[entry->name_len + 1];
            memcpy(name, entry->name, entry->name_len);
            name[entry->name_len] = '\0';

            if (strcmp(name, filename) == 0 && entry->inode == inode_num) {
                if (prev_entry) {
                    // Adiciona o tamanho do registro atual ao anterior para apagar o atual.
                    prev_entry->rec_len += entry->rec_len;
                    write_directory_entry(file, prev_entry, sb, &parent_inode, prev_entry_offset);
                    entry_removed = 1;
                } else {
                    // primeira entrada válida no bloco, apenas zera o inode
                    entry->inode = 0;
                    write_directory_entry(file, entry, sb, &parent_inode, offset);
                    entry_removed = 1;
                }
                free(entry);
                break; 
            }
        }

        // Vai para a próxima iteração.
        if (prev_entry) free(prev_entry);
        prev_entry = entry;
        prev_entry_offset = offset;
        offset += entry->rec_len;
    }
    if (prev_entry) free(prev_entry);

    if (!entry_removed) {
        fprintf(stderr, "rm: erro critico ao tentar remover a entrada do diretorio '%s'.\n", filename);
        return -1;
    }

    // Atualiza os metadados.
    parent_inode.i_mtime = time(NULL);
    write_inode(file, &parent_inode, sb, bgds, parent_inode_num);
    target_inode.i_links_count--;
    
    if (target_inode.i_links_count == 0) {
        deallocate_inode_blocks(file, sb, bgds, &target_inode);
        deallocate_inode_metadata(file, sb, bgds, inode_num);
        target_inode.i_dtime = time(NULL);
    }

    write_inode(file, &target_inode, sb, bgds, inode_num);
    write_superblock(file, sb);

    int group_count = (sb->s_blocks_count + sb->s_blocks_per_group - 1) / sb->s_blocks_per_group;
    for (int i = 0; i < group_count; i++) {
        write_block_group_descriptor(file, &bgds[i], sb, i);
    }

    printf("Arquivo '%s' removido.\n", path);
    return 0;
}

// Remove o diretório especificado, se estiver vazio
int cmd_rmdir(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path) {
    // resolver o caminho e obter o inode do diretorio
    uint32_t target_inode_num = path_to_inode(file, sb, bgds, path, current_inode);

    // Valida o diretório alvo.
    if (target_inode_num == 0) {
        fprintf(stderr, "rmdir: falha ao remover '%s': Arquivo ou diretorio nao encontrado\n", path);
        return -1;
    }

    if (target_inode_num == 2) {
        fprintf(stderr, "rmdir: falha ao remover '/': Nao e possivel remover o diretorio raiz\n");
        return -1;
    }
    if (strcmp(path, ".") == 0 || strcmp(path, "..") == 0) {
        fprintf(stderr, "rmdir: falha ao remover '%s': Argumento invalido\n", path);
        return -1;
    }

    // Lê o inode.
    inode target_inode;
    if (read_inode(file, &target_inode, sb, bgds, target_inode_num) != 0) {
        fprintf(stderr, "rmdir: erro critico ao ler o inode %u\n", target_inode_num);
        return -1;
    }

    // Verifica se o inode é um arquivo comum.
    if (!is_inode_dir(&target_inode)) {
        fprintf(stderr, "rmdir: falha ao remover '%s': Nao e um diretorio\n", path);
        return -1;
    }

    // Verificar se o diretório está vazio (contém apenas '.' e '..')
    uint32_t offset = 0;
    while (offset < target_inode.i_size) {
        ext2_dir_entry *entry = NULL;
        if (read_directory_entry(file, &entry, sb, &target_inode, offset) != 0) break;
        
        if (entry->inode != 0) { // se for uma entrada válida
            char name[entry->name_len + 1];
            memcpy(name, entry->name, entry->name_len);
            name[entry->name_len] = '\0';
            if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
                fprintf(stderr, "rmdir: falha ao remover '%s': O diretorio nao esta vazio\n", path);
                free(entry);
                return -1;
            }
        }
        offset += entry->rec_len;
        free(entry);
    }

    // remover a entrada do diretório do seu pai
    char parent_path[MAX_PATH_SIZE];
    char dir_name[256];
    // reconstroi o caminho absoluto para encontrar o pai
    char full_path[MAX_PATH_SIZE];
    inode_to_path(file, sb, bgds, target_inode_num, full_path, MAX_PATH_SIZE);
    
    char *last_slash = strrchr(full_path, '/');
    strncpy(dir_name, last_slash + 1, sizeof(dir_name) - 1);
    if (last_slash == full_path) strcpy(parent_path, "/");
    else { *last_slash = '\0'; strcpy(parent_path, full_path); }

    // Lê o inode pai (diretório pai)
    uint32_t parent_inode_num = path_to_inode(file, sb, bgds, parent_path, 2);
    inode parent_inode;
    read_inode(file, &parent_inode, sb, bgds, parent_inode_num);

    // Remove a entrada do diretório pai.
    offset = 0;
    uint32_t prev_entry_offset = 0;
    while (offset < parent_inode.i_size) {
        ext2_dir_entry *prev_entry = NULL;
        read_directory_entry(file, &prev_entry, sb, &parent_inode, prev_entry_offset);
        
        ext2_dir_entry *entry = NULL;
        read_directory_entry(file, &entry, sb, &parent_inode, offset);

        if (entry->inode == target_inode_num) {
            prev_entry->rec_len += entry->rec_len;
            write_directory_entry(file, prev_entry, sb, &parent_inode, prev_entry_offset);
            free(entry);
            free(prev_entry);
            break;
        }

        prev_entry_offset = offset;
        offset += entry->rec_len;
        free(entry);
        free(prev_entry);
    }
    
    // atualizar metadados e contadores
    // diminui o contador de links do pai, já que a entrada '..' do diretório removido apontava para ele
    parent_inode.i_links_count--;
    parent_inode.i_mtime = time(NULL);
    write_inode(file, &parent_inode, sb, bgds, parent_inode_num);
    
    // zera o contador de links do diretório a ser removido (a entrada '.' apontava pra ele mesmo e a entrada no pai foi removida)
    target_inode.i_links_count = 0;
    target_inode.i_dtime = time(NULL);
    write_inode(file, &target_inode, sb, bgds, target_inode_num);

    // desalocar blocos
    uint32_t group = (target_inode_num - 1) / sb->s_inodes_per_group;
    deallocate_inode_blocks(file, sb, bgds, &target_inode);
    deallocate_inode_metadata(file, sb, bgds, target_inode_num);
    
    // diminui o contador de diretórios usados no grupo
    bgds[group].bg_used_dirs_count--;

    // salvar Superbloco e todos os descritores de grupo
    write_superblock(file, sb);
    int group_count = (sb->s_blocks_count + sb->s_blocks_per_group - 1) / sb->s_blocks_per_group;
    for (int i = 0; i < group_count; i++) {
        write_block_group_descriptor(file, &bgds[i], sb, i);
    }
    
    printf("Diretorio '%s' removido.\n", path);
    return 0;
}

// Renomeia um arquivo de file para newfilename
int cmd_rename(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *source_name, const char *new_name) {
    // Faz validações
    if (strlen(new_name) > 255) {
        fprintf(stderr, "rename: o novo nome '%s' e muito longo\n", new_name);
        return -1;
    }
    if (strcmp(source_name, new_name) == 0) {
        return 0; // nada a fazer
    }

    uint32_t source_inode_num = path_to_inode(file, sb, bgds, source_name, current_inode);
    if (source_inode_num == 0) {
        fprintf(stderr, "rename: nao foi possivel renomear '%s': Arquivo ou diretorio nao encontrado\n", source_name);
        return -1;
    }

    uint32_t new_inode_num = path_to_inode(file, sb, bgds, new_name, current_inode);
    if (new_inode_num != 0) {
        fprintf(stderr, "rename: nao foi possivel renomear para '%s': Arquivo ou diretorio ja existe\n", new_name);
        return -1;
    }

    // Lê o inode pai
    inode parent_inode;
    read_inode(file, &parent_inode, sb, bgds, current_inode);

    // remover a entrada de diretorio
    uint32_t offset = 0;
    uint32_t prev_entry_offset = 0;
    int old_entry_removed = 0;

    while (offset < parent_inode.i_size) {
        ext2_dir_entry *prev_entry = NULL;
        read_directory_entry(file, &prev_entry, sb, &parent_inode, prev_entry_offset);
        
        ext2_dir_entry *entry = NULL;
        if(read_directory_entry(file, &entry, sb, &parent_inode, offset) != 0) {
            if(prev_entry) free(prev_entry);
            break;
        }

        // escreve na mesma entrada que a antiga se houver espaço
        if (entry->inode == source_inode_num && entry->name_len == strlen(source_name) && strncmp(entry->name, source_name, entry->name_len) == 0) {
            prev_entry->rec_len += entry->rec_len;
            write_directory_entry(file, prev_entry, sb, &parent_inode, prev_entry_offset);
            old_entry_removed = 1;
            free(entry);
            free(prev_entry);
            break;
        }
        
        prev_entry_offset = offset;
        offset += entry->rec_len;
        free(entry);
        free(prev_entry);
    }

    if (!old_entry_removed) {
        fprintf(stderr, "rename: erro ao tentar remover a entrada antiga '%s'\n", source_name);
        return -1;
    }

    // adiciona a nova entrada no diretorio
    // recarrega o inode pai, pois seus dados foram modificados
    read_inode(file, &parent_inode, sb, bgds, current_inode);
    offset = 0;
    int new_entry_added = 0;
    uint16_t required_len_new = 8 + ((strlen(new_name) + 3) & ~3);

    while (offset < parent_inode.i_size) {
        ext2_dir_entry *entry = NULL;
        if(read_directory_entry(file, &entry, sb, &parent_inode, offset) != 0) break;
        
        uint16_t ideal_len_entry = 8 + ((entry->name_len + 3) & ~3);
        
        if (entry->inode != 0 && entry->rec_len >= ideal_len_entry + required_len_new) {
            uint16_t original_rec_len = entry->rec_len;
            entry->rec_len = ideal_len_entry;
            write_directory_entry(file, entry, sb, &parent_inode, offset);

            uint32_t new_entry_offset = offset + ideal_len_entry;
            uint16_t new_entry_rec_len = original_rec_len - ideal_len_entry;
            
            ext2_dir_entry *new_file_entry = calloc(1, new_entry_rec_len);
            new_file_entry->inode = source_inode_num;
            new_file_entry->rec_len = new_entry_rec_len;
            new_file_entry->name_len = strlen(new_name);
            
            inode source_inode; // precisamos ler o inode para saber o tipo do arquivo
            read_inode(file, &source_inode, sb, bgds, source_inode_num);
            new_file_entry->file_type = is_inode_dir(&source_inode) ? 2 : 1;
            
            memcpy(new_file_entry->name, new_name, new_file_entry->name_len);

            write_directory_entry(file, new_file_entry, sb, &parent_inode, new_entry_offset);
            
            new_entry_added = 1;
            free(new_file_entry);
            free(entry);
            break;
        }
        offset += entry->rec_len;// fechar o arquivo e liberar memória alocada
    
        free(entry);
    }
    
    if (!new_entry_added) {
        fprintf(stderr, "rename: nao ha espaco no diretorio para o novo nome '%s'\n", new_name);
        return -1;
    }

    // atualiza timestamps
    // atualiza ctime do inode renomeado
    inode source_inode;
    read_inode(file, &source_inode, sb, bgds, source_inode_num);
    source_inode.i_ctime = time(NULL);
    write_inode(file, &source_inode, sb, bgds, source_inode_num);
    
    // atualiza mtime do diretório pai, pois seu conteúdo mudou
    parent_inode.i_mtime = time(NULL);
    write_inode(file, &parent_inode, sb, bgds, current_inode);
    
    printf("'%s' renomeado para '%s'\n", source_name, new_name);
    return 0;
}

// Copia arquivo source_ext2_path (na imagem) para dest_host_path (no sistema hospedeiro)
int cmd_cp(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *source_ext2_path, const char *dest_host_path) {
    // --- 1. Validação do Caminho de Destino ---
    // O caminho de destino no sistema hospedeiro deve ser absoluto.
    if (dest_host_path[0] != '/') {
        fprintf(stderr, "cp: O caminho de destino no sistema hospedeiro deve ser um caminho absoluto (iniciar com '/').\n");
        return -1;
    }

    // --- 2. Resolução e Validação do Arquivo de Origem ---
    // Resolve o caminho na imagem EXT2 para encontrar o inode do arquivo de origem.
    char *full_source_path;
    uint32_t source_inode_num = resolve_path(file, sb, bgds, current_inode, source_ext2_path, &full_source_path, MAX_PATH_SIZE);

    if (source_inode_num == 0) {
        fprintf(stderr, "cp: nao foi possivel copiar '%s': Arquivo nao encontrado na imagem\n", source_ext2_path);
        return -1;
    }

    // Lê o inode de origem.
    inode source_inode;
    if (read_inode(file, &source_inode, sb, bgds, source_inode_num) != 0) {
        fprintf(stderr, "cp: erro ao ler o inóde de origem %u\n", source_inode_num);
        free(full_source_path);
        return -1;
    }

    // Valida se o inode de origem é um arquivo regular. A cópia de diretórios não é suportada.
    if (is_inode_dir(&source_inode)) {
        fprintf(stderr, "cp: nao e possivel copiar diretorios (nao suportado)\n");
        free(full_source_path);
        return -1;
    }
    
    // --- 3. Abertura do Arquivo de Destino no Sistema Hospedeiro ---
    // Usa fopen da biblioteca padrão para criar/abrir o arquivo no sistema local em modo de escrita binária.
    FILE *host_file = fopen(dest_host_path, "wb");
    if (host_file == NULL) {
        perror("cp: erro ao criar o arquivo de destino no sistema hospedeiro");
        return -1;
    }

    // --- 4. Leitura e Cópia dos Blocos de Dados ---
    // Prepara um buffer para a transferência de dados e inicializa o contador de bytes restantes.
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint32_t size_left = source_inode.i_size;
    uint8_t *buffer = malloc(block_size);
    if (!buffer) {
        perror("cp: falha ao alocar buffer de copia");
        fclose(host_file);
        return -1;
    }

    // blocos diretos (0-11)
    for (int i = 0; i < 12 && size_left > 0; i++) {
        copy_data_block(file, host_file, source_inode.i_block[i], buffer, block_size, &size_left);
    }

    // bloco indireto simples (12)
    if (size_left > 0 && source_inode.i_block[12]) {
        uint32_t *indirect = malloc(block_size);
        if(indirect && read_block(file, indirect, source_inode.i_block[12], block_size) == 0) {
            for (uint32_t i = 0; i < block_size / 4 && size_left > 0; i++) {
                copy_data_block(file, host_file, indirect[i], buffer, block_size, &size_left);
            }
        }
        if(indirect) free(indirect);
    }

    // bloco indireto duplo (13)
    if (size_left > 0 && source_inode.i_block[13]) {
        uint32_t *double_indirect = malloc(block_size);
        if(double_indirect && read_block(file, double_indirect, source_inode.i_block[13], block_size) == 0) {
            for (uint32_t i = 0; i < block_size / 4 && size_left > 0; i++) {
                if (!double_indirect[i]) continue;
                uint32_t *indirect = malloc(block_size);
                if(indirect && read_block(file, indirect, double_indirect[i], block_size) == 0) {
                    for (uint32_t j = 0; j < block_size / 4 && size_left > 0; j++) {
                        copy_data_block(file, host_file, indirect[j], buffer, block_size, &size_left);
                    }
                }
                if(indirect) free(indirect);
            }
        }
        if(double_indirect) free(double_indirect);
    }

    // bloco indireto triplo (14)
    if (size_left > 0 && source_inode.i_block[14]) {
        uint32_t *triple_indirect = malloc(block_size);
        if (triple_indirect && read_block(file, triple_indirect, source_inode.i_block[14], block_size) == 0) {
            for (uint32_t i = 0; i < block_size / 4 && size_left > 0; i++) {
                if (!triple_indirect[i]) continue;
                uint32_t *double_indirect = malloc(block_size);
                if (double_indirect && read_block(file, double_indirect, triple_indirect[i], block_size) == 0) {
                    for (uint32_t j = 0; j < block_size / 4 && size_left > 0; j++) {
                        if (!double_indirect[j]) continue;
                        uint32_t *indirect = malloc(block_size);
                        if (indirect && read_block(file, indirect, double_indirect[j], block_size) == 0) {
                            for (uint32_t k = 0; k < block_size / 4 && size_left > 0; k++) {
                                copy_data_block(file, host_file, indirect[k], buffer, block_size, &size_left);
                            }
                        }
                        if (indirect) free(indirect);
                    }
                }
                if (double_indirect) free(double_indirect);
            }
        }
        if (triple_indirect) free(triple_indirect);
    }

    // --- 5. Finalização ---
    // Libera a memória do buffer de cópia e fecha o arquivo do sistema hospedeiro.
    free(buffer);
    fclose(host_file);

    printf("Arquivo '%s' copiado para '%s' com sucesso.\n", source_ext2_path, dest_host_path);
    return 0;
}

// função para o comando "print superblock"
void cmd_print_superblock(Superblock *sb) {
    // Para formatar data e hora.
    char time_buffer[80];
    time_t last_check_time = sb->s_lastcheck;
    struct tm *tm_info = localtime(&last_check_time);
    strftime(time_buffer, sizeof(time_buffer), "%d/%m/%Y %H:%M", tm_info);

    printf("inodes count: %u\n", sb->s_inodes_count);
    printf("blocks count: %u\n", sb->s_blocks_count);
    printf("reserved blocks count: %u\n", sb->s_r_blocks_count);
    printf("free blocks count: %u\n", sb->s_free_blocks_count);
    printf("free inodes count: %u\n", sb->s_free_inodes_count);
    printf("first data block: %u\n", sb->s_first_data_block);
    printf("block size: %u\n", 1024 << sb->s_log_block_size);
    printf("fragment size: %u\n", 1024 << sb->s_log_frag_size);
    printf("blocks per group: %u\n", sb->s_blocks_per_group);
    printf("fragments per group: %u\n", sb->s_frags_per_group);
    printf("inodes per group: %u\n", sb->s_inodes_per_group);
    printf("mount time: %u\n", sb->s_mtime);
    printf("write time: %u\n", sb->s_wtime);
    printf("mount count: %u\n", sb->s_mnt_count);
    printf("max mount count: %u\n", sb->s_max_mnt_count);
    printf("magic signature: 0x%x\n", sb->s_magic);
    printf("file system state: %u\n", sb->s_state);
    printf("errors: %u\n", sb->s_errors);
    printf("minor revision level: %u\n", sb->s_minor_rev_level);
    printf("time of last check: %s\n", time_buffer);
    printf("max check interval: %u\n", sb->s_checkinterval);
    printf("creator OS: %u\n", sb->s_creator_os);
    printf("revision level: %u\n", sb->s_rev_level);
    printf("default uid reserved blocks: %u\n", sb->s_def_resuid);
    printf("defautl gid reserved blocks: %u\n", sb->s_def_resgid);
    printf("first non-reserved inode: %u\n", sb->s_first_ino);
    printf("inode size: %u\n", sb->s_inode_size);
    printf("block group number: %u\n", sb->s_block_group_nr);
    printf("compatible feature set: %u\n", sb->s_feature_compat);
    printf("incompatible feature set: %u\n", sb->s_feature_incompat);
    printf("read only comp feature set: %u\n", sb->s_feature_ro_compat);
    printf("volume UUID: ");
    print_uuid((const uint8_t*)sb->s_uuid);
    printf("volume name: %s\n", sb->s_volume_name);
    printf("volume last mounted: %s\n", sb->s_last_mounted);
    // Campos restantes do exemplo...
    printf("algorithm usage bitmap: %u\n", sb->s_algo_bitmap);
    printf("blocks to try to preallocate: %u\n", sb->s_prealloc_blocks);
    printf("blocks preallocate dir: %u\n", sb->s_prealloc_dir_blocks);
    printf("journal UUID: ");
    print_uuid((const uint8_t*)sb->s_journal_uuid);
    printf("journal INum: %u\n", sb->s_journal_inum);
}

// Função para o comando "print groups"
void cmd_print_groups(Superblock *sb, block_group_descriptor *bgds) {
    int group_count = (sb->s_blocks_count + sb->s_blocks_per_group - 1) / sb->s_blocks_per_group;
    for (int i = 0; i < group_count; i++) {
        printf("Block Group Descriptor %d:\n", i);
        printf("block bitmap: %u\n", bgds[i].bg_block_bitmap);
        printf("inode bitmap: %u\n", bgds[i].bg_inode_bitmap);
        printf("inode table: %u\n", bgds[i].bg_inode_table);
        printf("free blocks count: %u\n", bgds[i].bg_free_blocks_count);
        printf("free inodes count: %u\n", bgds[i].bg_free_inodes_count);
        printf("used dirs count: %u\n", bgds[i].bg_used_dirs_count);
        if (i < group_count - 1) printf("\n");
    }
}

// Função para o comando "print inode"
void cmd_print_inode(FILE *file, Superblock *sb, block_group_descriptor *bgds, const char *inode_num_str) {
    // Faz validação do inode fornecido pelo usuário.
    uint32_t inode_num = atoi(inode_num_str);
    if (inode_num == 0 || inode_num > sb->s_inodes_count) {
        fprintf(stderr, "Erro: Numero de inode invalido: %s\n", inode_num_str);
        return;
    }

    // Lê os inodes.
    inode node;
    if (read_inode(file, &node, sb, bgds, inode_num) != 0) {
        fprintf(stderr, "Erro: Nao foi possivel ler o inode %u\n", inode_num);
        return;
    }

    printf("file format and access rights: 0x%x\n", node.i_mode);
    printf("user id: %u\n", node.i_uid);
    printf("lower 32-bit file size: %u\n", node.i_size);
    printf("access time: %u\n", node.i_atime);
    printf("creation time: %u\n", node.i_ctime);
    printf("modification time: %u\n", node.i_mtime);
    printf("deletion time: %u\n", node.i_dtime);
    printf("group id: %u\n", node.i_gid);
    printf("link count inode: %u\n", node.i_links_count);
    printf("512-bytes blocks: %u\n", node.i_blocks);
    printf("ext2 flags: %u\n", node.i_flags);
    printf("reserved (Linux): %u\n", node.i_osd1);

    for (int i = 0; i < 15; i++) {
        printf("pointer[%d]: %u\n", i, node.i_block[i]);
    }

    printf("file version (nfs): %u\n", node.i_generation);
    printf("block number extended attributes: %u\n", node.i_file_acl);
    printf("higher 32-bit file size: %u\n", node.i_dir_acl);
    printf("location file fragment: %u\n", node.i_faddr);
}

// Move arquivo source_path para target_path (FUNÇÃO NÃO IMPLEMENTADA)
int  cmd_mv(const char *source_path, const char *target_path) {
    printf("Comando mv chamado de %s para %s (FUNÇÃO NÃO IMPLEMENTADA\n", source_path, target_path);
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

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        // Verifica se a linha inteira foi lida (se contém '\n').
        // Se não houver '\n', a entrada foi longa demais e o buffer de entrada precisa ser limpo.
        if (strchr(input, '\n') == NULL) {
            // A linha foi truncada. Limpa o resto do buffer de entrada.
            fprintf(stderr, "Aviso: A linha de comando era muito longa e foi truncada para %ld caracteres.\n", sizeof(input) - 1);
            int c;
            while ((c = getchar()) != '\n' && c != EOF); // Lê e descarta até o fim da linha
        } else {
            // A linha foi lida com sucesso, apenas remove o '\n' do final.
            input[strcspn(input, "\n")] = '\0';
        }

        // parser manual que lida com argumentos entre aspas.
        int argc = 0;
        char *p = input;
        while (*p != '\0' && argc < MAX_ARGS - 1) {
            // pula os espaços em branco antes de um argumento
            while (*p == ' ' || *p == '\t') {
                p++;
            }
            if (*p == '\0') {
                break; // fim da string
            }

            // início de um novo argumento foi encontrado
            args[argc] = p;
            argc++;

            // determina o fim do argumento (com ou sem aspas)
            if (*p == '"') {
                // argumento entre aspas
                args[argc - 1]++; // o argumento real começa depois da aspa
                p++; // move o ponteiro para depois da aspa de abertura
                // procura a aspa de fechamento
                while (*p != '\0' && *p != '"') {
                    p++;
                }
                // se encontrou a aspa de fechamento, termina a string ali
                if (*p == '"') {
                    *p = '\0';
                    p++; // move o ponteiro para depois da aspa de fechamento
                }
            } else {
                // argumento sem aspas
                // procura o próximo espaço ou o fim da string
                while (*p != '\0' && *p != ' ' && *p != '\t') {
                    p++;
                }
                // se não for o fim da string, termina a string no espaço
                if (*p != '\0') {
                    *p = '\0';
                    p++; // move o ponteiro para depois do espaço
                }
            }
        }

        args[argc] = NULL; // finaliza a lista de argumentos

        if (argc == 0) continue;

        // Mapeamento dos comandos
        if (strcmp(args[0], "info") == 0) {
            if(argc == 1) 
                cmd_info(&sb);
            else 
                fprintf(stderr, "Uso: info\n");
        } else if (strcmp(args[0], "cat") == 0) {
            if(argc == 2) 
                cmd_cat(file, &sb, bgds, current_inode, args[1]);
            else 
                fprintf(stderr, "Uso: cat <caminho_do_arquivo>\n");
        } else if (strcmp(args[0], "print") == 0) {
            if (argc < 2) {
                fprintf(stderr, "Uso: print <superblock|groups|inode> [numero_do_inode]\n");
            } else if (strcmp(args[1], "superblock") == 0) {
                cmd_print_superblock(&sb);
            } else if (strcmp(args[1], "groups") == 0) {
                cmd_print_groups(&sb, bgds);
            } else if (strcmp(args[1], "inode") == 0) {
                if (argc == 3) {
                    cmd_print_inode(file, &sb, bgds, args[2]);
                } else {
                    fprintf(stderr, "Uso: print inode <numero_do_inode>\n");
                }
            } else {
                fprintf(stderr, "Subcomando invalido: '%s'. Use 'superblock', 'groups', ou 'inode'.\n", args[1]);
            }
        } else if (strcmp(args[0], "help") == 0) {
            cmd_help();
        } else if (strcmp(args[0], "attr") == 0) {
            if(argc == 2) 
                cmd_attr(file, &sb, bgds, current_inode, args[1]);
            else 
                fprintf(stderr, "Uso: attr <caminho_arquivo_ou_dir>\n");
        } else if (strcmp(args[0], "cd") == 0) {
            if(argc == 1) 
                current_inode = 2;
            else if (argc == 2)     
                cmd_cd(file, &sb, bgds, args[1], current_path, &current_inode);
            else 
                fprintf(stderr, "Uso: cd <caminho_do_diretorio>\n");
        } else if (strcmp(args[0], "ls") == 0) {
            if(argc <= 2) {
                char *path_arg = (argc > 1) ? args[1] : "";
                cmd_ls(file, &sb, bgds, current_inode, path_arg);
            } else 
                fprintf(stderr, "Uso: ls <caminho_do_diretorio>\n");
        } else if (strcmp(args[0], "pwd") == 0) {
            if(argc == 1) 
                cmd_pwd(current_path);
            else 
                fprintf(stderr, "Uso: pwd\n");
        } else if (strcmp(args[0], "touch") == 0) {
            if (argc == 2) 
                cmd_touch(file, &sb, bgds, current_inode, args[1]);
            else 
                fprintf(stderr, "Uso: touch <nome_do_arquivo>\n");
        } else if (strcmp(args[0], "mkdir") == 0) {
            if(argc == 2) 
                cmd_mkdir(file, &sb, bgds, current_inode, args[1]);
            else 
                fprintf(stderr, "Uso: mkdir <nome_do_diretorio>\n");
        } else if (strcmp(args[0], "rm") == 0) {
            if(argc == 2) 
                cmd_rm(file, &sb, bgds, current_inode, args[1]);
            else 
                fprintf(stderr, "Uso: rm <nome_do_arquivo>\n");
        } else if (strcmp(args[0], "rmdir") == 0) {
            if(argc == 2) 
                cmd_rmdir(file, &sb, bgds, current_inode, args[1]);
            else 
                fprintf(stderr, "Uso: rmdir <nome_do_diretorio>\n");
        } else if (strcmp(args[0], "rename") == 0) {
            if(argc == 3) 
                cmd_rename(file, &sb, bgds, current_inode, args[1], args[2]);
            else 
                fprintf(stderr, "Uso: rename <nome_antigo> <nome_novo>\n");
        } else if (strcmp(args[0], "cp") == 0) {
            if (argc == 3) 
                cmd_cp(file, &sb, bgds, current_inode, args[1], args[2]);
            else 
                fprintf(stderr, "Uso: cp <origem_na_imagem> <destino_no_host>\n");
        } else if (strcmp(args[0], "exit") == 0) {
            break;
        } else {
            fprintf(stderr, "Comando nao reconhecido: '%s'. Digite 'help' para ver a lista de comandos.\n", args[0]);
        }
    }
}
