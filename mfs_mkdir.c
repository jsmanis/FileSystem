#define DECL_FILE
#include "mfs_mkdir.h"

int mfs_mkdir(char input_tokkens[NUMBER_COMMANDS][BUFSIZE],int pos)
{
    int i = 0;
    if (pos < 2)
    {
      fprintf(stderr,"mfs_mkdir should have at least one argument");
      return FAIL_MFS;
    }
    int mfs_fd = open(mfs_filename,O_RDWR,0666);
    if (mfs_fd < 0)
    {
      perror("mfs_mkdir on opening file");
      close(mfs_fd);
      return FAIL_MFS;
    }

    // Superblock retrieved_node;
    // if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
    // {
    //   perror("Error reading superblock on mfs_mkdir");
    //   close(mfs_fd);
    //   return FAIL_MFS;
    // }

    int capacity = -1;

    MDS directory_mds;

    for (i=1;i<pos;i++)
    {
      Superblock retrieved_node;
      
      lseek(mfs_fd,0,SEEK_SET);
      if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
      {
        perror("Error reading superblock on mfs_mkdir");
        close(mfs_fd);
        return FAIL_MFS;
      }
      //First create MDS
      memset(&directory_mds,0,sizeof(MDS));
      directory_mds.nodeid = retrieved_node.next_id;
      strcpy(directory_mds.filename,input_tokkens[i]);
      directory_mds.size = 1; //to do
      directory_mds.type = TYPE_DIRECTORY;
      directory_mds.parent_nodeid[0] = retrieved_node.working_directory[0];
      directory_mds.parent_nodeid[1] = retrieved_node.working_directory[1];
      time_t mytimes = time(NULL);
      directory_mds.creation_time = mytimes;
      directory_mds.access_time = mytimes;
      directory_mds.modification_time = mytimes;
      Datastream mds_datastream;
      memset(mds_datastream.direct_blocks,0,DIRECT_POINTERS_NUM+3);
      mds_datastream.last_chain_block = 0;
      memcpy(&directory_mds.data,&mds_datastream,sizeof(Datastream));
      //printMDS(directory_mds);

      int ret_seek = lseek(mfs_fd,sizeof(Superblock)+(retrieved_node.write_mds_no-1)*retrieved_node.block_size,SEEK_SET);
      if (ret_seek < 0 )
      {
        perror("Seek error on mfs_mkdir");
        close(mfs_fd);
        return FAIL_MFS;
      }

      int remaining = -1;
      if ( read( mfs_fd , &remaining, sizeof(int) ) == -1 )
      {
          perror("Error on reading remaining int");
          close(mfs_fd);
          return FAIL_MFS;
      }

      if (remaining == 0) // allocate new block
      {
        retrieved_node.number_of_blocks++;
        retrieved_node.write_mds_no = retrieved_node.number_of_blocks;

        ret_seek = lseek(mfs_fd,sizeof(Superblock)+(retrieved_node.number_of_blocks-1)*retrieved_node.block_size,SEEK_SET);
        if (ret_seek < 0)
        {
          perror("Seek error on mfs_mkdir");
          close(mfs_fd);
          return FAIL_MFS;
        }

        capacity = (retrieved_node.block_size-sizeof(int))/sizeof(MDS) - 1 ;
        if ( write( mfs_fd , &capacity , sizeof(int) ) == -1 )
        {
          perror("Error on writing capacity");
          close(mfs_fd);
          return FAIL_MFS;
        }

        if ( write( mfs_fd , &directory_mds , sizeof(MDS) ) == -1 )
        {
          perror("Error on writing capacity");
          close(mfs_fd);
          return FAIL_MFS;
        }
      }
      else
      {
        //fits to the current block
        int move_blocks = (retrieved_node.block_size -sizeof(int))/sizeof(MDS) - remaining ;
        remaining--;
        ret_seek = lseek(mfs_fd,sizeof(Superblock)+(retrieved_node.write_mds_no-1)*retrieved_node.block_size+sizeof(int)
                                +move_blocks*sizeof(MDS),SEEK_SET);
        if (ret_seek < 0)
        {
          perror("Seek error on mfs_mkdir");
          close(mfs_fd);
          return FAIL_MFS;
        }

        if ( write( mfs_fd , &directory_mds , sizeof(MDS) ) == -1 )
        {
          perror("Error on writing remaining");
          close(mfs_fd);
          return FAIL_MFS;
        }
        ret_seek = lseek(mfs_fd,sizeof(Superblock)+(retrieved_node.write_mds_no-1)*retrieved_node.block_size,SEEK_SET);
        if (ret_seek < 0)
        {
          perror("Seek error on mfs_mkdir");
          close(mfs_fd);
          return FAIL_MFS;
        }
        if ( write( mfs_fd , &remaining , sizeof(int) ) == -1 )
        {
          perror("Error on writing remaining");
          close(mfs_fd);
          return FAIL_MFS;
        }
      }
      lseek(mfs_fd,0,SEEK_SET);
      if ( write( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
      {
        perror("Error on writing superblock");
        close(mfs_fd);
        return FAIL_MFS;
      }
      printf("num of blocks (mkdir) is : %d\n", retrieved_node.number_of_blocks);
      printf("child id :%d child name :%s child block :%d parent block :%d parent id :%d\n", directory_mds.nodeid,directory_mds.filename,retrieved_node.write_mds_no, retrieved_node.working_directory[0], retrieved_node.working_directory[1]);
      updateParentDatastream(mfs_fd,directory_mds.nodeid,directory_mds.filename,retrieved_node.write_mds_no, retrieved_node.working_directory[0], retrieved_node.working_directory[1]);
      retrieved_node.next_id++;
      updateSuperblockNextId(mfs_fd);
      updateParentMDSsize ( mfs_fd, retrieved_node.block_size, retrieved_node.working_directory[0], retrieved_node.working_directory[1], 1);
      printf("directory:%s successfully created.\n",input_tokkens[i]);
    }
    // printMFS(mfs_fd);
    close(mfs_fd);
    return SUCCESS_MFS;
}



    //first go to working block
    // MDS searchstruct;
    //
    // lseek(mfs_fd,sizeof(Superblock)+(retrieved_node.working_directory[0]-1)*retrieved_node.block_size+sizeof(int),SEEK_SET);
    // while(1)
    // {
    //   if ( read( mfs_fd , &searchstruct , sizeof(MDS) ) == -1 )
    //   {
    //     perror("Searching on mfs_mkdir");
    //     return FAIL_MFS;
    //   }
    //   if (searchstruct.nodeid == retrieved_node.working_directory[1])
    //   {
    //     break;
    //   }
    // }
    //on searchstruct we have found the parent node id
