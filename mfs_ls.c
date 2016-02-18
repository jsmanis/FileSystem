#define DECL_FILE
#include "mfs_ls.h"

int mfs_ls(char input_tokkens[NUMBER_COMMANDS][BUFSIZE],int pos)
{
	int i = 0, count = 0, aExists = 0, rExists = 0, UExists = 0, dExists = 0, lExists = 0, capacity = -1, block_number, child_capacity = -1;
	char  files[BUFSIZE][BUFSIZE];
	count=0;
  	for (i = 1; i < pos; i++)
  	{
		if (strcmp("-a",input_tokkens[i]) == 0)
   	{
   	   		aExists = 1;
		}
		else if (strcmp("-r",input_tokkens[i]) == 0)
   	{
			rExists = 1;
		}
		else if (strcmp("-l",input_tokkens[i]) == 0)
		{
			lExists = 1;
		}
		else if (strcmp("-U",input_tokkens[i]) == 0)
		{
			UExists = 1;
		}
		else if (strcmp("-d",input_tokkens[i]) == 0)
		{
			dExists = 1;
		}
		else
		{
			strcpy(files[count],input_tokkens[i]);
			count++;
		}
	}
	int mfs_fd = open(mfs_filename,O_RDWR,0666);
	MDS searchNode, recursiveNode;
	child childNode;

	if (mfs_fd < 0)
	{
		perror("mfs_ls on opening file");
		close(mfs_fd);
		return FAIL_MFS;
	}

	Superblock retrieved_node;
	if ( read( mfs_fd , &retrieved_node , sizeof(Superblock) ) == -1 )
	{
		perror("Error reading superblock on mfs_mkdir");
		close(mfs_fd);
		return FAIL_MFS;
	}
	MDS strct = search(mfs_fd, retrieved_node.block_size,1,1);
	printMDS(mfs_fd, strct, retrieved_node.block_size);
	// sleep(100000);
	if ( UExists == 1 )
	{
		if( count == 0 )
		{
			if ( lsInPath( mfs_fd , retrieved_node.block_size , retrieved_node.working_directory[0] , retrieved_node.working_directory[1] , aExists , lExists , rExists, dExists) == FAIL_MFS )
			{
				perror("error in lsInPath");
				return FAIL_MFS;
			}
		}
	}
	printf("\n"); 
	close(mfs_fd);
	return SUCCESS_MFS;
}

int recursivePrintDirectory(int mfs_fd , int block_size , int block_number , int lExists)
{
	int capacity = -1, j;
	child searchstruct,child;
	MDS searchNode;

	//printf("block number %d\n",block_number);
	if ( block_number != 0 ) 
	{
		lseek(mfs_fd,sizeof(Superblock)+(block_number-1)*block_size+sizeof(int),SEEK_SET);
		if ( read( mfs_fd , &capacity , sizeof(int) ) == -1 )
		{
			perror("Searching on child block");
			return;
		}
	   	printf("capacity1 %lu\n", ((block_size-2*sizeof(int))/sizeof(child))-capacity);
		j=0;
		while(j<(((block_size-2*sizeof(int))/sizeof(child))-capacity))
		{
			//printf("Loop no : %d\n", j);
			if ( read( mfs_fd , &searchstruct , sizeof(child) ) == -1 )
			{
				perror("Searching on child block");
				return ;
			}
			searchNode = search(mfs_fd, block_size, searchstruct.child_node_block,searchstruct.child_nodeid);
			if ( lExists == 1 )
			{
				printf("Name %s\tCreationTime %ld\tModificationTime %ld\tAccessTime %ld\tSize %d\t \n", searchNode.filename, searchNode.creation_time, searchNode.modification_time, searchNode.access_time, searchNode.size);
			}
			else
			{
				printf("%s\n", searchstruct.childname);	
			}
			if (searchNode.type == TYPE_DIRECTORY)
			{
				printf("\tWe have a directory so we print it's members \n");
				//printf("next block : %d\n", searchNode.data.direct_blocks[0]);
				if( recursivePrintDirectory( mfs_fd , block_size , searchNode.data.direct_blocks[0] , lExists ) == FAIL_MFS )
				{
					return FAIL_MFS;
				}
			}
			//printf("xazos\n");
			j++;
		}
	}
	else
		printf("Directory is empty !\n");
	return SUCCESS_MFS; 
}




