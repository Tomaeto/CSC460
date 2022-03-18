#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define COINS shmem[0]
//Adrian Faircloth
//3-18-22
//CSC460
//P7: Crypto Wallet
//
//Simulates a virtual wallet system w/ synced processes adding/taking from wallet

main(int argc, char *argv[])
{
	int sem_id, *shmem, shmid;
	FILE *fp
;
//No args call, initializes sem, shared memory wallet, and file w/ IDs of each
	if (argc == 1)
	{
	//Checking cryptodata file for sem id
		if ((fp = fopen("cryptodata", "r")) == NULL)
		{
			printf("Could not open cryptodata to read.\n");
			return(0);
		}	

		fscanf(fp, "%d", &sem_id);
		fclose(fp);

	//If sem ID is not 0, system is already initialized
		if (sem_id != 0)
		{
			printf("System already initialized.\n");
			return(0);
		}

	//Getting semaphore for syncing and initializing to 1
		sem_id = semget(IPC_PRIVATE, 1, 0777);
		if (sem_id == -1)
		{
			printf("SemGet failed.\n");
			return(0);
		}

		semctl(sem_id, 0, SETVAL, 1);

	//Getting shared memory for coin amount, initializing to 1000
		shmid = shmget(IPC_PRIVATE, sizeof(int), 0777);
		if (shmid == -1)
		{
			printf("Failed to get shared memory.\n");
			return(0);
		}

		shmem = (int *) shmat(shmid, NULL, SHM_RND);
		COINS = 1000;

	//Writing sem ID and shmem ID into cryptodata file, separated by a space
		if ((fp = fopen("cryptodata", "w")) == NULL)
		{
			printf("Could not open cryptodata to write.\n");
			return(0);
		}

		fprintf(fp, "%d ", sem_id);
		fprintf(fp, "%d", shmid);
		fclose(fp);
	
	//Printing success message and exiting	
		printf("System initialized and shared memory/semaphore ID's written to cryptodata.\n");
		return(0);
	}

//"cleanup" call, prints coin count and removes sem and shared mem if system is set up
	if (strcmp(argv[1], "cleanup") == 0)
	{

	//Checking cryptodata file for sem id
		if ((fp = fopen("cryptodata", "r")) == NULL)
		{
			printf("Could not open cryptodata to read.\n");
			return(0);
		}

		fscanf(fp, "%d", &sem_id);
		fscanf(fp, "%d", &shmid);
		fclose(fp);

	//If sem ID is 0, system is not initialized
		if (sem_id == 0)
		{
			printf("System not initialized, nothing to cleanup.\n");
			return(0);
		}

	//Attatching to shared mem, printing coin total, and detatching
		shmem = (int *) shmat(shmid, NULL, SHM_RND);
		printf("Final coin total: %d\n", COINS);
		shmdt(shmem);

	//Removing shared memory
		if((shmctl(shmid, IPC_RMID, NULL)) == -1)
		{
			printf("Error in removing shared memory.\n");
			return(0);
		}

	//Removing semaphore
		if ((semctl(sem_id, 0, IPC_RMID, 0)) == -1)
		{
			printf("Error in removing semaphore.\n");
			return(0);
		}

	//Opening cryptodata to replace sem and shmem IDs w/ 0
		if ((fp = fopen("cryptodata", "w+")) == NULL)
		{
			printf("Could not open cryptodata to write.\n");
			return(0);
		}

	//Overwriting file to only contain 0, indicates system is not initialized
		fprintf(fp, "%d", 0);
		fclose(fp);

	//Printing success message and exiting
		printf("Shmem/sem removed, cryptodata reset.\n");
		return(0);
	}	

//"coins" call, prints current coin total in wallet
	if (strcmp(argv[1], "coins") == 0)
	{
	//Checking cryptodata for sem ID and shmem ID
		if ((fp = fopen("cryptodata", "r")) == NULL)
		{
			printf("Could not open cryptodata to read.\n");
			return(0);
		}

		fscanf(fp, "%d", &sem_id);

	//If sem ID is 0, system is not initialized
		if (sem_id == 0)
		{
			printf("System not yet initialized.\n");
			return(0);
		}

		fscanf(fp, "%d", &shmid);
		fclose(fp);

	//Attatching to shmem, printing coin total, detatching and exiting
		shmem = (int *) shmat(shmid, NULL, SHM_RND);
		printf("Current coin total in wallet: %d\n", COINS);
		shmdt(shmem);
		return(0);
	}

//Int call, performs N loops of processes adding to/taking from wallet
//N is between 1 and 100
	else if ( atoi(argv[1]) > 0 && atoi(argv[1]) < 101)
	{
		int N = atoi(argv[1]);

	//Checking cryptodata file for sem ID and shmem ID
		if ((fp = fopen("cryptodata", "r")) == NULL)
		{
			printf("Could not open cryptodata to read.\n");
			return(0);
		}

		fscanf(fp, "%d", &sem_id);

	//If sem ID is 0, system is not initialized
		if (sem_id == 0)
		{
			printf("System not yet initialized.\n");
			return(0);
		}
		fscanf(fp, "%d", &shmid);

	//Attatching to shared memory
		shmem = (int *) shmat(shmid, NULL, SHM_RND);

	//Forking to a total of 16 processes, each w/ unique myVal 0 to 15
		int myVal = 0;
		int i;
		for (i = 0; i < 15; i++)
		{

			if (fork() == 0)
				myVal++;
			else
				break;
		}


	//Getting amount to give/take from wallet, 2^myVal
		int changeAmt = 1;
		for (i = 0; i < myVal; i++)
			changeAmt *= 2;

	//Each process forks child, child processes take changeAmt N times and parents add changeAmt N times
	//Syncs using semaphores, final coin count should always be 1000
		int oldAmt;
		if (fork() == 0)
		{
		//Child process
		//Takes changeAmt from wallet N times, uses semaphores to sync & prevent lost updates
			for (i = 0; i < N; i++)
			{
				p(0, sem_id);
			//Critical Section
			//Takes changeAmt from coins, prints line showing subtraction from wallet
				oldAmt = COINS;
				COINS -= changeAmt;
				printf("\t%d - %d = %d\n", oldAmt, changeAmt, COINS);
				v(0, sem_id);
			}
		}
		else
		{
		//Parent process
		//Adds changeAmt to wallet N times, uses semaphores to sync
			for (i = 0; i < N; i++)
			{
				p(0, sem_id);
			//Critical Section
			//Adds changeAmt to coins, prints line showing addition to wallet
				oldAmt = COINS;
				COINS += changeAmt;
				printf("%d + %d = %d\n", oldAmt, changeAmt, COINS);
				v(0, sem_id);
			}
		}

		return(0);	
	}

}


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

