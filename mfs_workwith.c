#define DECL_FILE
#include "mfs_workwith.h"

void mfs_workwith(char input_tokkens[BUFSIZE])
{
  char mybuf[BUFSIZE];
  strcpy(mybuf,DIRECTORY_MFS);
  strcat(mybuf,"/");
  strcat(mybuf,input_tokkens);
  strcpy(mfs_filename,mybuf);
}
