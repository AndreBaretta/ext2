//  - Descrição:............ Código responsável por declarar as funções do shell
//  - Autor:................ André Felipe Baretta, João Pedro Inoe
//  - Data de criação:...... 29/05/2025
//  - Datas de atualização:. 29/05/2025, 21/06/2025, 22/06/2025.

#ifndef SHELL_H
#define SHELL_H

// Suas funções de comando:
void cmd_info(void);
void cmd_cat(const char *filename);
int  cmd_attr(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path);
int  cmd_cd(FILE *file, Superblock *sb, block_group_descriptor *bgds, const char *path, char *current_path, uint32_t *current_inode);
int  cmd_ls(FILE *file, Superblock *sb, block_group_descriptor *bgds, uint32_t current_inode, const char *path);
int  cmd_pwd(const char* current_path);
int  cmd_touch(const char *filename);
int  cmd_mkdir(const char *dirname);
int  cmd_rm(const char *filename);
int  cmd_rmdir(const char *dirname);
int  cmd_rename(const char *file, const char *newfilename);
int  cmd_cp(const char *source_path, const char *target_path);
int  cmd_mv(const char *source_path, const char *target_path);

// Função principal do shell
void shell_loop(FILE *file);

#endif
