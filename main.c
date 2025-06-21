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

    print_superblock(&sb);

    fclose(file);
    return 0;
}