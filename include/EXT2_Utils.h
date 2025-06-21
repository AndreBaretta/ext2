#ifndef EXT2_UTILS
#define EXT2_UTILS

#include <stdio.h>
#include "EXT2.h"

int read_superblock(FILE *file, Superblock *sb);


int read_block(FILE *file, void *buffer, int block_number, int block_size);
int read_block_group_descriptor(FILE *file, block_group_descriptor *bgd, Superblock *sb, int group_number);
int read_inode(FILE *file, inode *inode, Superblock *sb, block_group_descriptor *bgd, int inode_number);

// Funções de print
void print_superblock(Superblock *sb);
void print_uuid(const char uuid[16]);
void print_block_group_descriptor(block_group_descriptor *bgd, int number);
void print_inode(inode *inode, int number);

#endif
