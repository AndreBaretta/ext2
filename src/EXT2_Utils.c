//  - Descrição:............ Código responsável por implementar as funções básicas de leitura e escrita de estruturas do sistema de arquivos EXT2
//  - Autor:................ André Felipe Baretta, João Pedro Inoe
//  - Data de criação:...... 29/05/2025
//  - Datas de atualização:. 29/05/2025, 19/06/2025, 20/06/2025, 21/06/2025, 22/06/2025, 23/06/2025, 24/06/2025, 27/06/2026, 28/06/2025.


#include "../include/EXT2_Utils.h"
#include "../include/EXT2.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//Operações de leitura e escrita no sistema de arquivos EXT2:


// Funções de Superblock

int read_superblock(FILE *file, Superblock *sb){
    if (fseek(file, 1024, SEEK_SET) != 0) {
        perror("Erro ao buscar o superblock");
        return -1;
    }
    if (fread(sb, sizeof(Superblock), 1, file) != 1) {
        perror("Erro ao ler o superblock");
        return -1;
    }
    return 0;
}

int write_superblock(FILE *file, Superblock *sb){
    if (fseek(file, 1024, SEEK_SET) != 0) {
        perror("Erro ao buscar o superblock");
        return -1;
    }
    if (fwrite(sb, sizeof(Superblock), 1, file) != 1) {
        perror("Erro ao escrever o superblock");
        return -1;
    }
    return 0;
}

void print_uuid(const uint8_t uuid[16]) {
    for (int i = 0; i < 16; i++) {
        printf("%02x", (unsigned char)uuid[i]);
    }
    printf("\n");
}

void print_superblock(Superblock *sb){
    printf("Inodes count: %d\n", sb->s_inodes_count);
    printf("Blocks count: %d\n", sb->s_blocks_count);
    printf("Reserved blocks count: %d\n", sb->s_r_blocks_count);
    printf("Free blocks count: %d\n", sb->s_free_blocks_count);
    printf("Free inodes count: %d\n", sb->s_free_inodes_count);
    printf("First data block: %d\n", sb->s_first_data_block);
    printf("Log block size: %d\n", sb->s_log_block_size);
    printf("Log fragment size: %d\n", sb->s_log_frag_size);
    printf("Blocks per group: %d\n", sb->s_blocks_per_group);
    printf("Fragments per group: %d\n", sb->s_frags_per_group);
    printf("Inodes per group: %d\n", sb->s_inodes_per_group);
    printf("Mount time: %d\n", sb->s_mtime);
    printf("Write time: %d\n", sb->s_wtime);
    printf("Mount count: %d\n", sb->s_mnt_count);
    printf("Max mount count: %d\n", sb->s_max_mnt_count);
    printf("Magic Signature: 0x%x\n", sb->s_magic);
    printf("File System State: %d\n", sb->s_state);
    printf("Errors: %d\n", sb->s_errors);
    printf("Minor revision level: %d\n", sb->s_minor_rev_level);
    printf("Last check: %d\n", sb->s_lastcheck);
    printf("Check interval: %d\n", sb->s_checkinterval);
    printf("Creator OS: %d\n", sb->s_creator_os);
    printf("Revision level: %d\n", sb->s_rev_level);
    printf("Default reserved UID: %d\n", sb->s_def_resuid);
    printf("Default reserved GID: %d\n", sb->s_def_resgid);
    printf("First inode: %d\n", sb->s_first_ino);
    printf("Inode size: %d\n", sb->s_inode_size);
    printf("Block group number: %d\n", sb->s_block_group_nr);
    printf("Feature compat: %d\n", sb->s_feature_compat);
    printf("Feature incompat: %d\n", sb->s_feature_incompat);
    printf("Feature ro compat: %d\n", sb->s_feature_ro_compat);
    printf("Volume UUID: ");
    print_uuid( (const uint8_t*) sb->s_uuid);
    printf("Volume name: %s\n", sb->s_volume_name);
    printf("Last mounted: %s\n", sb->s_last_mounted);
    printf("Algo bitmap: %d\n", sb->s_algo_bitmap);
    printf("Prealloc blocks: %d\n", sb->s_prealloc_blocks);
    printf("Prealloc dir blocks: %d\n", sb->s_prealloc_dir_blocks);
    printf("Padding: %d\n", sb->s_padding1);
    printf("Journal UUID: ");
    print_uuid( (const uint8_t*) sb->s_journal_uuid);
    printf("Journal inum: %d\n", sb->s_journal_inum);
}

