// - Descrição:............ Struct de Inodes
// - Autor:................ André Felipe Baretta
// - Data de Criação:...... 25/05/2025
// - Data de Atualização:.. 26/05/2025

#include <stdint.h>

typedef struct {
   char h_i_frag;
   char h_i_fsize;
   uint16_t h_i_mode_high;
   uint16_t h_i_uid_high;
   uint16_t h_i_gid_high;
   uint32_t h_i_author;
} i_osd2;

typedef struct {
   uint16_t i_mode;
   uint16_t i_uid;
   uint32_t i_size;
   uint32_t i_atime;
   uint32_t i_ctime;
   uint32_t i_mtime;
   uint32_t i_dtime;
   uint16_t i_gid;
   uint16_t i_links_count;
   uint32_t i_blocks;
   uint32_t i_flags;
   uint32_t i_ctime;
   uint32_t i_block[15];
   uint32_t i_generation;
   uint32_t i_file_acl;
   uint32_t i_dir_acl;
   uint32_t i_faddr;
   i_osd2 i_osd2;
} Inode;

