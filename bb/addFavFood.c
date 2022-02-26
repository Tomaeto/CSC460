#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
//Adrian Faircloth
//2-25-22
//CSC460
//Shared Bulletin Board

//Updates favFood field of my struct in bulletin board shared memory

//struct for shared memory stored as array of bbStruct
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

//Checking if command-line arg was given
	if (argc == 1)
	{
		printf("No argument given.\n");
		return(0);
	}

//Checking if given arg is longer than size in struct	
	if ( strlen(argv[1]) > 30)
	{
		printf("Argument too long, max is 30 chars.\n");
		return(0);
	}
	
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

//Updating favFood field of my struct in shared memory (the third struct)
	strcpy(shmem[2].favFood, argv[1]);

	printf("favFood successfully updated.\n");
	return(0);
}
