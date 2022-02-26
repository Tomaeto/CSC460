#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
//Adrian Faircloth
//2-25-22
//Shared Bulletin Board

//Updates favNum field of my struct in shared memory
struct bbStruct {
	int id;
	char name[20];
	int favNum;
	char favFood[30];
};

main(int argc, char* argv[])
{

	int bbID, favNum;
	struct bbStruct* shmem;
	FILE *fopen(), *idfile;

//Checking if command-line argument was given
	if (argc == 1)
	{
		printf("No argument given.\n");
		return(0);
	}

//Getting favNum from command line
	favNum = atoi(argv[1]);

//Opening file w/ ID of shared memory
	if( (idfile = fopen("/pub/os/bb/BBID.txt", "r")) == NULL)
	{
		printf("Could not open BBID.txt\n");
		return(0);
	}

//Getting shared memory ID from file and attaching
	fscanf(idfile, "%d", &bbID);
	fclose(idfile);
	shmem = shmat(bbID, NULL, SHM_RND);

//Updating favNum field of my struct in shared memory (struct w/ my info is third)
	shmem[2].favNum = favNum;


	printf("favNum successfully updated.\n");
	return(0);
}