int read_block(FILE *file, void *buffer, uint32_t block_number, uint32_t block_size){
    if (fseek(file, block_number * block_size, SEEK_SET) != 0) {
        perror("Erro ao buscar o bloco");
        return -1;
    }
    if (fread(buffer, block_size, 1, file) != 1) {
        perror("Erro ao ler o bloco");
        return -1;
    }
    return 0;
}

int write_block(FILE *file, void *buffer, uint32_t block_number, uint32_t block_size){
    if (fseek(file, block_number * block_size, SEEK_SET) != 0) {
        perror("Erro ao buscar o bloco");
        return -1;
    }
    if (fwrite(buffer, block_size, 1, file) != 1) {
        perror("Erro ao escrever o bloco");
        return -1;
    }
    return 0;
}

// Funções de Block Group Descriptor

int read_block_group_descriptor(FILE *file, block_group_descriptor *bgd, Superblock *sb, uint32_t group_number){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t descriptor_table_block = (block_size == 1024) ? 2 : 1;
    uint32_t descriptor_table_position = descriptor_table_block * block_size;

    uint32_t position = descriptor_table_position + group_number * sizeof(block_group_descriptor);

    if (fseek(file, position, SEEK_SET) != 0) {
        perror("Erro ao buscar o bloco do grupo");
        return -1;
    }
    if (fread(bgd, sizeof(block_group_descriptor), 1, file) != 1) {
        perror("Erro ao ler o bloco do grupo");
        return -1;
    }

    return 0;
}

int write_block_group_descriptor(FILE *file, block_group_descriptor *bgd, Superblock *sb, uint32_t group_number){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t descriptor_table_block = (block_size == 1024) ? 2 : 1;
    uint32_t descriptor_table_position = descriptor_table_block * block_size;

    uint32_t position = descriptor_table_position + group_number * sizeof(block_group_descriptor);

    if (fseek(file, position, SEEK_SET) != 0) {
        perror("Erro ao buscar o bloco descritor do grupo");
        return -1;
    }
    if (fwrite(bgd, sizeof(block_group_descriptor), 1, file) != 1) {
        perror("Erro ao escrever o bloco descritor do grupo");
        return -1;
    }

    return 0;
}

void print_block_group_descriptor(block_group_descriptor *bgd, uint32_t number){
    printf("Block group descriptor %d:\n", number);
    printf("Block bitmap: %d\n", bgd->bg_block_bitmap);
    printf("Inode bitmap: %d\n", bgd->bg_inode_bitmap);
    printf("Inode table: %d\n", bgd->bg_inode_table);
    printf("Free blocks count: %d\n", bgd->bg_free_blocks_count);   
    printf("Free inodes count: %d\n", bgd->bg_free_inodes_count);
    printf("Used dirs count: %d\n", bgd->bg_used_dirs_count);
    printf("\n");
}

// Funções de Inode

