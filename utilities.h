#ifndef UTILITIES_H
#define UTILITIES_H
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "constants.h"
#include "mfs_create.h"
#include "mfs_import.h"
#include "mfs_cd.h"
#include "mfs_workwith.h"
#include "mfs_mkdir.h"
#include "mfs_functions.h"
#include "mfs_pwd.h"
#include "mfs_ls.h"
#include "mfs_touch.h"
#include "mfs_export.h"

void updateSuperblockNextId(int mfs_fd);
int updateParentDatastream(int mfs_fd,int dir_nodeid, char *dir_name, int child_MDSblock, int parentBlock_number, int parent_nodeid);
child name_search(int mfs_fd , int block_size , int block_number ,char *name ,int *flag );
void printSuperblock(Superblock superblock);
int printMDS(int mfs_fd,MDS printable_mds,int block_size);
int printMFS(int mfs_fd);
char* printFullpath(char *myfilename);
int updateParentMDSsize ( int mfs_fd, int block_size, int parentBlock_number, int parentNode_id, int sizeToIncreaze);
MDS search(int mfs_fd,int block_size,int block_number,int node_id);
#ifdef  DECL_FILE
char mfs_filename[BUFSIZE];
#else
extern char mfs_filename[BUFSIZE];
#endif

#endif
