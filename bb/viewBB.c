#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
//Adrian Faircloth
//2-25-22
//CSC460
//Shared Bulletin Board

//Prints bulletin board information stored in shared memory

//Struct for info stored in shared mem as array of bbStruct
struct bbStruct {
	int id;
	char name[20];
	int favNum;
	char favFood[30];
};

main(int argc, char* argv[])
{
	int bbID;
	struct bbStruct *shmem;
	FILE *fopen(), *idfile;

//Opening file w/ ID of shared memory
	if ( (idfile = fopen("/pub/os/bb/BBID.txt", "r")) == NULL)
	{
		printf("Could not open BBID.txt\n");
		return(0);
	}

//Getting shared memory ID from file and attaching
	fscanf(idfile, "%d", &bbID);
	fclose(idfile);
	shmem =  shmat(bbID, NULL, SHM_RND);

//Printing info from each struct stored in shared memory
	int i;
	for(i = 0; i < 17; i++)
	{
		printf("%2d: %20s| %8d| %30s|\n", shmem[i].id, shmem[i].name, shmem[i].favNum, shmem[i].favFood);
	}	

	return(0);
}
