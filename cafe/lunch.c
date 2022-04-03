#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <time.h>
//Adrian Faircloth
//4-2-22
//CSC460
//Cristina's Cafe

//Simulates Dining Philosophers problem, syncs processes to prevent deadlock

//Defines for Left & Right ID's
#define LEFT (myID + N + 1) % N
#define RIGHT (myID + 1) % N

//Define for mutex sem
#define MUTEX 0

//Defines for philosopher states
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define DEAD 3

//Define for shmem pointer to philosoper state array
#define STATE shmem
main()
{
	int i;
	int shmid, *shmem;
	int randNum;
	int myID = -1;
	int N = 5;
	int numDead = 0;
	time_t start;

//Getting N + 1 semaphores, 1 for each philosopher and a mutex
	int sem_id = semget(IPC_PRIVATE, N+1, 0777);
	if (sem_id == -1)
	{
		printf("Failed to get sems.\n");
		return(0);
	}

//Getting shmem array for N ints for philosopher states
	shmid = shmget(IPC_PRIVATE, N*sizeof(int), 0770);	
	if (shmid == -1)
	{
		printf("Failed to get shmem.\n");
		return(0);
	}

//Attatching to shared memory
	shmem = (int*) shmat(shmid, NULL, SHM_RND);

//Forking N processes to act as philosophers
	for (i = 0; i < N; i++)
	{
		if (fork()== 0) myID++;
			
		else break;
	}

//Setting mutex sem to 1
	semctl(sem_id, MUTEX, SETVAL, 1);

	if (myID >= 0)
	{//Philosophers
	 //For 100 seconds, perform Dining Philosophers algorithm:
	 //	'Think' (sleep) for 5 - 15 seconds
	 //	Get 'Hungry' and attempt to eat, block if not
	 //	If able, 'Eat' for 1 - 3 seconds
	 //	Return sems and repeat
	 //After 100 seconds and current loop ends, record state as 'dead' and exit
	
	//Setting philosopher sems to 0 before loop
		semctl(sem_id, myID, SETVAL, 0);
	//Recording starting time
		start = time(NULL);
		while (time(NULL) - start < 100)
		{		
		//*** Thinking ***
		//Record state as 'Thinking'
			shmem[myID] = THINKING;

		//Seed random number generator, get rand num, and sleep
			srand(getpid());
			randNum = (rand() % 10) + 5;
			sleep(randNum);

		//*** Become Hungry/Get 'Chopsticks' ***

		//Enter critical section
			p(MUTEX, sem_id);

		//Set state to 'Hungry'
			shmem[myID] = HUNGRY;

		//Test if philosopher is able to eat (is hungry, left & right are not eating)
			test(myID, sem_id, shmem);

		//Leave critical section
			v(MUTEX, sem_id);

		//Enter 'Eating' semaphore, blocks if not able
			p(myID, sem_id);
			
		//*** Eating ***
		//Sleep for 1-3 seconds
			randNum = rand() % 3 + 1;
			sleep(randNum);

		//*** Return 'Chopstick' ***
		//Enter critical section
			p(MUTEX, sem_id);

		//Set state back to thinking
			STATE[myID] = THINKING;

		//Test if Left & Right philosophers can eat
			test(LEFT, sem_id, shmem);
			test(RIGHT, sem_id, shmem);

		//Leave critical section
			v(MUTEX, sem_id);
			
		}

	//After Dining Algorithm, set state to dead and exit.
		STATE[myID] = DEAD;
		return(0);
	}

	else
	{//Parent Process
	 //Prints states of philosophers each second until all are dead
		int printNum = 0;
		while (numDead < N)
		{
		//Sleep b/w prints, allows philosophers to get started first time
			sleep(1);
			printNum++;

		//Reset numDead counter
			numDead = 0;

		//Print printNum w/ states of philosophers
			printf("%d. ", printNum);
			for (i = 0; i < N; i++)
			{
			//For each philosopher, check state and print accordingly
				if 	(STATE[i] == THINKING)
					printf("thinking\t");
				else if (STATE[i] == HUNGRY)
					printf("hungry\t");
				else if (STATE[i] == EATING)
					printf("eating\t");
				else if (STATE[i] == DEAD)
				{
				//If state is dead, increment numDead counter
					printf(" dead\t");
					numDead++;
				}
				else
					printf("????\t");
			}
			printf("\n");
		}

		//After all philosophers are dead, remove shared resources and exit.
		if (shmdt(shmem) == -1) 
			printf("Error detatching shmem.\n");

		if ( (shmctl(shmid, IPC_RMID, NULL)) == -1 )
			printf("Error removing shmem.\n");

		if ( (semctl(sem_id, 0, IPC_RMID, 0)) == -1 )
			printf("Error removing sems.\n");
		printf("Done\n");
		return(0);
	}
}

//Function for checking if philosopher is able to eat
//If philosopher is hungry and neighbors aren't eating, set state to eating
//	and v() on philosopher's semaphore
//Allows philosopher to not get blocked when attempting to enter eating state
test(int myID, int sem_id, int *shmem)
{
	int N = 5;
	if (STATE[myID] == HUNGRY && STATE[LEFT] != EATING && STATE[RIGHT] != EATING)
	{
		STATE[myID] = EATING;
		v(myID, sem_id);
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

