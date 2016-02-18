#include "utilities.h"

int main (void)
{
  char inputbuffer[INPUTBUFFERSIZE];
  //create prompt
  printf("$ ");
  while(fgets(inputbuffer,INPUTBUFFERSIZE,stdin) != NULL)
  {
    inputbuffer[strlen(inputbuffer)-1] ='\0'; //remove new line
    char full_path[INPUTBUFFERSIZE];
    strcpy(full_path,mfs_findfunctions(inputbuffer));
    printf("%s $ ",full_path);
  }

  return 0;
}
