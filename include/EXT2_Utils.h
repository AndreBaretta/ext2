//  - Descrição:............ Código responsável por declarar as funções básicas de leitura e escrita de estruturas do sistema de arquivos EXT2
//  - Autor:................ André Felipe Baretta
//  - Data de criação:...... 19/06/2025
//  - Datas de atualização:. 19/06/2025, 20/06/2025, 21/06/2025, 22/06/2025.

#ifndef EXT2_UTILS
#define EXT2_UTILS

#include <stdio.h>
#include "EXT2.h"


// Funções de leitura de estruturas
int read_superblock(FILE *file, Superblock *sb);
int read_block(FILE *file, void *buffer, uint32_t block_number, uint32_t block_size);
int read_block_group_descriptor(FILE *file, block_group_descriptor *bgd, Superblock *sb, uint32_t group_number);
int read_inode(FILE *file, inode *inode, Superblock *sb, block_group_descriptor *bgds, uint32_t inode_number);
int read_directory_entry(FILE *file, ext2_dir_entry **entry, Superblock *sb, inode *inode, uint32_t offset);
int read_inode_bitmap(FILE *file, uint8_t *bitmap, Superblock *sb, block_group_descriptor *bgds, uint32_t group_number);
int read_block_bitmap(FILE *file, uint8_t *bitmap, Superblock *sb, block_group_descriptor *bgds, uint32_t group_number);

// Funções de escrita de estruturas
int write_superblock(FILE *file, Superblock *sb);
int write_block(FILE *file, void *buffer, uint32_t block_number, uint32_t block_size);
int write_block_group_descriptor(FILE *file, block_group_descriptor *bgd, Superblock *sb, uint32_t group_number);
int write_inode(FILE *file, inode *inode, Superblock *sb, block_group_descriptor *bgd, uint32_t inode_number);
int write_directory_entry(FILE *file, ext2_dir_entry *entry, Superblock *sb, inode *inode, uint32_t offset);
int write_inode_bitmap(FILE *file, const uint8_t *bitmap, Superblock *sb, block_group_descriptor *bgds, uint32_t group_number);
int write_block_bitmap(FILE *file, const uint8_t *bitmap, Superblock *sb, block_group_descriptor *bgds, uint32_t group_number);

// Funções de print
void print_superblock(Superblock *sb);
void print_uuid(const char uuid[16]);
void print_block_group_descriptor(block_group_descriptor *bgd, uint32_t number);
void print_inode(inode *inode, uint32_t number);
void print_directory_entry(ext2_dir_entry *entry);

// Funções auxiliares
int is_inode_used(const uint8_t *bitmap, uint32_t inode_number, uint32_t inodes_per_group);
int is_block_used(const uint8_t *bitmap, uint32_t block_number, uint32_t blocks_per_group);
uint32_t path_to_inode(FILE *file, Superblock *sb, block_group_descriptor *bgds, const char *path);
int inode_to_path(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t inode_num, char *path, size_t max_len);

#endif
