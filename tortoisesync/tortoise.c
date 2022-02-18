#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

#define TURN shmem[0]

//Adrian Faircloth
//2-17-22
//CSC460
//Tortoise Sync

//Syncs n processes to print in order n times using shared memory and busy wait

main (int argc, char *argv[])
{

//Initializing shared memory variables
	int shmemID, *shmem;

//Storing ID of original process for deallocating shared memory at the end
	int firstID = getpid();

//Checking if command-line argument was given
	if (argc == 1)
	{
		printf("No argument given.\n");
		return(0);
	}

//Getting arg from command line and checking if it is in range
	int n = atoi(argv[1]);
	if (n > 26 || n < 1)
	{
		printf("Given argument out of range (1 - 26).\n");
		return(0);
	}


//Allocating shared memory
	shmemID = shmget(IPC_PRIVATE, sizeof(int), 0770);
	if (shmemID == -1)
	{
		printf("Could not get shared memory.\n");
		return(0);
	}


//Initializing shared memory value
	shmem = (int*) shmat(shmemID, NULL, SHM_RND);
	TURN = 0;

//Initializing relative ID of first process
	int myID = 0;

//Building tree of N processes w/ myID's in order
	int i;
	for (i = 1; i < n; i++)
	{
		if (fork() == 0)
			myID++;
		else
			break;
	}

//Creating letter var corresponding to process' myID
	char letter = 65 + myID;

//Performing busy wait and process syncing to print letter and PID in order n times
	for (i = 0; i < n; i++)
	{
		//Busy wait for each process
		while (TURN != myID);
	
		printf("%c: %d\n", letter, getpid());

//Setting shared memory variable after printing line
//If current process is last in line, reset shared memory variable
		if (myID + 1 == n)
			TURN = 0;
		else
			TURN++;
	}

//Detatching and removing shared memory
	if (shmdt(shmem) == -1)
		printf("Error detatching shared memory.\n");

	if (firstID == getpid())
	{
		if ( (shmctl(shmemID, IPC_RMID, NULL)) == -1)
			printf("Error removing shmem.\n");
	}

//Sleeping before exiting so bash prompt isn't in w/ child process prints
	sleep(1);
	return(0);
}
