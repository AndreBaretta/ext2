#include <stdio.h>
#include <stdlib.h>
#include "shell.h"

// Exibe informações do disco e do sistema de arquivos
void cmd_info(void) {
    printf("Comando info chamado\n");
    // TODO: implementar leitura e exibição das informações do superbloco e outros dados do disco
}

// Exibe o conteúdo de um arquivo texto
void cmd_cat(const char *filename) {
    printf("Comando cat chamado para arquivo: %s\n", filename);
    // TODO: implementar leitura do conteúdo do arquivo e exibição no terminal
}

// Exibe os atributos de um arquivo ou diretório
void cmd_attr(const char *path) {
    printf("Comando attr chamado para: %s\n", path);
    // TODO: implementar leitura dos atributos do inode (permissões, tamanho, datas etc.)
}

// Altera o diretório corrente para o definido em path
int cmd_cd(const char *path) {
    printf("Comando cd chamado para: %s\n", path);
    // TODO: alterar diretório corrente (mudar inode atual para o inode do path)
    return 0;
}

// Lista arquivos e diretórios do diretório corrente
void cmd_ls(void) {
    printf("Comando ls chamado\n");
    // TODO: listar entradas do diretório corrente
}

// Exibe o diretório corrente (caminho absoluto)
void cmd_pwd(void) {
    printf("Comando pwd chamado\n");
    // TODO: imprimir caminho absoluto do diretório atual
}

// Cria um arquivo vazio com o nome file
int cmd_touch(const char *filename) {
    printf("Comando touch chamado para arquivo: %s\n", filename);
    // TODO: criar arquivo vazio
    return 0;
}

// Cria um diretório vazio com o nome dir
int cmd_mkdir(const char *dirname) {
    printf("Comando mkdir chamado para diretório: %s\n", dirname);
    // TODO: criar diretório vazio
    return 0;
}

// Remove o arquivo especificado
int cmd_rm(const char *filename) {
    printf("Comando rm chamado para arquivo: %s\n", filename);
    // TODO: remover arquivo
    return 0;
}

// Remove o diretório especificado, se estiver vazio
int cmd_rmdir(const char *dirname) {
    printf("Comando rmdir chamado para diretório: %s\n", dirname);
    // TODO: remover diretório (se vazio)
    return 0;
}

// Renomeia um arquivo de file para newfilename
int cmd_rename(const char *file, const char *newfilename) {
    printf("Comando rename chamado para arquivo: %s -> %s\n", file, newfilename);
    // TODO: renomear arquivo
    return 0;
}

// Copia arquivo source_path para target_path
int cmd_cp(const char *source_path, const char *target_path) {
    printf("Comando cp chamado de: %s para %s\n", source_path, target_path);
    // TODO: copiar arquivo
    return 0;
}

// Move arquivo source_path para target_path
int cmd_mv(const char *source_path, const char *target_path) {
    printf("Comando mv chamado de: %s para %s\n", source_path, target_path);
    // TODO: mover arquivo
    return 0;
}
