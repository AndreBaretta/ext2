#include <stdio.h>
#include <stdlib.h>
#include "include/EXT2.h"
#include "include/EXT2_Utils.h"
#include "Shell/Shell.h"

int main(int argc, char **argv) {
    if(argc != 2){
        printf("Insira o caminho para uma imagem como argumento.\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "rb+");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    shell_loop(file);

    return 0;
}