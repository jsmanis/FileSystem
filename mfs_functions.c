#define DECL_FILE
#include "utilities.h"
char* mfs_findfunctions(char* command)
{
  char input_tokkens[NUMBER_COMMANDS][BUFSIZE];
  int pos = 0;
  char *tokken = strtok(command," ");
  while( tokken != NULL )
  {
      strcpy(input_tokkens[pos],tokken);
      pos++;
      tokken = strtok(NULL," ");
  }

  if (pos == 0)
  {
    fprintf(stderr,"No command given\n");
    return "";
  }

  if ( !strcmp( "mfs_workwith" , input_tokkens[0]) )
  {
    if (pos > 2 )
    {
      fprintf(stderr,"Too many arguments for mfs_workwith");
      return "";
    }
    mfs_workwith(input_tokkens[1]);
    if (printFullpath(mfs_filename) != NULL)
    {
      return printFullpath(mfs_filename);
    }
    return "";
  }
  else if (!strcmp( "mfs_ls" , input_tokkens[0]) )
  {
    if ( mfs_ls(input_tokkens,pos) == FAIL_MFS)
    {
      return printFullpath(mfs_filename);
    }
    return printFullpath(mfs_filename);
  }
  else if (!strcmp( "mfs_cd" , input_tokkens[0]) )
  {
    if (pos > 2 )
    {
      fprintf(stderr,"Too many arguments for mfs_cd");
      return printFullpath(mfs_filename);
    }
    if( mfs_cd(input_tokkens[1]) == FAIL_MFS )
    {
      return printFullpath(mfs_filename);
    }
    return printFullpath(mfs_filename);
  }
  else if (!strcmp( "mfs_pwd" , input_tokkens[0]) )
  {
    if( mfs_pwd() == FAIL_MFS )
    {
      fprintf(stderr,"Error on mfs_pwd\n");
      return "";
    }
    return printFullpath(mfs_filename);
  }
  else if (!strcmp( "mfs_cp" , input_tokkens[0]) )
  {

  }
  else if (!strcmp( "mfs_mv" , input_tokkens[0]) )
  {

  }
  else if (!strcmp( "mfs_rm" , input_tokkens[0]) )
  {

  }
  else if (!strcmp( "mfs_mkdir" , input_tokkens[0]) )
  {
    if ( mfs_mkdir(input_tokkens,pos) == FAIL_MFS)
    {
      fprintf(stderr,"Error on mfs_mkdir\n");
      return printFullpath(mfs_filename);
    }
    return printFullpath(mfs_filename);
  }
  else if (!strcmp( "mfs_touch" , input_tokkens[0]) )
  {
    if ( mfs_touch(input_tokkens,pos) == FAIL_MFS)
    {
      return printFullpath(mfs_filename);
    }
    return printFullpath(mfs_filename);
  }
  else if (!strcmp( "mfs_import" , input_tokkens[0]) )
  {
    if ( mfs_import(input_tokkens,pos) == FAIL_MFS)
    {
      return printFullpath(mfs_filename);
    }
    return printFullpath(mfs_filename);
  }
  else if (!strcmp( "mfs_export" , input_tokkens[0]) )
  {
    if ( mfs_export(input_tokkens,pos) == FAIL_MFS)
    {
      return printFullpath(mfs_filename);
    }
    return printFullpath(mfs_filename);
  }
  else if (!strcmp( "mfs_cat" , input_tokkens[0]) )
  {

  }

  /*start of mfs_create*/
  else if (!strcmp( "mfs_create" , input_tokkens[0]) )
  {
        if ( mfs_create(input_tokkens,pos) == FAIL_MFS)
        {
          fprintf(stderr,"Error on mfs_create\n");
          return printFullpath(mfs_filename);
        }
        return printFullpath(mfs_filename);
  }
  else{
    fprintf(stderr,"Command not found.\n");
    return printFullpath(mfs_filename);
  }
  /*end of mfs_create*/

}
