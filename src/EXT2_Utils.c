#include <stdlib.h>
#include <stdio.h>
#include "../include/EXT2.h"
#include "../include/EXT2_Utils.h"

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

int read_block(FILE *file, void *buffer, int block_number, int block_size){
    if (fseek(file, block_number * block_size, SEEK_SET) != 0) {
        perror("Error seeking to block");
        return -1;
    }
    if (fread(buffer, block_size, 1, file) != 1) {
        perror("Error reading block");
        return -1;
    }
    return 0;
}

int read_block_group_descriptor(FILE *file, block_group_descriptor *bgd, Superblock *sb, int group_number){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t descriptor_table_block = (block_size == 1024) ? 2 : 1;
    uint32_t descriptor_table_position = descriptor_table_block * block_size;

    uint32_t position = descriptor_table_position + group_number * sizeof(block_group_descriptor);

    if (fseek(file, position, SEEK_SET) != 0) {
        perror("Error seeking to block group descriptor");
        return -1;
    }
    if (fread(bgd, sizeof(block_group_descriptor), 1, file) != 1) {
        perror("Error reading block group descriptor");
        return -1;
    }

    return 0;
}

void print_block_group_descriptor(block_group_descriptor *bgd, int number){
    printf("Block group descriptor %d:\n", number);
    printf("Block bitmap: %d\n", bgd->bg_block_bitmap);
    printf("Inode bitmap: %d\n", bgd->bg_inode_bitmap);
    printf("Inode table: %d\n", bgd->bg_inode_table);
    printf("Free blocks count: %d\n", bgd->bg_free_blocks_count);   
    printf("Free inodes count: %d\n", bgd->bg_free_inodes_count);
    printf("Used dirs count: %d\n", bgd->bg_used_dirs_count);
    printf("\n");
}

int read_inode(FILE *file, inode *inode, Superblock *sb, block_group_descriptor *bgd_list, int inode_number){
    uint32_t block_size = 1024 << sb->s_log_block_size;

    uint32_t group = (inode_number - 1) / sb->s_inodes_per_group;
    uint32_t index = (inode_number - 1) % sb->s_inodes_per_group;

    uint32_t inode_table_position = bgd_list[group].bg_inode_table * block_size;

    uint32_t inode_position = inode_table_position + index * sb->s_inode_size;  

    if (fseek(file, inode_position, SEEK_SET) != 0) {
        perror("Error seeking to inode");
        return -1;
    }
    if (fread(inode, sb->s_inode_size, 1, file) != 1) {
        perror("Error reading inode");
        return -1;
    }
    
    return 0;
}

void print_inode(inode *inode, int number){
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
    printf("File ACL: %d\n", inode->i_file_acl);
    printf("Directory ACL: %d\n", inode->i_dir_acl);
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
