#ifndef CONTSTANTS_H
#define CONTSTANTS_H

#define INPUTBUFFERSIZE 2048
#define DIRECT_POINTERS_NUM 12
#define DIRECTORY_BUFSIZE 256
#define BUFSIZE 64
#define NUMBER_COMMANDS 64

#define DEFAULT_BLOCKSIZE 1024
#define DEFAULT_FILENAME_SIZE 128
#define DEFAULT_MAX_FILESIZE 1024
#define DEFAULT_MAX_DIRECTORY_FILE_NUMBER 5
#define DIRECTORY_MFS "myfs/"

#define TYPE_FILE 0
#define TYPE_DIRECTORY 1
#define TYPE_LINK 2

enum {FAIL_MFS,SUCCESS_MFS};

typedef struct{
  unsigned int child_nodeid;
  char childname[BUFSIZE];
  unsigned int child_node_block; // used only by directory to avoid search
}child;

typedef struct{
  unsigned int direct_blocks[DIRECT_POINTERS_NUM+3];
  unsigned int last_chain_block; // used only by directory to avoid search
}Datastream;

typedef struct{
  unsigned int nodeid;
  char filename[BUFSIZE];
  unsigned int size;
  unsigned int type;
  unsigned int parent_nodeid[2]; //0 is parent_nodeid_block ,1 is parent_nodeid_id
  time_t creation_time;
  time_t access_time;
  time_t modification_time;
  Datastream data;
}MDS;

typedef struct{
  unsigned int block_size;
  unsigned int size_mds;
  unsigned int root_id;
  unsigned int number_of_blocks;
  unsigned int next_id;//next id that we give in MDS
  unsigned int write_mds_no; //block that we will write MDS
  unsigned int working_directory[2]; //0 store -> block_number,1 store ->MDS ID
}Superblock;

#endif