int read_inode(FILE *file, inode *inode, Superblock *sb, block_group_descriptor *bgds, uint32_t inode_number){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t group = (inode_number - 1) / sb->s_inodes_per_group;
    uint32_t index = (inode_number - 1) % sb->s_inodes_per_group;

    uint32_t inode_table_position = bgds[group].bg_inode_table * block_size;

    uint32_t inode_position = inode_table_position + index * sb->s_inode_size;  

    if (fseek(file, inode_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o inode");
        return -1;
    }
    if (fread(inode, sb->s_inode_size, 1, file) != 1) {
        perror("Erro ao ler o inode");
        return -1;
    }

    return 0;
}

int write_inode(FILE *file, inode *inode, Superblock *sb, block_group_descriptor *bgd, uint32_t inode_number){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t group = (inode_number - 1) / sb->s_inodes_per_group;
    uint32_t index = (inode_number - 1) % sb->s_inodes_per_group;

    uint32_t inode_table_position = bgd[group].bg_inode_table * block_size;

    uint32_t inode_position = inode_table_position + index * sb->s_inode_size;

    if (fseek(file, inode_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o inode");
        return -1;
    }
    if (fwrite(inode, sb->s_inode_size, 1, file) != 1) {
        perror("Erro ao escrever o inode");
        return -1;
    }
    return 0;
}

void print_inode(inode *inode, uint32_t number){
    printf("Inode %d: \n", number);
    printf("Size: %d\n", inode->i_size);
    printf("Blocks: %d\n", inode->i_blocks);
    printf("Links: %d\n", inode->i_links_count);
    printf("Mode: %d\n", inode->i_mode);
    printf("User ID: %d\n", inode->i_uid);
    printf("Group ID: %d\n", inode->i_gid);
    printf("Links count: %d\n", inode->i_links_count);
    printf("Blocks count: %d\n", inode->i_blocks);
    printf("Generation number: %d\n", inode->i_generation);
    for(int i = 0; i < 15; i++){
        printf("Block %d: %d\n", i, inode->i_block[i]);
    }
    printf("File ACL: %d\n", inode->i_file_acl);
    printf("Directory ACL: %d\n", inode->i_dir_acl);
}

// Funções de diretório
int read_directory_entry(FILE *file, ext2_dir_entry **entry, Superblock *sb, inode *inode, uint32_t offset) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    if (offset >= inode->i_size) {
        return -1; // offset fora dos limites do diretório
    }

    uint32_t logical_block = offset / block_size;
    uint32_t offset_in_block = offset % block_size;
    uint32_t physical_block_num = 0;
    uint32_t pointers_per_block = block_size / sizeof(uint32_t);

    // encontrar o número do bloco físico correspondente ao bloco lógico
    if (logical_block < 12) {
        // blocos diretos
        physical_block_num = inode->i_block[logical_block];
    } else if (logical_block < 12 + pointers_per_block) {
        // bloco indireto simples
        uint32_t *indirect_block = malloc(block_size);
        if (read_block(file, indirect_block, inode->i_block[12], block_size) == 0) {
            physical_block_num = indirect_block[logical_block - 12];
        }
        free(indirect_block);
    } else if (logical_block < 12 + pointers_per_block + (pointers_per_block * pointers_per_block)) {
        // bloco indireto duplo
        uint32_t *double_indirect_block = malloc(block_size);
        if (read_block(file, double_indirect_block, inode->i_block[13], block_size) == 0) {
            uint32_t indirect_block_idx = (logical_block - 12 - pointers_per_block) / pointers_per_block;
            uint32_t *indirect_block = malloc(block_size);
            if (read_block(file, indirect_block, double_indirect_block[indirect_block_idx], block_size) == 0) {
                uint32_t data_block_idx = (logical_block - 12 - pointers_per_block) % pointers_per_block;
                physical_block_num = indirect_block[data_block_idx];
            }
            free(indirect_block);
        }
        free(double_indirect_block);
    }

    if (physical_block_num == 0) {
        fprintf(stderr, "Erro: Bloco fisico nao encontrado para o offset %u\n", offset);
        return -1;
    }

    uint32_t block_position = physical_block_num * block_size + offset_in_block;
    uint8_t temp[8];

    if (fseek(file, block_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o diretorio");
        return -1;
    }
    if (fread(temp, 1, 8, file) != 8) {
        return -1;
    }

    uint16_t rec_len = *(uint16_t *)&temp[4];
    if (rec_len < 8 || rec_len > block_size || rec_len % 4 != 0) {
        fprintf(stderr, "Tamanho de registro invalido: %u\n", rec_len);
        return -1;
    }

    *entry = malloc(rec_len);
    if (!*entry) {
        perror("Falha ao alocar memoria para a entrada de diretorio");
        return -1;
    }

    if (fseek(file, block_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o diretorio novamente");
        free(*entry);
        return -1;
    }
    if (fread(*entry, rec_len, 1, file) != 1) {
        perror("Erro ao ler a entrada completa do diretorio");
        free(*entry);
        return -1;
    }

    return 0;
}

int write_directory_entry(FILE *file, ext2_dir_entry *entry, Superblock *sb, inode *inode, uint32_t offset){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t block_number = inode->i_block[offset / block_size];
    if(block_number == 0){
        fprintf(stderr, "Número de bloco é 0\n");
        return -1;
    }

    uint32_t block_position = block_number * block_size + (offset % block_size);

    if (fseek(file, block_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o diretório");
        return -1;
    }
    if (fwrite(entry, entry->rec_len, 1, file) != 1) {
        perror("Erro ao escrever o diretório");
        return -1;
    }

    return 0;
}

void print_directory_entry(ext2_dir_entry *entry){
    printf("%.*s\n", entry->name_len, entry->name);
    printf("tamanho do nome: %u\n", entry->name_len);
    printf("Inode: %d\n", entry->inode);
    printf("Tipo: %d\n", entry->file_type);
    printf("Tamanho: %d\n", entry->rec_len);
    printf("\n");
}

// Funções de arquivo

int read_file_data(FILE *file, void *buffer, Superblock *sb, inode *inode, uint32_t offset, uint32_t size){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t block_number = inode->i_block[offset / block_size];

    uint32_t block_position = block_number * block_size + (offset % block_size);

    if (fseek(file, block_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o arquivo");
        return -1;
    }
    if (fread(buffer, size, 1, file) != 1) {
        perror("Erro ao ler o arquivo");
        return -1;
    }

    return 0;
}

int write_file_data(FILE *file, void *buffer, Superblock *sb, inode *inode, uint32_t offset, uint32_t size){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t block_number = inode->i_block[offset / block_size];

    uint32_t block_position = block_number * block_size + (offset % block_size);

    if (fseek(file, block_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o arquivo");
        return -1;
    }
    if (fwrite(buffer, size, 1, file) != 1) {
        perror("Erro ao escrever o arquivo");
        return -1;
    }
    return 0;
}

// Funções de Bitmap

int read_inode_bitmap(FILE *file, uint8_t *bitmap, Superblock *sb, block_group_descriptor *bgd, uint32_t group_number){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t inode_bitmap_block = bgd[group_number].bg_inode_bitmap;

    uint32_t inode_bitmap_position = inode_bitmap_block * block_size;

    uint32_t inode_bitmap_size = sb->s_inodes_per_group / 8;

    if (fseek(file, inode_bitmap_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o bitmap do inode");
        return -1;
    }
    if (fread(bitmap, inode_bitmap_size, 1, file) != 1) {
        perror("Erro ao ler o bitmap do inode");
        return -1;
    }

    return 0;
}

int write_inode_bitmap(FILE *file, const uint8_t *bitmap, Superblock *sb, block_group_descriptor *bgds, uint32_t group_number){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t inode_bitmap_block = bgds[group_number].bg_inode_bitmap;
    uint32_t inode_bitmap_position = inode_bitmap_block * block_size;
    
    uint32_t inode_bitmap_size = sb->s_inodes_per_group / 8;

    if (fseek(file, inode_bitmap_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o bitmap do inode");
        return -1;
    }
    if (fwrite(bitmap, inode_bitmap_size, 1, file) != 1) {
        perror("Erro ao escrever o bitmap do inode");
        return -1;
    }

    return 0;
}

int is_inode_used(const uint8_t *bitmap, uint32_t inode_number, uint32_t inodes_per_group){
    uint32_t index         = (inode_number - 1) % inodes_per_group;
    uint32_t byte_position = index / 8;
    uint32_t bit_position  = index % 8;

    int is_used = (bitmap[byte_position] >> bit_position) & 1;

    return is_used;
}

int read_block_bitmap(FILE* file, uint8_t *bitmap, Superblock *sb, block_group_descriptor *bgds, uint32_t group_number){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t block_bitmap_block = bgds[group_number].bg_block_bitmap;

    uint32_t block_bitmap_position = block_bitmap_block * block_size;

    uint32_t block_bitmap_size = sb->s_blocks_per_group / 8;


    if (fseek(file, block_bitmap_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o bitmap do bloco");
        return -1;
    }
    if (fread(bitmap, block_bitmap_size, 1, file) != 1) {
        perror("Erro ao ler o bitmap do bloco");
        return -1;
    }

    return 0;
}

int write_block_bitmap(FILE *file, const uint8_t *bitmap, Superblock *sb, block_group_descriptor *bgds, uint32_t group_number){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t block_bitmap_block = bgds[group_number].bg_block_bitmap;
    uint32_t block_bitmap_position = block_bitmap_block * block_size;
    
    uint32_t block_bitmap_size = sb->s_blocks_per_group / 8;

    if (fseek(file, block_bitmap_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o bitmap do bloco");
        return -1;
    }
    if (fwrite(bitmap, block_bitmap_size, 1, file) != 1) {
        perror("Erro ao escrever o bitmap do bloco");
        return -1;
    }

    return 0;
}

int is_block_used(const uint8_t *bitmap, uint32_t block_number, uint32_t blocks_per_group){
    uint32_t index         = (block_number - 1) % blocks_per_group;
    uint32_t byte_position = index / 8;
    uint32_t bit_position  = index % 8;

    int is_used = (bitmap[byte_position] >> bit_position) & 1;

    return is_used;
}

uint32_t path_to_inode(FILE *file, Superblock *sb, block_group_descriptor *bgds, const char *path, uint32_t start_inode_num){
    uint32_t current_inode_num = start_inode_num;

    // lida com caminhos vazios ou que são apenas a raiz
    if (path == NULL || path[0] == '\0') {
        return start_inode_num;
    }
    if (strcmp(path, "/") == 0) {
        return 2; // retorna o inode da raiz
    }

    // copia o path para não modificar o original
    char path_copy[512];
    strncpy(path_copy, path, sizeof(path_copy));
    path_copy[sizeof(path_copy) - 1] = '\0';

    char *token = strtok(path_copy, "/");

    while(token != NULL){
        // pula tokens vazios que podem ocorrer com barras duplas (ex: /dir//arquivo.txt)
        if (strlen(token) == 0) {
            token = strtok(NULL, "/");
            continue;
        }

        inode current_inode;
        if(read_inode(file, &current_inode, sb, bgds, current_inode_num) != 0){
            fprintf(stderr, "Erro ao ler inode %u\n", current_inode_num);
            return 0; // Retorna 0 em caso de erro (inode inválido)
        }

        // checa se o inode atual é um diretório antes de continuar.
        if (!S_ISDIR(current_inode.i_mode)) {
            fprintf(stderr, "Nao e um diretorio: %s\n", token);
            return 0;
        }
        
        // trata os casos . e ..
        if (strcmp(token, ".") == 0) {
            // . significa o diretório atual, logo não fazemos nada
        } else {
            uint32_t offset = 0;
            int found = 0;
            while(offset < current_inode.i_size){
                ext2_dir_entry *entry = NULL;
                // a leitura da entrada de diretório já é tratada dentro da função
                if(read_directory_entry(file, &entry, sb, &current_inode, offset) != 0){
                    free(entry);
                    break;
                }

                if (entry->inode != 0) {
                    char name[entry->name_len + 1];
                    memcpy(name, entry->name, entry->name_len);
                    name[entry->name_len] = '\0';

                    // compara o nome do token com a entrada do diretório
                    if (strcmp(name, token) == 0) {
                        current_inode_num = entry->inode;
                        found = 1;
                        free(entry);
                        break;
                    }
                }
                offset += entry->rec_len;
                free(entry);
            }

            if(!found){
                return 0; // retorna 0 se o inode do caminho não for encontrado
            }
        }
        token = strtok(NULL, "/");
    }
    return current_inode_num;
}

int inode_to_path(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t inode_num, char *path, size_t max_len) {
    inode target_inode;
    if (read_inode(file, &target_inode, sb, bgds, inode_num) != 0) {
        fprintf(stderr, "inode_to_path: nao foi possivel ler o inode %u\n", inode_num);
        return -1;
    }

    if (!S_ISDIR(target_inode.i_mode) && inode_num != 2) {
        return -1;
    }
    
    if (inode_num == 2) {
        strncpy(path, "/", max_len);
        return 0;
    }

    char *components[128];
    int depth = 0;
    uint32_t current_inode_num = inode_num;

    while (current_inode_num != 2 && depth < 128) {
        inode current_inode;
        inode parent;
        if (read_inode(file, &current_inode, sb, bgds, current_inode_num) != 0) 
            return -1;

        ext2_dir_entry *entry;
        if (read_directory_entry(file, &entry, sb, &current_inode, 0) != 0) 
            return -1;

        uint32_t offset = entry->rec_len;
        free(entry);

        if (read_directory_entry(file, &entry, sb, &current_inode, offset) != 0) 
            return -1;
        
        uint32_t parent_inode = entry->inode;
        free(entry);

        if (read_inode(file, &parent, sb, bgds, parent_inode) != 0) 
            return -1;

        // Procura o nome do inode atual dentro do pai
        offset = 0;
        int found = 0;
        while (offset < parent.i_size) {
            if (read_directory_entry(file, &entry, sb, &parent, offset) != 0) 
                break;
            char temp_name[256];
            memcpy(temp_name, entry->name, entry->name_len);
            temp_name[entry->name_len] = '\0';

            if (entry->inode == current_inode_num &&
                strcmp(temp_name, ".") != 0 &&
                strcmp(temp_name, "..") != 0) {

                char *name = malloc(entry->name_len + 1);
                memcpy(name, entry->name, entry->name_len);
                name[entry->name_len] = '\0';
                components[depth++] = name;
                free(entry);
                found = 1;
                break;
            }

            offset += entry->rec_len;
            free(entry);
        }

        if (!found) 
            return -1;
        current_inode_num = parent_inode;
    }

    // Monta caminho reverso
    path[0] = '\0';
    for (int i = depth - 1; i >= 0; i--) {
        strncat(path, "/", max_len - strlen(path) - 1);
        strncat(path, components[i], max_len - strlen(path) - 1);
        free(components[i]);
    }

    return 0;
}

uint32_t resolve_path(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path, char **return_path, size_t max_len) {
    uint32_t final_inode;

    if (path[0] == '/') {
        // caminho absoluto: começa a busca a partir da raiz (inode 2)
        final_inode = path_to_inode(file, sb, bgds, path, 2);
    } else {
        // caminho relativo: começa a busca a partir do inode atual
        final_inode = path_to_inode(file, sb, bgds, path, current_inode);
    }

    if (final_inode == 0) {
        return 0;
    }

    *return_path = malloc(sizeof(char) * max_len);
    if (*return_path == NULL) {
        fprintf(stderr, "Falha ao alocar memoria para o caminho\n");
        return 0;
    }

    if (inode_to_path(file, sb, bgds, final_inode, *return_path, max_len) != 0) {
        if (final_inode == 2) strncpy(*return_path, "/", max_len);
        else snprintf(*return_path, max_len, "/<inode:%u>", final_inode);
    }

    return final_inode;
}

void format_permissions(uint16_t mode, char *permissions){
    permissions[0] = S_ISDIR(mode) ? 'd' :
                     S_ISREG(mode) ? 'f' :
                     S_ISLNK(mode) ? 'l' :
                     S_ISCHR(mode) ? 'c' :
                     S_ISBLK(mode) ? 'b' :
                     S_ISFIFO(mode) ? 'p' :
                     S_ISSOCK(mode) ? 's' : '?';

    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';
    permissions[10] = '\0';
}

void format_size(uint32_t size, char *output, size_t str_len){
    const char *units[] = {"B", "KiB", "MiB", "GiB", "TiB"};
    int unit_index = 0;
    double size_double = (double)size;

    while (size_double >= 1024.0 && unit_index < 4) {
        size_double /= 1024.0;
        unit_index++;
    }

    snprintf(output, str_len, "%.2f %s", size_double, units[unit_index]);
}

int read_and_print_block(FILE *file, uint32_t block, uint8_t *buffer, uint32_t block_size, uint32_t *size_left) {
    if (block == 0) return 0;

    if (*size_left == 0) return 0;

    uint64_t to_read = (*size_left < block_size) ? *size_left : block_size;

    if (fseek(file, block * block_size, SEEK_SET) != 0)
        return -1;

    if (fread(buffer, 1, to_read, file) != to_read)
        return -1;

    fwrite(buffer, 1, to_read, stdout);
    *size_left -= to_read;

    return 0;
}

int is_inode_dir(const inode *node) {
    uint16_t file_type = node->i_mode & 0xF000; // pega os 4 bits mais significativos do campo i_mode, que mostra o tipo do arquivo
    
    uint16_t dir_type = 0x4000; // tipo diretorio

    return file_type == dir_type;
}

void deallocate_block(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t block_num) {
    if (block_num == 0) return;

    uint32_t group = (block_num - sb->s_first_data_block) / sb->s_blocks_per_group;
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint8_t *block_bitmap = malloc(block_size);
    if (!block_bitmap) {
        perror("Falha ao alocar memoria para o bitmap de blocos");
        return;
    }

    if (read_block_bitmap(file, block_bitmap, sb, bgds, group) != 0) {
        fprintf(stderr, "Falha ao ler o bitmap de blocos para o grupo %u\n", group);
        free(block_bitmap);
        return;
    }

    uint32_t index = (block_num - sb->s_first_data_block) % sb->s_blocks_per_group;
    uint32_t byte_pos = index / 8;
    uint8_t bit_pos = index % 8;

    if (!((block_bitmap[byte_pos] >> bit_pos) & 1)) {
        fprintf(stderr, "Aviso: O bloco %u ja esta livre.\n", block_num);
        free(block_bitmap);
        return;
    }
    
    block_bitmap[byte_pos] &= ~(1 << bit_pos);

    if (write_block_bitmap(file, block_bitmap, sb, bgds, group) != 0) {
        fprintf(stderr, "Falha ao escrever o bitmap de blocos para o grupo %u\n", group);
        free(block_bitmap);
        return;
    }

    free(block_bitmap);

    sb->s_free_blocks_count++;
    bgds[group].bg_free_blocks_count++;
}

void deallocate_inode_blocks(FILE *file, Superblock *sb, block_group_descriptor *bgds, inode *target_inode) {
    uint32_t block_size = 1024 << sb->s_log_block_size;
    uint32_t pointers_per_block = block_size / sizeof(uint32_t);

    // desaloca blocos diretos
    for (int i = 0; i < 12; i++) {
        if (target_inode->i_block[i] != 0) {
            deallocate_block(file, sb, bgds, target_inode->i_block[i]);
        }
    }

    // desaloca bloco indireto simples
    if (target_inode->i_block[12] != 0) {
        uint32_t *indirect_block = malloc(block_size);
        if (indirect_block && read_block(file, indirect_block, target_inode->i_block[12], block_size) == 0) {
            for (uint32_t i = 0; i < pointers_per_block; i++) {
                if (indirect_block[i] != 0) {
                    deallocate_block(file, sb, bgds, indirect_block[i]); // desaloca bloco de dados
                }
            }
        }
        if (indirect_block) free(indirect_block);
        deallocate_block(file, sb, bgds, target_inode->i_block[12]); // desaloca o bloco de ponteiros
    }

    // desaloca bloco indireto duplo
    if (target_inode->i_block[13] != 0) {
        uint32_t *double_indirect_block = malloc(block_size);
        if (double_indirect_block && read_block(file, double_indirect_block, target_inode->i_block[13], block_size) == 0) {
            for (uint32_t i = 0; i < pointers_per_block; i++) {
                if (double_indirect_block[i] != 0) { // ponteiro para um bloco indireto simples
                    uint32_t *indirect_block = malloc(block_size);
                    if (indirect_block && read_block(file, indirect_block, double_indirect_block[i], block_size) == 0) {
                        for (uint32_t j = 0; j < pointers_per_block; j++) {
                            if (indirect_block[j] != 0) {
                                deallocate_block(file, sb, bgds, indirect_block[j]); // desaloca bloco de dados
                            }
                        }
                    }
                    if (indirect_block) free(indirect_block);
                    deallocate_block(file, sb, bgds, double_indirect_block[i]); // desaloca o bloco indireto simples
                }
            }
        }
        if (double_indirect_block) free(double_indirect_block);
        deallocate_block(file, sb, bgds, target_inode->i_block[13]); // desaloca o bloco de ponteiros duplos
    }

    // desaloca bloco indireto triplo
    if (target_inode->i_block[14] != 0) {
        uint32_t *triple_indirect_block = malloc(block_size);
        if (triple_indirect_block && read_block(file, triple_indirect_block, target_inode->i_block[14], block_size) == 0) {
            for (uint32_t i = 0; i < pointers_per_block; i++) {
                if (triple_indirect_block[i] != 0) { // ponteiro para um bloco indireto duplo
                    uint32_t *double_indirect_block = malloc(block_size);
                    if (double_indirect_block && read_block(file, double_indirect_block, triple_indirect_block[i], block_size) == 0) {
                        for (uint32_t j = 0; j < pointers_per_block; j++) {
                            if (double_indirect_block[j] != 0) { // ponteiro para um bloco indireto simples
                                uint32_t *indirect_block = malloc(block_size);
                                if (indirect_block && read_block(file, indirect_block, double_indirect_block[j], block_size) == 0) {
                                    for (uint32_t k = 0; k < pointers_per_block; k++) {
                                        if (indirect_block[k] != 0) {
                                            deallocate_block(file, sb, bgds, indirect_block[k]); // desaloca bloco de dados
                                        }
                                    }
                                }
                                if (indirect_block) free(indirect_block);
                                deallocate_block(file, sb, bgds, double_indirect_block[j]); // desaloca o bloco indireto simples
                            }
                        }
                    }
                    if (double_indirect_block) free(double_indirect_block);
                    deallocate_block(file, sb, bgds, triple_indirect_block[i]); // desaloca o bloco indireto duplo
                }
            }
        }
        if (triple_indirect_block) free(triple_indirect_block);
        deallocate_block(file, sb, bgds, target_inode->i_block[14]); // desaloca o próprio bloco de ponteiros triplos
    }

    // zera os campos no inode
    target_inode->i_blocks = 0;
    target_inode->i_size = 0;
    for(int i=0; i<15; i++) {
        target_inode->i_block[i] = 0;
    }
}

void deallocate_inode_metadata(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t inode_num) {
    if (inode_num < sb->s_first_ino) {
         return;
    }
    uint32_t group = (inode_num - 1) / sb->s_inodes_per_group;
    uint32_t inode_bitmap_size = sb->s_inodes_per_group / 8;
    uint8_t *inode_bitmap = malloc(inode_bitmap_size);
    if (!inode_bitmap) {
        perror("Falha ao alocar memoria para o bitmap de inodes");
        return;
    }

    if (read_inode_bitmap(file, inode_bitmap, sb, bgds, group) != 0) {
        fprintf(stderr, "Falha ao ler o bitmap de inodes para o grupo %u\n", group);
        free(inode_bitmap);
        return;
    }

    uint32_t index = (inode_num - 1) % sb->s_inodes_per_group;
    uint32_t byte_pos = index / 8;
    uint8_t bit_pos = index % 8;

    if (!((inode_bitmap[byte_pos] >> bit_pos) & 1)) {
        fprintf(stderr, "Aviso: O inode %u ja esta livre.\n", inode_num);
        free(inode_bitmap);
        return;
    }

    inode_bitmap[byte_pos] &= ~(1 << bit_pos);

    if (write_inode_bitmap(file, inode_bitmap, sb, bgds, group) != 0) {
        fprintf(stderr, "Falha ao escrever o bitmap de inodes para o grupo %u\n", group);
        free(inode_bitmap);
        return;
    }

    free(inode_bitmap);

    sb->s_free_inodes_count++;
    bgds[group].bg_free_inodes_count++;
}

void copy_data_block(FILE *ext2_image_file, FILE *host_file, uint32_t block_num, uint8_t *buffer, uint32_t block_size, uint32_t *size_left) {
    if (block_num == 0 || *size_left == 0) {
        return;
    }

    // Determina quantos bytes ler (o mínimo entre um bloco inteiro e o que falta)
    uint32_t bytes_to_read = (*size_left < block_size) ? *size_left : block_size;
    
    // Lê o bloco da imagem EXT2 para o buffer
    if (read_block(ext2_image_file, buffer, block_num, block_size) == 0) {
        // Escreve o conteúdo do buffer no arquivo do sistema hospedeiro
        fwrite(buffer, 1, bytes_to_read, host_file);
        // Decrementa o contador de bytes restantes
        *size_left -= bytes_to_read;
    }
}
