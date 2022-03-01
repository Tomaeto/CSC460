#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
//Adrian Faircloth
//2-28-22
//CSC460
//Cheetah Sync

//Syncs n processes to print character and pid n times using semaphores

main(int argc, char *argv[])
{
	int i;
	int firstID, myID;
	int sem_id;

//Checking if command-line arg was given
	if (argc == 1)
	{
		printf("No argument given.\n");
		return(0);
	}

//Getting n from command line
	int n = atoi(argv[1]);

//Checking if given arg is within bounds
	if (n < 1 || n > 26)
	{
		printf("Argument out of range (1 - 26).\n");
		return(0);
	}

//Storing parent process' PID and initializing myID relative to other processes
	firstID = getpid();
	myID = 0;

//Getting n semaphores for syncing processes
	sem_id = semget(IPC_PRIVATE, n, 0777);
	if (sem_id == -1)
	{
		printf("Failed to get semaphores.\n");
		return(0);
	}

//Setting first semaphore's value to 1
	semctl(sem_id, myID, SETVAL, 1);

//Setting all other semaphore values to 0
	for (i = 1; i < n; i++)
	{
		semctl(sem_id, i, SETVAL, 0);

//Forking n - 1 processes and incrementing myID for each
		if (fork() == 0)
			myID++;
		else
			break;
	}	

//Setting unique char for each process
	char letter = 65 + myID;

//Using semaphores to print processes in order by myID n times each
	for (i = 0; i < n; i++)
	{
		p(myID, sem_id);
		printf("%c:%d\n", letter, getpid());

	//If current process is last, loop around to first process
	//Else continue to next process
		if(myID == n - 1)
			v(0, sem_id);
		else
			v(myID+1, sem_id);
	}

//Sleep to allow all processes to finish before removing semaphores
	sleep(2);

//Original process removing semaphores
	if (firstID == getpid())
	{
		if ( (semctl(sem_id, 0, IPC_RMID, 0)) == -1)
			printf("Error removing semaphores.\n");
	}

	return(0);
}

//Semaphore block/continue functions
p(int s,int sem_id)
{
struct sembuf sops;

sops.sem_num = s;
sops.sem_op = -1;
sops.sem_flg = 0;
if((semop(sem_id, &sops, 1)) == -1) printf("%s", "'P' error\n");
}

v(int s, int sem_id)
{
struct sembuf sops;

sops.sem_num = s;
sops.sem_op = 1;
sops.sem_flg = 0;
if((semop(sem_id, &sops, 1)) == -1) printf("%s","'V' error\n");
}


