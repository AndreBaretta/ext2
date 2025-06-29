//  - Descrição:............ Código responsável por declarar as funções do shell
//  - Autor:................ André Felipe Baretta, João Pedro Inoe
//  - Data de criação:...... 29/05/2025
//  - Datas de atualização:. 29/05/2025, 21/06/2025, 22/06/2025, 23/06/2025, 24/06/2025, 27/06/2026, 28/06/2025.

#ifndef SHELL_H
#define SHELL_H

// Funções de comando
int  cmd_info(Superblock *sb); // Mostra informações do disco
int  cmd_cat(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *filename);  // Imprime o conteúdo de um arquivo
int  cmd_attr(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path); // Exibe os atributos de um arquivo ou diretório
int  cmd_cd(FILE *file, Superblock *sb, block_group_descriptor *bgds, const char *path, char *current_path, uint32_t *current_inode); // Altera o diretorio atual
int  cmd_ls(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path); // Lista os arquivos e diretórios dentro de um diretório
int  cmd_pwd(const char* current_path); // Imprime o diretório atual
int  cmd_touch(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path); // Cria um arquivo vazio
int  cmd_mkdir(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path); // Cria um diretorio vazio
int  cmd_rm(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path);     // Exclui um arquivo
int  cmd_rmdir(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path);    // Exclui um diretório
int  cmd_rename(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *source_name, const char *new_name); // Renomeia um arquivo
int  cmd_cp(FILE *file, Superblock *sb, block_group_descriptor *bgds, const char *source_ext2_path, const char *dest_host_path);  // Copia um arquivo da imagem para o SO hospedeiro
// int  cmd_mv(const char *source_path, const char *target_path);  // Move um arquivo de um path para outro

// Função principal do shell
void shell_loop(FILE *file);

#endif
