#include <stdlib.h>
#include <stdio.h>
#include "EXT2.h"

//Operações de leitura e escrita no sistema de arquivos EXT2:
int read_superblock(FILE *file, Superblock *sb){
    if (fseek(file, 1024, SEEK_SET) != 0) {
        perror("Error seeking to superblock");
        return -1;
    }
    if (fread(sb, sizeof(Superblock), 1, file) != 1) {
        perror("Error reading superblock");
        return -1;
    }
    return 0;
}
// int write_superblock(FILE *file, Superblock *sb);
// int read_block_group_descriptor(FILE *file, BlockGroupDescriptor *bgd, int group_number);
// int write_block_group_descriptor(FILE *file, BlockGroupDescriptor *bgd, int group_number);
// int read_inode(FILE *file, Inode *inode, int inode_number);
// int write_inode(FILE *file, Inode *inode, int inode_number);
// int read_block(FILE *file, void *buffer, int block_number);
// int write_block(FILE *file, const void *buffer, int block_number);
// int read_inode_bitmap(FILE *file, uint8_t *bitmap, int group_number);
// int write_inode_bitmap(FILE *file, const uint8_t *bitmap, int group_number);
// int read_block_bitmap(FILE *file, uint8_t *bitmap, int group_number);
// int write_block_bitmap(FILE *file, const uint8_t *bitmap, int group_number);
// int read_directory_entry(FILE *file, DirectoryEntry *entry, int inode_number, int offset);
// int write_directory_entry(FILE *file, const DirectoryEntry *entry, int inode_number, int offset);
// int read_file_data(FILE *file, void *buffer, int inode_number, int offset, size_t size);
// int write_file_data(FILE *file, const void *buffer, int inode_number, int offset, size_t size);
    