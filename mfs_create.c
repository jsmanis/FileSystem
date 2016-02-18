#define DECL_FILE
#include "mfs_create.h"

int mfs_create(char input_tokkens[NUMBER_COMMANDS][BUFSIZE],int pos)
{
    if (pos < 1)
    {
      fprintf(stderr,"Few arguments given\n");
      return FAIL_MFS;
    }
    int block_size = DEFAULT_BLOCKSIZE;
    int filename_size = DEFAULT_FILENAME_SIZE;
    int max_file_size = DEFAULT_MAX_FILESIZE;
    int max_directory_file_number = DEFAULT_MAX_DIRECTORY_FILE_NUMBER;

    int i = 0;

    for (i=1;i<pos-1;i+=2)
    {
        if (!strcmp("-bs",input_tokkens[i]))
        {
          block_size = atoi(input_tokkens[i+1]);
        }
        else if (!strcmp("-fns",input_tokkens[i]))
        {
          filename_size = atoi(input_tokkens[i+1]);
        }
        else if (!strcmp("-mfs",input_tokkens[i]))
        {
          max_file_size = atoi(input_tokkens[i+1]);
        }
        else if (!strcmp("-mdfn",input_tokkens[i]))
        {
          max_directory_file_number = atoi(input_tokkens[i+1]);
        }
        else{
          fprintf(stderr,"Invalid arguments given\n");
          return FAIL_MFS;
        }
    }

    char filename[BUFSIZE*2];
    strcpy(filename,DIRECTORY_MFS);
    strcat(filename,input_tokkens[pos-1]);
    strcat(filename,".mfs");
    strcpy(mfs_filename,filename);
    int flag_low_capacity = 0;
    int mfs_fd = open(filename,O_RDWR | O_CREAT, 0666);
    if (mfs_fd < 0)
    {
      perror("opening mfs_create");
      return FAIL_MFS;
    }
    Superblock superblock;
    superblock.block_size = block_size;
    superblock.size_mds = sizeof(MDS);
    superblock.root_id = 1;
    superblock.next_id = 1;
    superblock.number_of_blocks = 1;
    superblock.working_directory[0] = 1 ; //first block
    superblock.working_directory[1] = 1 ; //first mds
    int capacity = (block_size-sizeof(int))/sizeof(MDS) - 1 ;
    if (capacity < 0 )
    {
      fprintf(stderr,"Block size is too small!\n");
      return FAIL_MFS;
    }

    superblock.write_mds_no = 1;
    if (capacity == 0 )
    {
      superblock.write_mds_no = 2;
      superblock.number_of_blocks = 2;
      flag_low_capacity = 1;
    }

    if ( write( mfs_fd , &superblock , sizeof(Superblock) ) == -1 )
    {
      perror("writing superblock");
      return FAIL_MFS;
    }

    MDS root_mds;


    if ( write( mfs_fd, &capacity, sizeof(int) ) == - 1  ) //remaining blocks to write
    {
      perror("writing capacity");
      return FAIL_MFS;
    }

    root_mds.nodeid = superblock.next_id;
    strcpy(root_mds.filename,"/");
    root_mds.size = 1; //to do
    root_mds.type = TYPE_DIRECTORY;
    root_mds.parent_nodeid[0] = root_mds.nodeid; //always will be one
    root_mds.parent_nodeid[1] = root_mds.nodeid;
    time_t mytimes = time(NULL);
    root_mds.creation_time = mytimes;
    root_mds.access_time = mytimes;
    root_mds.modification_time = mytimes;

    Datastream root_datastream;
    memset(root_datastream.direct_blocks,0,DIRECT_POINTERS_NUM+3);
    root_datastream.last_chain_block = 0;
    memcpy(&root_mds.data,&root_datastream,sizeof(Datastream));

    if ( write( mfs_fd , &root_mds , sizeof(MDS) ) == -1 )
    {
      perror("writing root mds");
      return FAIL_MFS;
    }

    if (flag_low_capacity) // too small block_size.Fits only one MDS per block
    {
      int new_capacity = 1;
      int ret_seek = lseek(mfs_fd,sizeof(Superblock)+block_size,SEEK_SET);
      if (ret_seek < 0)
      {
        perror("Seek error");
        return FAIL_MFS;
      }
      if ( write( mfs_fd , &new_capacity , sizeof(int) ) == -1)
      {
        perror("Writing new capacity");
        return FAIL_MFS;
      }
    }

    /*update next id of superblock*/
    updateSuperblockNextId(mfs_fd);

    if ( close( mfs_fd ) == -1 )
    {
      perror("closing file");
      return FAIL_MFS;
    }

    // int fd = open(filename,O_RDONLY, 0666);
    // Superblock retrieved_node;
    // if ( read( fd , &retrieved_node , sizeof(Superblock) ) == -1 )
    // {
    //   perror("read");
    //   return FAIL_MFS;
    // }
    // printSuperblock(retrieved_node);
    // if ( close( fd ) == -1 )
    // {
    //   perror("closing file");
    //   return FAIL_MFS;
    // }
    return SUCCESS_MFS;
}
