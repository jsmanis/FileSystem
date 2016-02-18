#define DECL_FILE
#ifndef MFS_TOUCH_H
#define MFS_TOUCH_H
#include "utilities.h"

enum {M_FLAG,A_FLAG,BOTH_FLAGS};
int mfs_touch(char input_tokkens[NUMBER_COMMANDS][BUFSIZE],int pos);
#endif
