#define DECL_FILE
#ifndef MFS_LS_H
#define MFS_LS_H
#include "utilities.h"
int mfs_ls(char input_tokkens[NUMBER_COMMANDS][BUFSIZE],int pos);
int recursivePrintDirectory(int mfs_fd , int block_size , int block_number , int lExists);
#endif
