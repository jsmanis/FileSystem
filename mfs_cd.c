#define DECL_FILE
#include "mfs_cd.h"

int mfs_cd(char input_tokkens[BUFSIZE])
{
    // cd ..
    // cd .
    // cd /kati/kati2/kati3
    // cd kati/kati2
    if ( strlen(input_tokkens) == 1 && input_tokkens[0]== '.')
    {
      //do nothing
      return SUCCESS_MFS;
    }
    else if (strlen(input_tokkens) == 2 && input_tokkens[0] == '.' && input_tokkens[1] == '.')
    {
      if (updateWorkingDirectory_ParentOnly(mfs_filename) == FAIL_MFS)
        return FAIL_MFS;
      return SUCCESS_MFS;
    }
    else
    {
      if (updateWorkingDirectory(mfs_filename, input_tokkens) == FAIL_MFS)
        return FAIL_MFS;
      return SUCCESS_MFS;
    }
    return FAIL_MFS;
}
