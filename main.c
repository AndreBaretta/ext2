#include <stdio.h>
#include <stdlib.h>
#include "include/EXT2.h"
#include "include/EXT2_Utils.h"

int main() {
    FILE *file = fopen("myext2image.img", "rb+");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    Superblock sb;
    
    if (read_superblock(file, &sb) != 0) {
        fprintf(stderr, "Erro ao ler o superblock\n");
        fclose(file);
        return 1;
    }

    int group_number = (sb.s_inodes_count + sb.s_inodes_per_group - 1) / sb.s_inodes_per_group;

    block_group_descriptor* bgds = malloc(sizeof(block_group_descriptor) * group_number);

    for(int i = 0; i < group_number; i++){
        read_block_group_descriptor(file, &bgds[i], &sb, i);
    }

    inode inode;
    if(read_inode(file, &inode, &sb, bgds, 2) != 0){
        fprintf(stderr, "Erro ao ler o inode\n");
        fclose(file);
        free(bgds);
        return 1;
    }

    uint32_t offset = 0;
    while (offset < inode.i_size) {
        ext2_dir_entry *entry = NULL;
        if (read_directory_entry(file, &entry, &sb, &inode, offset) != 0) {
            break;
        }

        print_directory_entry(entry);

        if (entry->rec_len == 0) {
            fprintf(stderr, "rec_len == 0\n");
            free(entry);
            break;
        }

        offset += entry->rec_len;
        free(entry);
    }

    fclose(file);
    free(bgds);
    return 0;
}