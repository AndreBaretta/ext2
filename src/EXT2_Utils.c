//  - Descrição:............ Código responsável por implementar as funções básicas de leitura e escrita de estruturas do sistema de arquivos EXT2
//  - Autor:................ André Felipe Baretta, João Pedro Inoe
//  - Data de criação:...... 29/05/2025
//  - Datas de atualização:. 29/05/2025, 19/06/2025, 20/06/2025, 21/06/2025.

#include <stdlib.h>
#include <stdio.h>
#include "../include/EXT2.h"
#include "../include/EXT2_Utils.h"

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

void print_uuid(const char uuid[16]) {
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
    print_uuid(sb->s_uuid);
    printf("Volume name: %s\n", sb->s_volume_name);
    printf("Last mounted: %s\n", sb->s_last_mounted);
    printf("Algo bitmap: %d\n", sb->s_algo_bitmap);
    printf("Prealloc blocks: %d\n", sb->s_prealloc_blocks);
    printf("Prealloc dir blocks: %d\n", sb->s_prealloc_dir_blocks);
    printf("Padding: %d\n", sb->s_padding1);
    printf("Journal UUID: ");
    print_uuid(sb->s_journal_uuid);
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

int read_directory_entry(FILE *file, ext2_dir_entry **entry, Superblock *sb, inode *inode, uint32_t offset){
    uint32_t block_size = 1024 << sb->s_log_block_size;    

    uint32_t block_number = inode->i_block[offset / block_size];
    if(block_number == 0){
        fprintf(stderr, "Block number is 0\n");
        return -1;
    }
    uint32_t block_position = block_number * block_size + offset % block_size;

    uint8_t temp[8];

    if (fseek(file, block_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o diretório");
        return -1;
    }
    if (fread(temp, 1, 8, file) != 8) {
        perror("Erro ao ler o diretório");
        return -1;
    }

    uint16_t rec_len = *(uint16_t *)&temp[4];

    if(rec_len < 8 || rec_len > block_size || rec_len % 4 != 0){
        fprintf(stderr, "Tamanho de registro inválido\n");
        return -1;
    }

    *entry = malloc(rec_len);

    if (fseek(file, block_position, SEEK_SET) != 0) {
        perror("Erro ao buscar o diretório");
        return -1;
    }
    if (fread(*entry, rec_len, 1, file) != 1) {
        perror("Erro ao ler o diretório");
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

