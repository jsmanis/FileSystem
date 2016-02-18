#include "utilities.h"

void updateSuperblockNextId(int mfs_fd)
{
  int block_size = 0;
  int id = 0;
  int ret = lseek(mfs_fd,0,SEEK_SET);
  if ( ret < 0 )
  {
    perror("Seek error");
  }

  Superblock superblock;
  Superblock old;

  if ( read( mfs_fd , &superblock , sizeof(Superblock ) ) == -1  )
  {
    perror("read");
    exit(1);
  }
  ret = lseek(mfs_fd,0,SEEK_SET);
  if ( ret < 0 )
  {
    perror("Seek error");
  }

  superblock.next_id++;

  block_size = superblock.block_size;
  id = superblock.next_id - 1;

  if (write( mfs_fd ,&superblock , sizeof(Superblock) ) == -1)
  {
    perror("updating next id of superblock");
    exit(1);
  }
  ret = lseek(mfs_fd,0,SEEK_SET);
  if ( ret < 0 )
  {
    perror("Seek error");
  }

}

void printSuperblock(Superblock superblock)
{
  printf("**********SUPERBLOCK**********\n");
  printf("Block size:(%d),size_mds:(%d),root_id:(%d),next_id:(%d),write_mds_no:(%d),working_directory[0]:(%d),working_directory[1]:(%d)\n",superblock.block_size,
          superblock.size_mds,superblock.root_id,superblock.next_id,superblock.write_mds_no,
          superblock.working_directory[0],superblock.working_directory[1]);
  printf("******************************\n");
}

int printMDS(int mfs_fd,MDS printable_mds,int block_size)
{
  printf("|nodeid: %d,filename: %s| parent: %u |",printable_mds.nodeid,printable_mds.filename,printable_mds.parent_nodeid[0]);
  if (printable_mds.type == TYPE_DIRECTORY)
  {
    int current_block = printable_mds.data.direct_blocks[0];
    while(current_block !=0)
    {
      printf("\n\n\n**********Datastream Block(%d)***********\n",current_block);

      int i = 0;
      int capacity = -1;
      lseek(mfs_fd,sizeof(Superblock)+(current_block-1)*block_size,SEEK_SET);
      if ( read( mfs_fd , &current_block , sizeof(int) ) == -1 )
      {
        perror("Error reading current_block on printMFS");
        return FAIL_MFS;
      }
      if ( read( mfs_fd , &capacity , sizeof(int) ) == -1 )
      {
        perror("Error reading capacity on printMFS");
        return FAIL_MFS;
      }
      // lseek(mfs_fd,)
      for (i=(block_size-2*sizeof(int))/sizeof(child);i>capacity;i--)
      {
        child child_node;
        if ( read( mfs_fd , &child_node , sizeof(child) ) == -1 )
        {
          perror("Error reading printable on printMFS");
          return FAIL_MFS;
        }
        printf("|child-node id (%d),child-node name (%s), child_node block Id(%d)|",child_node.child_nodeid,child_node.childname,child_node.child_node_block);
      }
      putchar('\n');
      printf("******************************\n\n\n");
    }
  }
  return SUCCESS_MFS;
}

int printMFS(int mfs_fd)
{
  Superblock retrieved_node;
  lseek(mfs_fd,0,SEEK_SET);
  if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
  {
    perror("read");
    return FAIL_MFS;
  }
  printSuperblock(retrieved_node);
  int current_block = 1;
  while(current_block <= retrieved_node.number_of_blocks)
  {
    printf("**********Block(%d)***********\n",current_block);

    int i = 0;
    int capacity;
    if ( read( mfs_fd , &capacity , sizeof(int) ) == -1 )
    {
      perror("Error reading capacity on printMFS");
      return FAIL_MFS;
    }
    // lseek(mfs_fd,)
    for (i=0;i<(retrieved_node.block_size-sizeof(int))/sizeof(MDS) - capacity;i++)
    {
      MDS printable_mds;
      if ( read( mfs_fd , &printable_mds , sizeof(MDS) ) == -1 )
      {
        perror("Error reading printable on printMFS");
        return FAIL_MFS;
      }
      printMDS(mfs_fd,printable_mds,retrieved_node.block_size);
      lseek(mfs_fd,sizeof(Superblock)+(current_block-1)*retrieved_node.block_size+sizeof(int)+(i+1)*sizeof(MDS),SEEK_SET);
    }
    putchar('\n');
    current_block++;
    lseek(mfs_fd,sizeof(Superblock) + (current_block-1) * retrieved_node.block_size,SEEK_SET);
    printf("******************************\n");
  }
}

