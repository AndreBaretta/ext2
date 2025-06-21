//  - Descrição:............ Código responsável por declarar as estruturas de dados do sistema de arquivos EXT2
//  - Autor:................ André Felipe Baretta, Henry Meneguini Farias, João Pedro Inoe
//  - Data de criação:...... 26/05/2025
//  - Datas de atualização:. 26/05/2025, 29/05/2025, 21/06/2025.

#ifndef EXT2
#define EXT2
#include <stdint.h>

//Estruturas de dados do sistesma de arquivos EXT2

//***SUPERBLOCK:

#pragma pack(push,1)

typedef struct {
   uint32_t s_inodes_count;        // Número total de inodes
   uint32_t s_blocks_count;        // Número total de blocos
   uint32_t s_r_blocks_count;      // Número de blocos reservados
   uint32_t s_free_blocks_count;   // Número de blocos livres
   uint32_t s_free_inodes_count;   // Número de inodes livres
   uint32_t s_first_data_block;    // Número do primeiro bloco de dados
   uint32_t s_log_block_size;      // Tamanho do bloco em potências de 2
   uint32_t s_log_frag_size;       // Tamanho do fragmento em potências de 2
   uint32_t s_blocks_per_group;    // Número de blocos por grupo
   uint32_t s_frags_per_group;     // Número de fragmentos por grupo
   uint32_t s_inodes_per_group;    // Número de inodes por grupo
   uint32_t s_mtime;               // Hora da última montagem
   uint32_t s_wtime;               // Hora da última escrita
   uint16_t s_mnt_count;           // Contador de montagens
   uint16_t s_max_mnt_count;       // Contador máximo de montagens
   uint16_t s_magic;              // Número mágico (0xEF53 para EXT2)
   uint16_t s_state;              // Estado do sistema de arquivos (0x0001 = montado)
   uint16_t s_errors;             // Comportamento em caso de erro (0x0001 = continuar)
   uint16_t s_minor_rev_level;    // Nível de revisão menor
   uint32_t s_lastcheck;          // Hora da última verificação
   uint32_t s_checkinterval;      // Intervalo entre verificações
   uint32_t s_creator_os;         // Sistema operacional criador (0x00000001 = Linux)
   uint32_t s_rev_level;          // Nível de revisão do sistema de arquivos
   uint16_t s_def_resuid;         // UID do usuário reservado por padrão
   uint16_t s_def_resgid;         // GID do grupo reservado por padrão
   uint32_t s_first_ino;          // Número do primeiro inode (normalmente 11)
   uint16_t s_inode_size;         // Tamanho do inode em bytes
   uint16_t s_block_group_nr;     // Número do grupo de blocos
   uint32_t s_feature_compat;     // Recursos compatíveis
   uint32_t s_feature_incompat;   // Recursos incompatíveis
   uint32_t s_feature_ro_compat;  // Recursos somente leitura compatíveis
   char s_uuid[16];               // UUID do sistema de arquivos
   char s_volume_name[16];        // Nome do volume
   char s_last_mounted[64];       // Último ponto de montagem
   uint32_t s_algo_bitmap;        // Bitmap de uso de algoritmos
   uint8_t s_prealloc_blocks;     // Número de blocos pré-alocados
   uint8_t s_prealloc_dir_blocks; // Número de blocos pré-alocados para diretórios
   uint16_t s_padding1;           // Preenchimento
   uint32_t s_reserved[204];      // Reservado para uso futuro
   uint8_t s_journal_uuid[16];    // UUID do journal
   uint32_t s_journal_inum;       // Número do inode do journal
} Superblock;

//***BLOCK GROUP DESCRIPTOR:

typedef struct {
   uint32_t bg_block_bitmap;
   uint32_t bg_inode_bitmap;
   uint32_t bg_inode_table;
   uint16_t bg_free_blocks_count;
   uint16_t bg_free_inodes_count;
   uint16_t bg_used_dirs_count;
   uint16_t bg_pad;
   char bg_reserved[12];
} block_group_descriptor;

//***I_OSD2:
typedef struct {
   char h_i_frag;
   char h_i_fsize;
   uint16_t h_i_mode_high;
   uint16_t h_i_uid_high;
   uint16_t h_i_gid_high;
   uint32_t h_i_author;
} i_osd2;

//***INODE:
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
   uint32_t i_osd1;
   uint32_t i_block[15];
   uint32_t i_generation;
   uint32_t i_file_acl;
   uint32_t i_dir_acl;
   uint32_t i_faddr;
   i_osd2 i_osd2;
} inode;

typedef struct {
   uint32_t inode;
   uint16_t rec_len;
   uint8_t name_len;
   uint8_t file_type;
   char name[];
} __attribute__((packed)) ext2_dir_entry;

#pragma pack(pop)

#endif