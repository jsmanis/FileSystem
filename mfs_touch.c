#define DECL_FILE
#include "mfs_touch.h"

int mfs_touch(char input_tokkens[NUMBER_COMMANDS][BUFSIZE],int pos)
{
  if (pos < 2)
  {
    fprintf(stderr,"Few arguments given\n");
    return FAIL_MFS;
  }

  int i = 0;
  int a_flag = 0;
  int m_flag = 0;

  for (i=1;i<pos;i++)
  {
      if (!strcmp("-a",input_tokkens[i]))
      {
        a_flag = 1;
      }
      else if (!strcmp("-m",input_tokkens[i]))
      {
        m_flag = 1;
      }
  }

  if ( (a_flag==0 && m_flag == 0) || (a_flag==1 && m_flag == 1) ) //both
  {
    for (i=1;i<pos;i++)
    {
      if ( (strcmp(input_tokkens[i],"-a") ) && (strcmp(input_tokkens[i],"-m")) )
      {
        updateTimestamp(mfs_filename,input_tokkens[i],BOTH_FLAGS);
      }
    }
  }
  else if (a_flag == 1)
  {
    for (i=1;i<pos;i++)
    {
      if ( (strcmp(input_tokkens[i],"-a") ))
      {
        updateTimestamp(mfs_filename,input_tokkens[i],A_FLAG);
      }
    }
  }
  else
  {
    for ( i = 1 ; i < pos ; i++)
    {
      if ((strcmp(input_tokkens[i],"-m")) )
      {
        updateTimestamp(mfs_filename,input_tokkens[i],M_FLAG);
      }
    }
  }

}

int updateTimestamp(char* mfs_myfilename,char* filename,int updateflag)
{
  int mfs_fd = open(mfs_myfilename,O_RDWR,0666);
  if (mfs_fd < 0)
  {
    perror("updateTimestamp on opening file");
    close(mfs_fd);
    return FAIL_MFS;
  }

  Superblock retrieved_node;
  if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
  {
    perror("Error reading superblock on updateWorkingDirectory_ParentOnly");
    close(mfs_fd);
    return FAIL_MFS;
  }

  MDS parent_node ;
  MDS name_mds;
  child child_node;
  parent_node = search(mfs_fd,retrieved_node.block_size,retrieved_node.working_directory[0],retrieved_node.working_directory[1]);
  int flag = 1;
  child_node = name_search(mfs_fd , retrieved_node.block_size , parent_node.data.direct_blocks[0], filename ,&flag);
  if(flag == 0)
  {
    printf("filename '%s' does not exists in location\n", filename);
    close(mfs_fd);
    return FAIL_MFS;
  }
  name_mds = search(mfs_fd,retrieved_node.block_size,child_node.child_node_block,child_node.child_nodeid);
  size_t new_times = time(NULL);

  if (updateflag == BOTH_FLAGS)
  {
    name_mds.modification_time = new_times;
    name_mds.access_time = new_times;
  }
  else if (updateflag == A_FLAG)
  {
    name_mds.access_time = new_times;
  }
  else
  {
    name_mds.modification_time = new_times;
  }
  printf("Name is %s \n",name_mds.filename);
  int retseek = lseek(mfs_fd,-sizeof(MDS),SEEK_CUR);
  if (retseek < 0)
  {
    perror("Error on seek updateTimestamp");
    close(mfs_fd);
    return FAIL_MFS;
  }

  if ( write( mfs_fd , &name_mds , sizeof(MDS) ) == -1)
  {
    perror("writing on updating updateTimestamp");
    close(mfs_fd);
    return FAIL_MFS;
  }
  return SUCCESS_MFS;
}