int updateParentDatastream(int mfs_fd,int dir_nodeid, char *dir_name, int child_MDSblock, int parentBlock_number, int parent_nodeid)
{
  Superblock retrieved_node;
  lseek(mfs_fd,0,SEEK_SET);
  if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
  {
    perror("read");
    return FAIL_MFS;
  }

  //first go to working block
   MDS searchparent;
   printf("block %d , id %d\n", parentBlock_number, parent_nodeid);
  lseek(mfs_fd,sizeof(Superblock)+(parentBlock_number-1)*retrieved_node.block_size+sizeof(int),SEEK_SET);
   printf("num of blocks (outer while) is : %d\n", retrieved_node.number_of_blocks);
  while(1)
  {
   if ( read( mfs_fd , &searchparent , sizeof(MDS) ) == -1 )
   {
       perror("Searching on mfs_mkdir");
       return FAIL_MFS;
   }
   if (searchparent.nodeid == parent_nodeid)
   {
    printf("Datastream: %d\n", searchparent.data.direct_blocks[0]);
       //update
       if (searchparent.data.direct_blocks[0] == 0) //first time on adding a directory
       {

          printf("blocks before increase :%d\n", retrieved_node.number_of_blocks);
          retrieved_node.number_of_blocks ++;
          printf("blocks after increase :%d\n", retrieved_node.number_of_blocks);
           searchparent.data.direct_blocks[0] = retrieved_node.number_of_blocks;
           searchparent.data.last_chain_block = retrieved_node.number_of_blocks;
           lseek(mfs_fd,-sizeof(MDS) ,SEEK_CUR);
           if ( write( mfs_fd , &searchparent , sizeof(MDS) ) == -1)
           {
             perror("Updating search parent");
             return FAIL_MFS;
           }
           lseek(mfs_fd,0,SEEK_SET);
           if ( write( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1)
           {
             perror("Updating superblock");
             return FAIL_MFS;
           }
           printf("block to read :%d\n", retrieved_node.number_of_blocks);
           lseek(mfs_fd,sizeof(Superblock)+(retrieved_node.number_of_blocks-1)*retrieved_node.block_size,SEEK_SET);
           int ischain = 0; //puts 0 when isn't chain

           if ( write( mfs_fd , &ischain , sizeof(int) ) == -1)
           {
             perror("writing ischain");
             return FAIL_MFS;
           }
           int remaining_ints = (retrieved_node.block_size - 2 * sizeof(int) )/sizeof(child) - 1;

           if ( write( mfs_fd , &remaining_ints , sizeof(int) ) == -1)
           {
             perror("writing remaining_ints");
             return FAIL_MFS;
           }
           child dir_node;

           dir_node.child_nodeid = dir_nodeid;
           strcpy(dir_node.childname,dir_name);
           dir_node.child_node_block = child_MDSblock;

           if ( write( mfs_fd , &dir_node , sizeof(child) ) == -1)
           {
             perror("writing dir_node");
             return FAIL_MFS;
           }

       }
       else
       {
           //we have created already a block of ints
           lseek(mfs_fd,sizeof(Superblock)+(searchparent.data.last_chain_block-1)*retrieved_node.block_size+sizeof(int),SEEK_SET);
           int remaining_ints = -1;
           if ( read( mfs_fd , &remaining_ints , sizeof(int) ) == -1 )
           {
               perror("Reading remaining_ints");
               return FAIL_MFS;
           }
           if ( remaining_ints == 0 ) //full.Create new block of ints
           {
             retrieved_node.number_of_blocks ++;
            //  searchparent.data.direct_blocks[0] = retrieved_node.number_of_blocks;
             lseek(mfs_fd,sizeof(Superblock)+(searchparent.data.last_chain_block-1)*retrieved_node.block_size,SEEK_SET);
             if ( write( mfs_fd , &retrieved_node.number_of_blocks , sizeof(int) ) == -1)
             {
               perror("Updating number_of_blocks");
               return FAIL_MFS;
             }
             searchparent.data.last_chain_block = retrieved_node.number_of_blocks;

             lseek(mfs_fd,-sizeof(MDS) ,SEEK_CUR);
             if ( write( mfs_fd , &searchparent , sizeof(MDS) ) == -1)
             {
               perror("Updating search parent");
               return FAIL_MFS;
             }
             lseek(mfs_fd,0,SEEK_SET);
             if ( write( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1)
             {
               perror("Updating superblock");
               return FAIL_MFS;
             }
             lseek(mfs_fd,sizeof(Superblock)+(retrieved_node.number_of_blocks-1)*retrieved_node.block_size,SEEK_SET);
             int ischain = 0;

             if ( write( mfs_fd , &ischain , sizeof(int) ) == -1)
             {
               perror("writing ischain");
               return FAIL_MFS;
             }
             int remaining_ints = (retrieved_node.block_size - 2 * sizeof(int) )/sizeof(int) - 1;

             if ( write( mfs_fd , &remaining_ints , sizeof(int) ) == -1)
             {
               perror("writing remaining_ints");
               return FAIL_MFS;
             }
             child dir_node;

             dir_node.child_nodeid = dir_nodeid;
             strcpy(dir_node.childname,dir_name);
             dir_node.child_node_block = child_MDSblock;
             if ( write( mfs_fd , &dir_node , sizeof(child) ) == -1)
             {
               perror("writing dir_nodeid");
               return FAIL_MFS;
             }

           }
           else
           {
             int move = (retrieved_node.block_size-2*sizeof(int))/sizeof(child) - remaining_ints;
             printf("move : %d", move);

             lseek(mfs_fd,-sizeof(int),SEEK_CUR);
             remaining_ints--;
             if ( write( mfs_fd , &remaining_ints , sizeof(int) ) == -1)
             {
               perror("writing remaining_ints");
               return FAIL_MFS;
             }

             child dir_node;

             dir_node.child_nodeid = dir_nodeid;
             strcpy(dir_node.childname,dir_name);
             dir_node.child_node_block = child_MDSblock;
             printf("I'm going to write in block %d\n", dir_node.child_node_block );
             lseek(mfs_fd,move*sizeof(child),SEEK_CUR);
             if ( write( mfs_fd , &dir_node , sizeof(child) ) == -1)
             {
               perror("writing dir_nodeid");
               return FAIL_MFS;
             }

           }
          
        }
        break; 
    } 
  }
  printf("=lalalala\n");
 lseek(mfs_fd,0,SEEK_SET);
 if ( write( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
 {
   perror("writing superblock on updateParentDatastream");
   return FAIL_MFS;
 }
 printf("num of blocks is : %d\n", retrieved_node.number_of_blocks);
  //on searchstruct we have found the parent node idv
}

char* printFullpath(char *myfilename)
{
  int mfs_fd = open(myfilename,O_RDONLY,0666);
  if (mfs_fd < 0)
  {
    perror("on opening file");
    close(mfs_fd);
    return NULL;
  }

  Superblock retrieved_node;
  if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
  {
    perror("Error reading superblock on printFullpath");
    close(mfs_fd);
    return NULL;
  }

  MDS directory_struct;
  char temp_path[DIRECTORY_BUFSIZE];
  char full_path[DIRECTORY_BUFSIZE];

  directory_struct = search(mfs_fd,retrieved_node.block_size,retrieved_node.working_directory[0],retrieved_node.working_directory[1]);
  strcpy(full_path,directory_struct.filename);
  strcpy(temp_path,full_path);
  while( strcmp(directory_struct.filename,"/") ) //until we find root
  {
    directory_struct = search(mfs_fd,retrieved_node.block_size,directory_struct.parent_nodeid[0],directory_struct.parent_nodeid[1]);
    strcpy(full_path,directory_struct.filename);
    if( strcmp(directory_struct.filename,"/") )
      strcat(full_path,"/");
    strcat(full_path,temp_path);
    strcpy(temp_path,full_path);
  }
  close(mfs_fd);
  return full_path;
}

MDS search(int mfs_fd,int block_size,int block_number,int node_id)
{
  MDS searchstruct;

  lseek(mfs_fd,sizeof(Superblock)+(block_number-1)*block_size+sizeof(int),SEEK_SET);
  while(1)
  {
    //printf("lalala\n");
    if ( read( mfs_fd , &searchstruct , sizeof(MDS) ) == -1 )
    {
      perror("Searching on printDirectory");
      return;
    }
    if (searchstruct.nodeid == node_id)
    {
      return searchstruct;
    }
  }
}

int updateWorkingDirectory_ParentOnly(char *myfilename)
{
  int mfs_fd = open(myfilename,O_RDWR,0666);
  if (mfs_fd < 0)
  {
    perror("updateWorkingDirectory_ParentOnly on opening file");
    return FAIL_MFS;
  }

  Superblock retrieved_node;
  if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
  {
    perror("Error reading superblock on updateWorkingDirectory_ParentOnly");
    return FAIL_MFS;
  }

  MDS directory_struct;

  directory_struct = search(mfs_fd,retrieved_node.block_size,retrieved_node.working_directory[0],retrieved_node.working_directory[1]);

  retrieved_node.working_directory[0] = directory_struct.parent_nodeid[0];
  retrieved_node.working_directory[1] = directory_struct.parent_nodeid[1];

  int retseek = lseek(mfs_fd,0,SEEK_SET);
  if (retseek < 0 )
  {
    perror("Error on updating superblock updateWorkingDirectory_ParentOnly");
    return FAIL_MFS;
  }

  if ( write( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1)
  {
    perror("writing on updating superblock updateWorkingDirectory_ParentOnly");
    return FAIL_MFS;
  }

  close(mfs_fd);
  return SUCCESS_MFS;

}

int updateWorkingDirectory(char *myfilename,char *path)
{
  char *tokken;
  int flag = 1;
  int mfs_fd = open(myfilename,O_RDWR,0666);

  if (mfs_fd < 0)
  {
    perror("updateWorkingDirectory on opening file");
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

  MDS directory_struct;
  child child_node;

  if (path[0] == '/') // search absolute path
  {
    if( strlen(path) == 1)
    {
      retrieved_node.working_directory[0] = 1;
      retrieved_node.working_directory[1] = 1;
      if (lseek(mfs_fd,0,SEEK_SET) < 0 )
      {
        perror("Error on updating superblock updateWorkingDirectory");
        close(mfs_fd);
        return FAIL_MFS;
      }
      if ( write( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1)
      {
        perror("writing on updating superblock updateWorkingDirectory");
        close(mfs_fd);
        return FAIL_MFS;
      }
      close(mfs_fd);
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
          close(mfs_fd);
          return FAIL_MFS;
        }
        // printf("Tokken (%s)\n", tokken );//, child_node.child_node_block);
        directory_struct = search(mfs_fd , retrieved_node.block_size , child_node.child_node_block,child_node.child_nodeid);
        if(directory_struct.type != TYPE_DIRECTORY)
        {
          printf("'%s' isn't a directory to open\n", directory_struct.filename);
          close(mfs_fd);
          return FAIL_MFS;
        }
        tokken = strtok(NULL,"/");
    }
    retrieved_node.working_directory[0] = child_node.child_node_block;
    retrieved_node.working_directory[1] = directory_struct.nodeid;
    if (lseek(mfs_fd,0,SEEK_SET) < 0 )
    {
      perror("Error on updating superblock updateWorkingDirectory");
      close(mfs_fd);
      return FAIL_MFS;
    }
    if ( write( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1)
    {
      perror("writing on updating superblock updateWorkingDirectory");
      close(mfs_fd);
      return FAIL_MFS;
    }
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
          close(mfs_fd);
          return FAIL_MFS;
        }
        //printf("Tokken (%s) found with MDS in block (%u)\n", tokken , child_node.child_node_block);
        directory_struct = search(mfs_fd , retrieved_node.block_size , child_node.child_node_block,child_node.child_nodeid);
        if(directory_struct.type != TYPE_DIRECTORY)
        {
          printf("'%s' isn't a directory to open\n",directory_struct.filename);
          close(mfs_fd);
          return FAIL_MFS;
        }
        tokken = strtok(NULL,"/");
    }
    retrieved_node.working_directory[0] = child_node.child_node_block;
    retrieved_node.working_directory[1] = directory_struct.nodeid;
    if (lseek(mfs_fd,0,SEEK_SET) < 0 )
    {
      perror("Error on updating superblock updateWorkingDirectory");
      close(mfs_fd);
      return FAIL_MFS;
    }
    if ( write( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1)
    {
      perror("writing on updating superblock updateWorkingDirectory");
      close(mfs_fd);
      return FAIL_MFS;
    }
  }
  close(mfs_fd);
  return SUCCESS_MFS;
}

child name_search(int mfs_fd , int block_size , int block_number ,char *name ,int *flag)
{
  child searchstruct;
  int capacity, i;

  while(block_number!=0)
  {
    lseek(mfs_fd,sizeof(Superblock)+(block_number-1)*block_size+sizeof(int),SEEK_SET);
    if ( read( mfs_fd , &capacity , sizeof(int) ) == -1 )
    {
      perror("Searching on child block");
      return;
    }
    i=0;
    while(i<(block_size-2*sizeof(int)/sizeof(child)))
    {
      if ( read( mfs_fd , &searchstruct , sizeof(child) ) == -1 )
      {
        perror("Searching on child block");
        return ;
      }
      if (strcmp(searchstruct.childname, name)==0)
      {
        return searchstruct;
      }
      i++;
    }
    lseek(mfs_fd,sizeof(Superblock)+(block_number-1)*block_size,SEEK_SET);
    if ( read( mfs_fd , &block_number , sizeof(int) ) == -1 )
    {
      perror("Reading block number");
      return ;
    }
  }
  *flag = 0;
}



int updateParentMDSsize ( int mfs_fd, int block_size, int parentBlock_number, int parentNode_id, int sizeToIncreaze)
{
    MDS searchstruct;
    int retSeek;

    while(1)
    {
        searchstruct = search(mfs_fd, block_size, parentBlock_number, parentNode_id);
        searchstruct.size += sizeToIncreaze; 
        retSeek = lseek(mfs_fd, -sizeof(MDS), SEEK_CUR);
        if ( write( mfs_fd , &searchstruct , sizeof(MDS) ) == -1)
        {
          perror("writing on updating superblock updateParentMDSsize");
          close(mfs_fd);
          return FAIL_MFS;
        }
        if (parentNode_id ==1)
        {
            break;
        }
        parentNode_id = searchstruct.parent_nodeid[1];
        parentBlock_number = searchstruct.parent_nodeid[0];
    }
    return SUCCESS_MFS;
}