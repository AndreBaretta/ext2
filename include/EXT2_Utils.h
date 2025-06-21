#ifndef EXT2_UTILS
#define EXT2_UTILS

#include <stdio.h>
#include "EXT2.h"


// Funções de leitura de estruturas
int read_superblock(FILE *file, Superblock *sb);
int read_block(FILE *file, void *buffer, uint32_t block_number, uint32_t block_size);
int read_block_group_descriptor(FILE *file, block_group_descriptor *bgd, Superblock *sb, uint32_t group_number);
int read_inode(FILE *file, inode *inode, Superblock *sb, block_group_descriptor *bgd, uint32_t inode_number);

// Funções de escrita de estruturas
int write_superblock(FILE *file, Superblock *sb);
int write_block(FILE *file, void *buffer, uint32_t block_number, uint32_t block_size);
int write_block_group_descriptor(FILE *file, block_group_descriptor *bgd, Superblock *sb, uint32_t group_number);
int write_inode(FILE *file, inode *inode, Superblock *sb, block_group_descriptor *bgd, uint32_t inode_number);

// Funções de print
void print_superblock(Superblock *sb);
void print_uuid(const char uuid[16]);
void print_block_group_descriptor(block_group_descriptor *bgd, uint32_t number);
void print_inode(inode *inode, uint32_t number);

#endif
