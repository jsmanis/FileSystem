#ifndef MFS_IMPORT_H
#define MFS_IMPORT_H
#include "utilities.h"
int mfs_import(char input_tokkens[NUMBER_COMMANDS][BUFSIZE],int pos);
int findDirectoryByName(int mfs_fd,char *path,int * block_position,int *mds_id,int isFile);
#endif
