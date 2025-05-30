#ifndef SHELL_H
#define SHELL_H

// Exibe informações do disco e do sistema de arquivos
void cmd_info(void);

// Exibe o conteúdo de um arquivo texto
void cmd_cat(const char *filename);

// Exibe os atributos de um arquivo ou diretório
void cmd_attr(const char *path);

// Altera o diretório corrente para o definido em path
int cmd_cd(const char *path);

// Lista arquivos e diretórios do diretório corrente
void cmd_ls(void);

// Exibe o diretório corrente (caminho absoluto)
void cmd_pwd(void);

// Cria um arquivo vazio com o nome file
int cmd_touch(const char *filename);

// Cria um diretório vazio com o nome dir
int cmd_mkdir(const char *dirname);

// Remove o arquivo especificado
int cmd_rm(const char *filename);

// Remove o diretório especificado, se estiver vazio
int cmd_rmdir(const char *dirname);

// Renomeia um arquivo de file para newfilename
int cmd_rename(const char *file, const char *newfilename);

// Copia arquivo source_path para target_path
int cmd_cp(const char *source_path, const char *target_path);

// Move arquivo source_path para target_path
int cmd_mv(const char *source_path, const char *target_path);

#endif // SHELL_H
