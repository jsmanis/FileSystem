#define DECL_FILE
#include "mfs_pwd.h"

int mfs_pwd(void)
{
  int mfs_fd = open(mfs_filename,O_RDWR,0666);
  if (mfs_fd < 0)
  {
    perror("mfs_mkdir on opening file");
    return FAIL_MFS;
  }

  Superblock retrieved_node;
  if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
  {
    perror("Error reading superblock on mfs_import");
    return FAIL_MFS;
  }
  MDS directory_struct;

  directory_struct = search(mfs_fd,retrieved_node.block_size,retrieved_node.working_directory[0],retrieved_node.working_directory[1]);
  printf("\nCurrent Directory | nodeid: %d,filename: %s parent: %u time_access:%ld,time_modification:%ld |\n\n",directory_struct.nodeid,directory_struct.filename,directory_struct.parent_nodeid[0],directory_struct.access_time,directory_struct.modification_time);
}
