#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
//Adrian Faircloth
//3-29-22
//CSC460
//Project 9: Bob's Bistro
//
//Simulates Dining Philosophers problem with semaphores, destined to deadlock

//Defines for infinite loop & left/right semaphore IDs
#define TRUE 1
#define LEFT (myID + 5 - 1) % 5
#define RIGHT (myID + 1) % 5

main()
{
	int i, j, dummyVal;
	int myID = 0;

//Getting 5 semaphores to act as 'chopsticks'
	int sem_id = semget(IPC_PRIVATE, 5, 0777);
	if (sem_id == -1)
	{
		printf("Failed to get sems.\n");
		return(0);
	}

//Forking to get 5 'philosophers' w/ unique myIDs (0 - 4)
	for (i = 0; i < 4; i++)
	{
		if (fork() == 0)
			myID++;
		else
			break;
	}

//Setting each semaphore to 1
	semctl(sem_id, myID, SETVAL, 1);

//Infinite loop to run problem:
//	Think: Spend time 'thinking'
//	Get Hungry: Attempt to get LEFT and RIGHT chopsticks
//	Eat: Spend time 'eating' and return chopsticks

//Destined to deadlock when each process has one 'chopstick' and is waiting for another to be available
	while(TRUE)
	{
	// *** Thinking ***
	// Printing thinking message w/ myID number of tabs
		for (i = 0; i < myID; i++) printf("\t");
		printf("%d THINKING...\n", myID);

	//Wasting CPU cycles to vary timing of each process
		for (i = 0; i < 1000; i++)
			for (j = 0; j < i; j++)
				dummyVal = i * j / 10;

	// ** Get Hungry ***
	// Printing hungry message w/ myID number of tabs
		for (i = 0; i < myID; i++) printf("\t");
		printf("%d HUNGRY...\n", myID);
		
	//Getting left 'chopstick' from semaphore
		p(LEFT, sem_id);

	//Wasting CPU cycles
		for (i = 0; i < 1000; i++)
			for (j = 0; j < i; j++)
				dummyVal = i * j / 10;

	//Getting right 'chopstick' from semaphore
		p(RIGHT, sem_id);

	//Printing eating message w/ myID number of tabs
		for (i = 0; i < myID; i++) printf("\t");
		printf("%d EATING...\n", myID);

	//Wasting CPU cycles
		for (i = 0; i < 1000; i++)
			for (j = 0; j < i; j++)
				dummyVal = i * j / 10;;

	//Returning 'chopsticks' for other processes to use
		v(LEFT, sem_id);
		v(RIGHT, sem_id);
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


