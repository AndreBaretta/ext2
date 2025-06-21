#include <stdio.h>
#include <stdlib.h>
#include "include/EXT2.h"
#include "include/EXT2_Utils.h"

int main() {
    FILE *file = fopen("myext2image.img", "rb");
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
    
    int block_group_count = (sb.s_blocks_count + sb.s_blocks_per_group - 1) / sb.s_blocks_per_group;
    block_group_descriptor *bgd = malloc(sizeof(block_group_descriptor) * block_group_count);

    for (int i = 0; i < block_group_count; i++) {
        if (read_block_group_descriptor(file, &bgd[i], &sb, i) != 0) {
            fprintf(stderr, "Erro ao ler o block group descriptor\n");
            fclose(file);
            return 1;
        }   
    }

    for (int i = 0; i < block_group_count; i++) {
        print_block_group_descriptor(&bgd[i], i);
    }

    inode inode;

    int inode_number = 2;
    if (read_inode(file, &inode, &sb, bgd, inode_number) != 0) {
        fprintf(stderr, "Erro ao ler o inode\n");
        fclose(file);
        return 1;
    }
    print_inode(&inode, inode_number);
    // print_superblock(&sb);


    fclose(file);
    return 0;
}