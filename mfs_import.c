#define DECL_FILE
#include "mfs_import.h"

int mfs_import(char input_tokkens[NUMBER_COMMANDS][BUFSIZE],int pos)
{
  int i = 0;
  if (pos < 3)
  {
    fprintf(stderr,"Mfs_import should have at least 2 arguments");
    return FAIL_MFS;
  }
  char directory[BUFSIZE];
  strcpy(directory,input_tokkens[pos-1]);

  int mfs_fd = open(mfs_filename,O_RDWR,0666);
  if (mfs_fd < 0)
  {
    perror("mfs_import on opening file");
    return FAIL_MFS;
  }
  int block_position = -1;
  int mds_id = -1;
  if ( findDirectoryByName(mfs_fd,directory,&block_position,&mds_id,0) == FAIL_MFS )
  {
    return FAIL_MFS;
  }
  printf("block pos :%d parent id %d\n", block_position, mds_id);

  int retseek = lseek(mfs_fd,0,SEEK_SET);
  if ( retseek < 0 )
  {
    perror("Seek failed on mfs_import");
    return FAIL_MFS;
  }

  Superblock retrieved_node;
  if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
  {
    perror("Error reading superblock on mfs_import");
    close(mfs_fd);
    return FAIL_MFS;
  }

  // printf("Found in position %d and mds_id is %d \n",block_position,mds_id);

  for (i=1;i<pos-1;i++) // through every file
  {
    FILE* file_ptr = fopen(input_tokkens[i],"rb");
    if ( file_ptr == NULL )
    {
      fprintf(stderr,"File %s does not exists \n",input_tokkens[i]);
      continue;
    }

    long lSize;
    fseek(file_ptr,0,SEEK_END);
    lSize = ftell(file_ptr);
    rewind(file_ptr);
    long maxsize_file = DIRECT_POINTERS_NUM*retrieved_node.block_size + (retrieved_node.block_size/sizeof(int) - 1) * retrieved_node.block_size + (retrieved_node.block_size/sizeof(int) - 1)*(retrieved_node.block_size/sizeof(int) - 1) * retrieved_node.block_size;
    if (maxsize_file < lSize)
    {
      fprintf(stderr,"File %s is too big\n",input_tokkens[i]);
      continue;
    }


    int capacity = -1;

    //First create MDS
    MDS file_mds;
    //
    file_mds.nodeid = retrieved_node.next_id;
    strcpy(file_mds.filename,input_tokkens[i]);
    file_mds.size = lSize; //to do
    file_mds.type = TYPE_FILE;
    file_mds.parent_nodeid[0] = block_position;
    file_mds.parent_nodeid[1] = mds_id;

    time_t mytimes = time(NULL);
    file_mds.creation_time = mytimes;
    file_mds.access_time = mytimes;
    file_mds.modification_time = mytimes;
    Datastream mds_datastream;
    memset(mds_datastream.direct_blocks,0,(DIRECT_POINTERS_NUM+3)*sizeof(int));
    mds_datastream.last_chain_block = 0;
    memcpy(&file_mds.data,&mds_datastream,sizeof(Datastream));

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

    if (remaining == 0) // allocate new block for mds
    {
      retrieved_node.number_of_blocks++;
      retrieved_node.write_mds_no = retrieved_node.number_of_blocks;

      ret_seek = lseek(mfs_fd,sizeof(Superblock)+(retrieved_node.number_of_blocks-1)*retrieved_node.block_size,SEEK_SET);
      if (ret_seek < 0)
      {
        perror("Seek error on mfs_import");
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

      if ( write( mfs_fd , &file_mds , sizeof(MDS) ) == -1 )
      {
        perror("Error on writing capacity");
        close(mfs_fd);
        return FAIL_MFS;
      }
    }
    else
    {
      //mds fits to the current block
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

      if ( write( mfs_fd , &file_mds , sizeof(MDS) ) == -1 )
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
    // 
    updateParentDatastream(mfs_fd,file_mds.nodeid,file_mds.filename,retrieved_node.write_mds_no, block_position, mds_id);
    retrieved_node.next_id++;
    updateSuperblockNextId(mfs_fd);
    updateParentMDSsize ( mfs_fd, retrieved_node.block_size, block_position, mds_id, lSize);
    printf("File mds :%s successfully created.\n",input_tokkens[i]);
    lseek(mfs_fd,0,SEEK_SET);
    if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
    {
      perror("read");
      return FAIL_MFS;
    }
    //Make i-node blocks

    int j = 0;
    int flag_small_file = 0;

    int blob_data_num = (int) lSize/retrieved_node.block_size;
    blob_data_num++;
    fprintf(stderr,"create %d blocks\n",blob_data_num);
    // sleep(10000);
    for (j = 0 ; j < DIRECT_POINTERS_NUM ; j++ ) //first read direct pointers
    {
      //at least one block will be created
      char binarybuffer[retrieved_node.block_size];
      memset(binarybuffer,0,retrieved_node.block_size);

      retrieved_node.number_of_blocks++;
      file_mds.data.direct_blocks[j] = retrieved_node.number_of_blocks;
      int seek_file;
      seek_file = lseek(mfs_fd, sizeof(Superblock) + (retrieved_node.number_of_blocks-1)*retrieved_node.block_size,SEEK_SET);
      if ( seek_file < 0 )
      {
        perror("Seek failed on writing file");
        return FAIL_MFS;
      }
      int myread = fread(binarybuffer,1,retrieved_node.block_size,file_ptr);
      // fprintf(stderr,"I will try to write %d with %s \n",myread,binarybuffer);
      if ( write( mfs_fd , &binarybuffer , myread ) == -1 )
      {
        perror("Error on writing to direct block");
        close(mfs_fd);
        return FAIL_MFS;
      }

      blob_data_num --;
      if ( blob_data_num == 0 ) //end of file
      {
        flag_small_file = 1;
        break;
      }

    }
    int flag_continue_second_level = 1 ;
    int flag_continue_third_level = 0;
    if (flag_small_file == 0) //first_inode_level
    {
      printf("Goes in first level\n");
      retrieved_node.number_of_blocks++;
      file_mds.data.direct_blocks[DIRECT_POINTERS_NUM+1] = retrieved_node.number_of_blocks;

      int seek_file;
      seek_file = lseek(mfs_fd, sizeof(Superblock) + (retrieved_node.number_of_blocks-1)*retrieved_node.block_size,SEEK_SET);
      if ( seek_file < 0 )
      {
        perror("Seek failed on first_inode_level file");
        return FAIL_MFS;
      }

      int capacity = 0;


      while( capacity <= (retrieved_node.block_size - sizeof(int))/sizeof(int) ) //first inode loop
      {
        seek_file = lseek(mfs_fd, sizeof(Superblock) + (file_mds.data.direct_blocks[DIRECT_POINTERS_NUM+1] - 1) * retrieved_node.block_size + sizeof(int)+capacity*sizeof(int),SEEK_SET);
        if ( seek_file < 0 )
        {
          perror("Seek failed on first_inode_level file");
          return FAIL_MFS;
        }

        retrieved_node.number_of_blocks++;
        int new_block_num = retrieved_node.number_of_blocks;
        // printf("new block num %d \n",new_block_num);
        if ( write(mfs_fd,&new_block_num,sizeof(int)) == -1 )
        {
          perror("Error on write to first_inode_level");
          close(mfs_fd);
          return FAIL_MFS;
        }

        char binarybuffer[retrieved_node.block_size];
        memset(binarybuffer,0,retrieved_node.block_size);

        lseek(mfs_fd,sizeof(Superblock)+(retrieved_node.number_of_blocks-1) * retrieved_node.block_size,SEEK_SET);

        int my_read = fread(binarybuffer,1,retrieved_node.block_size,file_ptr);
        // fprintf(stderr,"I will try to write %d with %s \n",myread,binarybuffer);
        if ( write( mfs_fd , &binarybuffer , my_read ) == -1 )
        {
          perror("Error on writing to first_inode_level block");
          close(mfs_fd);
          return FAIL_MFS;
        }

        // fread(binarybuffer,retrieved_node.block_size,1,file_ptr);
        // if ( write( mfs_fd , &binarybuffer , retrieved_node.block_size ) == -1 )
        // {
        //   perror("Error on writing to direct block");
        //   close(mfs_fd);
        //   return FAIL_MFS;
        // }
        blob_data_num --;
        capacity++;
        if ( blob_data_num == 0 )
        {

          break;
        }

      }
      lseek(mfs_fd, sizeof(Superblock) + (file_mds.data.direct_blocks[DIRECT_POINTERS_NUM+1] - 1) * retrieved_node.block_size,SEEK_SET);
      if ( write(mfs_fd,&capacity,sizeof(int)) == -1 )
      {
        perror("Error on writing capacity first_inode_level");
        close(mfs_fd);
        return FAIL_MFS;
      }

      if ( blob_data_num >0 )
      {
        flag_continue_second_level = 0;
      }
      printf("Goes out from first level\n");
    }



      if (flag_continue_second_level == 0) //start inode level-two
      {
        printf("Goes in second level\n");
        retrieved_node.number_of_blocks++;
        file_mds.data.direct_blocks[DIRECT_POINTERS_NUM+2] = retrieved_node.number_of_blocks;

        int seek_file;
        seek_file = lseek(mfs_fd, sizeof(Superblock) + (retrieved_node.number_of_blocks-1)*retrieved_node.block_size,SEEK_SET);
        if ( seek_file < 0 )
        {
          perror("Seek failed on first_inode_level file");
          return FAIL_MFS;
        }

        int capacity = 0;
        int outer_capacity = 0;
        int outer_block_number = 0;

        while( outer_capacity <= (retrieved_node.block_size - sizeof(int))/sizeof(int) )
        {
            retrieved_node.number_of_blocks++;
            outer_block_number = retrieved_node.number_of_blocks;

            int outerseek= lseek(mfs_fd, sizeof(Superblock) + (file_mds.data.direct_blocks[DIRECT_POINTERS_NUM+2] - 1) * retrieved_node.block_size + sizeof(int)+outer_capacity*sizeof(int),SEEK_SET);
            capacity = 0;
            if ( outerseek < 0 )
            {
              perror("Seek failed on second_inode_level file");
              return FAIL_MFS;
            }

            if ( write(mfs_fd,&outer_block_number,sizeof(int)) == -1 )
            {
              perror("Error on writing outer_block_number");
              close(mfs_fd);
              return FAIL_MFS;
            }

            printf("Outer block num is %d \n",outer_block_number);
            while( capacity <= (retrieved_node.block_size - sizeof(int))/sizeof(int) )
            {
              seek_file = lseek(mfs_fd, sizeof(Superblock) + (outer_block_number - 1) * retrieved_node.block_size + sizeof(int)+capacity*sizeof(int),SEEK_SET);
              if ( seek_file < 0 )
              {
                perror("Seek failed on first_inode_level file");
                return FAIL_MFS;
              }

              retrieved_node.number_of_blocks++;
              int new_block_num = retrieved_node.number_of_blocks;
              // printf("**new_block_num is %d \n",new_block_num);
              if ( write(mfs_fd,&new_block_num,sizeof(int)) == -1 )
              {
                perror("Error on write to first_inode_level");
                close(mfs_fd);
                return FAIL_MFS;
              }

              char binarybuffer[retrieved_node.block_size];
              memset(binarybuffer,0,retrieved_node.block_size);

              lseek(mfs_fd,sizeof(Superblock)+(retrieved_node.number_of_blocks-1) * retrieved_node.block_size,SEEK_SET);

              int my__read = fread(binarybuffer,1,retrieved_node.block_size,file_ptr);
              // printf("My read is %d \n",my__read);
              // fprintf(stderr,"I will try to write %d with %s \n",myread,binarybuffer);
              if ( write( mfs_fd , &binarybuffer , my__read ) == -1 )
              {
                perror("Error on writing to second_inode_level block");
                close(mfs_fd);
                return FAIL_MFS;
              }

              // fread(binarybuffer,retrieved_node.block_size,1,file_ptr);
              // if ( write( mfs_fd , &binarybuffer , retrieved_node.block_size ) == -1 )
              // {
              //   perror("Error on writing to direct block");
              //   close(mfs_fd);
              //   return FAIL_MFS;
              // }
              blob_data_num --;
              capacity++;
              if ( blob_data_num <= 0 )
              {
                flag_continue_third_level = 1;
                break;
              }
            }

          outer_capacity++;
          printf("capacity is %d\n",capacity);
          lseek(mfs_fd,sizeof(Superblock) + (outer_block_number  - 1)*retrieved_node.block_size,SEEK_SET);
          if ( write(mfs_fd,&capacity,sizeof(int)) == -1 )
          {
            perror("Error on capacity to second_inode_level");
            close(mfs_fd);
            return FAIL_MFS;
          }

        if (flag_continue_third_level == 1)
          break;
      }



      seek_file = lseek(mfs_fd, sizeof(Superblock) + (file_mds.data.direct_blocks[DIRECT_POINTERS_NUM+2] - 1) * retrieved_node.block_size,SEEK_SET);
      if ( seek_file < 0 )
      {
        perror("Seek failed on second_inode_level file");
        return FAIL_MFS;
      }

      if ( write(mfs_fd,&outer_capacity,sizeof(int)) == -1 )
      {
        perror("Error on capacity to second_inode_level");
        close(mfs_fd);
        return FAIL_MFS;
      }

      printf("Goes out second level\n");
    }

    int move_start = lseek(mfs_fd,0,SEEK_SET);
    if ( move_start < 0 )
    {
      perror("Error on seek to start on superblock");
      return FAIL_MFS;
    }
    if ( write(mfs_fd,&retrieved_node,sizeof(Superblock)) == -1 )
    {
      perror("Error on writing to Superblock");
      close(mfs_fd);
      return FAIL_MFS;
    }

    lseek(mfs_fd,sizeof(Superblock)+(retrieved_node.write_mds_no-1)*retrieved_node.block_size,SEEK_SET);
    int remaining_cap = -1 ;

    if ( read(mfs_fd , &remaining_cap , sizeof(int) ) == -1 )
    {
      perror("read");
      return FAIL_MFS;
    }

    int lseek_places = ((retrieved_node.block_size - sizeof(int))/sizeof(MDS)) - remaining_cap; //BUG!!!!KAI AN PREPEI NA FTIAKSOUME KAINOURIO ?
    printf("Lseek places %d\n",lseek_places);
    lseek(mfs_fd,(lseek_places-1)*sizeof(MDS),SEEK_CUR);
    if ( write( mfs_fd , &file_mds , sizeof(MDS) ) == -1 )
    {
      perror("Error on writing mds");
      close(mfs_fd);
      return FAIL_MFS;
    }

    fclose(file_ptr);

  }

  close(mfs_fd);
}


int findDirectoryByName(int mfs_fd,char *path,int * block_position,int *mds_id,int isFile)
{
  char *tokken;
  int flag = 1;
  int retseek = lseek(mfs_fd,0,SEEK_SET);

  Superblock retrieved_node;
  if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
  {
    perror("Error reading superblock on findDirectoryByName");
    return FAIL_MFS;
  }

  MDS directory_struct;
  child child_node;

  if (path[0] == '/') // search absolute path
  {
    if( strlen(path) == 1)
    {
      *block_position = 1;
      *mds_id = 1;
      if (lseek(mfs_fd,0,SEEK_SET) < 0 )
      {
        perror("Error on updating superblock findDirectoryByName");
        return FAIL_MFS;
      }
      return SUCCESS_MFS;
    }

    directory_struct = search(mfs_fd , retrieved_node.block_size ,1,retrieved_node.root_id);
    tokken = strtok(path,"/");
    char directory[BUFSIZE];
    while( tokken != NULL )
    {
        child_node = name_search(mfs_fd , retrieved_node.block_size , directory_struct.data.direct_blocks[0], tokken, &flag);
        if( flag == 0 )
        {
          printf("directory '%s' does not exists in location '%s'\n", tokken, path);
          return FAIL_MFS;
        }
        // printf("Tokken (%s)\n", tokken );//, child_node.child_node_block);
        directory_struct = search(mfs_fd , retrieved_node.block_size , child_node.child_node_block,child_node.child_nodeid);
        if (!isFile)
        {
          if(directory_struct.type != TYPE_DIRECTORY)
          {
            printf("'%s' isn't a directory to open\n", directory_struct.filename);
            return FAIL_MFS;
          }
        }

        tokken = strtok(NULL,"/");
    }
    *block_position = child_node.child_node_block;
    *mds_id = directory_struct.nodeid;
  }
  else
  {
    directory_struct = search(mfs_fd , retrieved_node.block_size , retrieved_node.working_directory[0],retrieved_node.working_directory[1]);
    tokken = strtok(path,"/");
    char directory[BUFSIZE];
    while( tokken != NULL )
    {
        child_node = name_search(mfs_fd , retrieved_node.block_size , directory_struct.data.direct_blocks[0], tokken ,&flag);
        if(flag == 0)
        {
          printf("directory '%s' does not exists in location '%s'\n", tokken, path);
          return FAIL_MFS;
        }
        //printf("Tokken (%s) found with MDS in block (%u)\n", tokken , child_node.child_node_block);
        directory_struct = search(mfs_fd , retrieved_node.block_size , child_node.child_node_block,child_node.child_nodeid);
        if (!isFile)
        {
          if(directory_struct.type != TYPE_DIRECTORY)
          {
            printf("'%s' isn't a directory to open\n",directory_struct.filename);
            return FAIL_MFS;
          }
        }
        tokken = strtok(NULL,"/");
    }
    *block_position = child_node.child_node_block;
    *mds_id = directory_struct.nodeid;
  }
  return SUCCESS_MFS;
}
