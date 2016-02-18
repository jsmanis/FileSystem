#define DECL_FILE
#include "mfs_export.h"

int mfs_export(char input_tokkens[NUMBER_COMMANDS][BUFSIZE],int pos)
{

  int mfs_fd = open(mfs_filename,O_RDONLY,0666);
  if (mfs_fd < 0)
  {
    perror("mfs_export on opening file");
    return FAIL_MFS;
  }
  Superblock retrieved_node;
  if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
  {
    perror("Error reading superblock on printFullpath");
    close(mfs_fd);
    return FAIL_MFS;
  }


  int i = 0;
  if (pos < 3)
  {
    fprintf(stderr,"Mfs_export should have at least 2 arguments");
    return FAIL_MFS;
  }


  for (i=1;i<pos-1;i++) // through every file
  {
    printf("1\n");
    char append_name[BUFSIZE];
    strcpy(append_name,reverseNameOnly(input_tokkens[i]));
    // printf("finally %s \n",append_name);
    char unix_directory_name[BUFSIZE];
    strcpy(unix_directory_name,input_tokkens[pos-1]);
    strcat(unix_directory_name,"/");
    strcat(unix_directory_name,append_name);



    int fp_fd = open(unix_directory_name,O_RDWR | O_CREAT,0666);
    if (fp_fd < 0)
    {
      fprintf(stderr,"File '%s' can not be opened",unix_directory_name);
      continue;
    }
    printf("2\n");

    int block_number = -1;
    int mds_id = -1;

    findDirectoryByName(mfs_fd,input_tokkens[i],&block_number,&mds_id,1);
    if (block_number == -1 || mds_id == -1 )
    {
      fprintf(stderr,"MFS file '%s' does not exist.\n",input_tokkens[i]);
      continue;
    }
    printf("3\n");

    MDS mds_export = search(mfs_fd,retrieved_node.block_size,block_number,mds_id);
    // printf("debug %d %d \n",block_number,mds_id);
    int j = 0;

    // for (j=0;j< DIRECT_POINTERS_NUM;j++)
    // {
    //   printf("%d points to %d\n",j,mds_export.data.direct_blocks[j]);
    //
    // }

    for ( j=0 ; j < DIRECT_POINTERS_NUM; j++ )
    {
      printf("Block number is %d \n",mds_export.data.direct_blocks[j]);

      //write data from mfs to unix filesystem
      //char *binarybuffer = (char*)malloc(retrieved_node.block_size);
      char binarybuffer[retrieved_node.block_size];
      memset(binarybuffer,0,retrieved_node.block_size);

      int seek_ret = lseek(mfs_fd,sizeof(Superblock)+(mds_export.data.direct_blocks[j]-1)*retrieved_node.block_size,SEEK_SET);
      if( seek_ret < 0 )
      {
        perror("On reading block on mfs_export");
        return FAIL_MFS;
      }
      int myread = 0;
      myread = read( mfs_fd , binarybuffer , retrieved_node.block_size );

      // printf("I want to write %d with %s \n",myread,binarybuffer);
    //memcpy(&binarybuffer,&bufptr,retrieved_node.block_size);
      if (myread == 0)
        break;
      // printf("I will write\n");
      if ( write( fp_fd , &binarybuffer , myread ) == -1 )
      {
        perror("Error on writing binary file");
        close(mfs_fd);
        return FAIL_MFS;
      }

    }
    if ( mds_export.data.direct_blocks[DIRECT_POINTERS_NUM+1] != 0 ) //first-level export
    {
      int capacity = -1 ;
      int myseek = lseek(mfs_fd,sizeof(Superblock)+(mds_export.data.direct_blocks[DIRECT_POINTERS_NUM+1]-1)*retrieved_node.block_size,SEEK_SET);
      if ( myseek < 0 )
      {
        perror("Seek error on reading capacity on first-level export");
        return FAIL_MFS;
      }
      if ( read( mfs_fd , &capacity , sizeof(int) ) == -1 )
      {
        perror("Error reading capacity on first-level export");
        return FAIL_MFS;
      }
      int k = 0;
      printf("capacity is %d \n",capacity);
      for ( k = 0 ; k < capacity; k++ )
      {
        int block_number = 0;
        lseek(mfs_fd,sizeof(Superblock)+(mds_export.data.direct_blocks[DIRECT_POINTERS_NUM+1] - 1)*retrieved_node.block_size+sizeof(int)+k*sizeof(int),SEEK_SET);
        if ( read( mfs_fd , &block_number , sizeof(int) ) == -1 )
        {
          perror("Error reading block_number on first-level export");
          return FAIL_MFS;
        }
        printf("Block number is %d \n",block_number);
        int the_seek = lseek(mfs_fd,sizeof(Superblock)+(block_number-1)*retrieved_node.block_size,SEEK_SET);
        if ( the_seek < 0)
        {
          perror("Seek error first-level export");
          return FAIL_MFS;
        }

        char binarybuffer[retrieved_node.block_size];
        memset(binarybuffer,0,retrieved_node.block_size);

        int myread = 0;
        myread = read( mfs_fd , binarybuffer , retrieved_node.block_size );

        if (myread == 0)
          break;
        if ( write( fp_fd , &binarybuffer , myread ) == -1 )
        {
          perror("Error on writing binary file");
          close(mfs_fd);
          return FAIL_MFS;
        }

      }

    }

    if ( mds_export.data.direct_blocks[DIRECT_POINTERS_NUM+2] != 0 ) //second-level export
    {
      int outer_capacity = -1 ;
      int myseek = lseek(mfs_fd,sizeof(Superblock)+(mds_export.data.direct_blocks[DIRECT_POINTERS_NUM+2]-1)*retrieved_node.block_size,SEEK_SET);
      if ( myseek < 0 )
      {
        perror("Seek error on reading capacity on second-level export");
        return FAIL_MFS;
      }
      if ( read( mfs_fd , &outer_capacity , sizeof(int) ) == -1 )
      {
        perror("Error reading capacity on second-level export");
        return FAIL_MFS;
      }
      printf("Outer is %d \n ",outer_capacity);
      int m = 0 ;
      for( m = 0 ; m < outer_capacity ; m++ )
      {
        int seek_err = lseek(mfs_fd,sizeof(Superblock)+(mds_export.data.direct_blocks[DIRECT_POINTERS_NUM+2]-1)*retrieved_node.block_size + m*sizeof(int) + sizeof(int),SEEK_SET);
        if ( seek_err < 0 )
        {
          perror("Seek error on reading outer_capacity on second-level export");
          return FAIL_MFS;
        }

        int my_block_num = 0 ;
        if ( read( mfs_fd , &my_block_num , sizeof(int) ) == -1 )
        {
          perror("Error reading my_block_num on export");
          return FAIL_MFS;
        }
        // printf("My block num is %d \n",my_block_num);
        seek_err = lseek(mfs_fd,sizeof(Superblock)+(my_block_num-1)*retrieved_node.block_size,SEEK_SET);
        if ( seek_err < 0 )
        {
          perror("Seek error on reading capacity on second-level export");
          return FAIL_MFS;
        }
        int capacity = 0;
        if ( read( mfs_fd , &capacity , sizeof(int) ) == -1 )
        {
          perror("Error reading capacity on second-level export");
          return FAIL_MFS;
        }
        printf("Capacity is %d \n",capacity);
        int t = 0;
        for ( t = 0 ; t < capacity ; t++ )
        {
          printf("t is %d \n",t);
          int seek_err = lseek(mfs_fd,sizeof(Superblock)+(my_block_num-1)*retrieved_node.block_size + t*sizeof(int) + sizeof(int),SEEK_SET);
          if ( seek_err < 0 )
          {
            perror("Seek error on reading outer_capacity on second-level export");
            return FAIL_MFS;
          }
          int the_block_num = 0 ;
          if ( read( mfs_fd , &the_block_num , sizeof(int) ) == -1 )
          {
            perror("Error reading my_block_num on export");
            return FAIL_MFS;
          }
          // printf("The block num is %d \n",the_block_num);

          char binarybuffer[retrieved_node.block_size];
          memset(binarybuffer,0,retrieved_node.block_size);

          int myread = 0;
          myread = read( mfs_fd , binarybuffer , retrieved_node.block_size );
          printf("My read is %d \n",myread);
          if (myread == 0)
            break;
          if ( write( fp_fd , &binarybuffer , myread ) == -1 )
          {
            perror("Error on writing binary file");
            close(mfs_fd);
            return FAIL_MFS;
          }

        }


      }

    }


    close(fp_fd);
  }






  //fwrite (buffer , sizeof(char), sizeof(buffer), pFile)  //
  //
  // int mfs_fd = open(mfs_filename,O_RDONLY,0666);
  // if (mfs_fd < 0)
  // {
  //   perror("mfs_export on opening file");
  //   return FAIL_MFS;
  // }
  //
  // int block_position = -1;
  // int mds_id = -1;
  // if ( findDirectoryByName(mfs_fd,directory,&block_position,&mds_id) == FAIL_MFS )
  // {
  //   return FAIL_MFS;
  // }

  return SUCCESS_MFS;
}

char* reverseNameOnly(char* name)
{
  int i = 0;
  int num_chars = 0;
  char tempbuf[BUFSIZE];
  printf("Received %s \n",name);
  for (i=strlen(name) - 1;i >=0 ;i--)
  {

    if ( i == strlen(name) - 1 && name[i] == '/' )
    {
        continue;
    }
    if(name[i] == '/')
      break;

    tempbuf[num_chars] = name[i];
    num_chars++;
  }

  tempbuf[num_chars] = '\0';
  char returnstring[BUFSIZE];
  printf("TEMP IS %s\n",tempbuf);

  int j = 0;
  for (i = strlen(tempbuf)-1 ; i>=0 ; i-- )
  {
    returnstring[j] = tempbuf[i];
    j++;
  }
  returnstring[j] = '\0';
  printf("IT IS %s\n",returnstring);
  return returnstring;

}