int lsInPath( int mfs_fd , int block_size , int working_block , int working_directory_id , int aExists, int lExists, int rExists, int dExists)
{
	int capacity = -1, block_number, child_capacity = -1, i;
	child childNode;
	MDS searchNode, recursiveNode;

	searchNode = search(mfs_fd, block_size, working_block, working_directory_id);
	//printf("working id : %d\n", working_block);
	block_number = searchNode.data.direct_blocks[0];		

	//printf("We are on block %d and id %d indirect block %d\n", retrieved_node.working_directory[0], retrieved_node.working_directory[1], block_number);
	//printf("block num : %d\n", block_number);
	while(block_number!=0)
	{
		lseek(mfs_fd,sizeof(Superblock)+(block_number-1)*block_size+sizeof(int),SEEK_SET);
		if ( read( mfs_fd , &capacity , sizeof(int) ) == -1 )
		{
			perror("Searching on child block");
			return;
	   }
	   //printf("records fits in  block : %lu\n", ((block_size-2*sizeof(int))/sizeof(child))-capacity);
		i=0;
		while(i<(((block_size-2*sizeof(int))/sizeof(child))-capacity))
		{
			// printf("I : %d \n", i);
			lseek(mfs_fd,sizeof(Superblock)+(block_number-1)*block_size+2*sizeof(int)+i*sizeof(child),SEEK_SET);
			if ( read( mfs_fd , &childNode , sizeof(child) ) == -1 )
			{
				perror("Searching on child block");
				return ;
			}
			// printf("searching for : %s with id : %dblock_id :%d\n", childNode.childname, childNode.child_nodeid, childNode.child_node_block);							
			if ( aExists == 1 )
			{
				if ( lExists == 0 )
				{
					printf("%s\t", childNode.childname);	
				}
				else 
				{
					recursiveNode = search(mfs_fd , block_size ,childNode.child_node_block ,childNode.child_nodeid );
					printf("Name %s\tCreationTime %ld\tModificationTime %ld\tAccessTime %ld\tSize %d\t \n", recursiveNode.filename, recursiveNode.creation_time, recursiveNode.modification_time, recursiveNode.access_time, recursiveNode.size);
				}
			}
			else if( rExists == 1 )
			{
				recursiveNode = search(mfs_fd , block_size ,childNode.child_node_block ,childNode.child_nodeid );
				if ( recursiveNode.type == TYPE_DIRECTORY )	
				{
					if ( lExists == 0 )
					{
						printf("%s\n", recursiveNode.filename);	
					}
					else 
					{
						printf("Name %s\tCreationTime %ld\tModificationTime %ld\tAccessTime %ld\tSize %d\t \n", recursiveNode.filename, recursiveNode.creation_time, recursiveNode.modification_time, recursiveNode.access_time, recursiveNode.size);
					}
						printf("\tWe have a directory so we print it's members \n");
						if( recursivePrintDirectory( mfs_fd , block_size , recursiveNode.data.direct_blocks[0] , lExists ) == FAIL_MFS )
					{
						return FAIL_MFS;
					}
				}
				else 
				{
					if ( lExists == 0 )
					{
						printf("%s\t", recursiveNode.filename);	
					}
					else 
					{
						printf("Name %s\tCreationTime %ld\tModificationTime %ld\tAccessTime %ld\tSize %d\t \n", recursiveNode.filename, recursiveNode.creation_time, recursiveNode.modification_time, recursiveNode.access_time, recursiveNode.size);
					}
				}
			}
			else if ( dExists == 1 )
			{
				recursiveNode = search(mfs_fd ,block_size ,childNode.child_node_block ,childNode.child_nodeid );
				if(recursiveNode.type == TYPE_DIRECTORY)
				{
					if ( lExists == 0 )
					{
						printf("%s\t", recursiveNode.filename);	
					}
					else 
					{
						printf("Name %s\tCreationTime %ld\tModificationTime %ld\tAccessTime %ld\tSize %d\t \n", recursiveNode.filename, recursiveNode.creation_time, recursiveNode.modification_time, recursiveNode.access_time, recursiveNode.size);
					}
				}
			}
			else
			{
				if(childNode.childname[0] != '.')
				{
					if ( lExists == 0 )
					{
						printf("%s\t", childNode.childname);	
					}
					else 
					{
						printf("searching for : %s with id : %dblock_id :%d\n", childNode.childname, childNode.child_nodeid, childNode.child_node_block);
						recursiveNode = search(mfs_fd ,block_size ,childNode.child_node_block ,childNode.child_nodeid );
						printf("Name %s\tCreationTime %ld\tModificationTime %ld\tAccessTime %ld\tSize %d\t \n", recursiveNode.filename, recursiveNode.creation_time, recursiveNode.modification_time, recursiveNode.access_time, recursiveNode.size);
					}
				}
			}
			i++;
		}
		lseek(mfs_fd,sizeof(Superblock)+(block_number-1)*block_size,SEEK_SET);
		if ( read( mfs_fd , &block_number , sizeof(int) ) == -1 )
		{
					perror("Reading block number");
			return ;
		}
		printf("block num : %d\n", block_number);
	}
	return SUCCESS_MFS;
}