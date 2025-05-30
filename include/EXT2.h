#include <stdint.h>

//Estruturas de dados do sistesma de arquivos EXT2

//***SUPERBLOCK:

#pragma pack(push,1)

typedef struct {
   uint32_t s_nodes_count;
   uint32_t s_blocks_count;
   uint32_t s_r_blocks_count;
   uint32_t s_free_blocks_count;
   uint32_t s_first_data_count;
   uint32_t s_log_block_size;
   uint32_t s_log_frag_size;
   uint32_t s_blocks_per_group;
   uint32_t s_frags_per_group;
   uint32_t s_inodes_per_group;
   uint32_t s_mtime;
   uint32_t s_wtime;
   uint16_t s_mnt_count;
   uint16_t s_max_mnt_count;
   uint16_t s_magic;
   uint16_t s_state;
   uint16_t s_errors;
   uint16_t s_minor_rev_level;
   uint32_t s_lastcheck;
   uint32_t s_checkinterval;
   uint32_t s_creator_os;
   uint32_t s_rev_level;
   uint16_t s_def_resuid;
   uint16_t s_def_resgid;
   uint32_t s_first_ino;
   uint16_t s_inode_size;
   uint16_t s_block_group_nr;
   uint32_t s_feature_compat;
   uint32_t s_feature_incompat;
   uint32_t s_feature_ro_compat;
   char s_uuid[16];
   char s_volume_name[16];
   char s_last_mounted[64];
   uint32_t s_algo_bitmpap;
   char s_prealloc_blocks;
   char s_prealloc_dir_blocks;
   uint16_t s_aligment; //UNUSED
   char s_journal_uuid[16];
   uint32_t s_journal_inum;
   uint32_t s_journal_dev;
   uint32_t s_last_orphan;
   uint32_t s_hash_seed[4];
   char s_def_hash_version;
   char s_padding[3];
   uint32_t s_default_mount_options;
   uint32_t s_first_meta_bg;
   char s_unused[760];

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
   uint32_t i_block[15];
   uint32_t i_generation;
   uint32_t i_file_acl;
   uint32_t i_dir_acl;
   uint32_t i_faddr;
   i_osd2 i_osd2;
} inode;

#pragma pack(pop)
