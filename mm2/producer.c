#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
//Adrian Faircloth
//4-28-22
//CSC460
//Memory Manager 2: Producer

//Sends product requests to consumer through bounded buffer, dies when request is complete

//defines for MUTEX, FULL, and EMPTY semaphores
#define MUTEX 0
#define FULL 1
#define EMPTY 2

//defines for FRONT and REAR of buffer, and STOP flag
#define FRONT shmem[buf].size
#define REAR shmem[buf+1].size
#define STOP shmem[buf+2].size

//Struct for product requests
struct product {
	int pid;
	int semid;
	int size;
	int time;
	time_t start;
	char letter;
};

main(int argc, char* argv[])
{
	int sem_id, shmid, ram_id;
	int buf, ramsize;
	FILE *fp;
	struct product *shmem;

//Checking for correct number of command-line args
	if (argc != 3)
	{
		printf("Incorrect command-line arguments.\n");
		return(0);
	}

//Getting product size and time from command line
	int size = atoi(argv[1]);
	int time = atoi(argv[2]);

//Opening shmemdata file and getting size of ram, buffer size, and shared memory id's
	if ((fp = fopen("shmemdata", "r")) == NULL)
	{
		printf("Failed to open shmemdata.\n");
		return(0);
	}

	fscanf(fp, "%d %d %d %d %d", &ramsize, &buf, &sem_id, &shmid, &ram_id);
	fclose(fp);

//Checking if request size is within range
	if (size > ramsize || size < 1)
	{
		printf("Size out of range (1 - %d).\n", ramsize);
		return(0);
	}

//Checking if request time is within range
	if (time > 30 || time < 1)
	{
		printf("Requested time out of range (1 - 30).\n");
		return(0);
	}

//Attatching to shared memory
	shmem = (struct product *) shmat(shmid, NULL, SHM_RND);

//Printing request message
	printf("%d is requesting %d blocks of RAM for %d seconds...\n", getpid(), size, time);

//Kill producer if flagged to stop before entering semaphore section
	if (STOP != 0)
		return(0);

//Entering bounded buffer
	p(EMPTY, sem_id);	

//Kill producer if flagged to stop after entering semaphore section
	if (STOP != 0) return(0);
	p(MUTEX, sem_id);

//Getting semaphore for consumer to signal that producer's request is complete
	int mySem = semget(IPC_PRIVATE, 1, 0777);
	semctl(mySem, 0, SETVAL, 0);

//Entering request data into buffer
	shmem[REAR].pid = getpid();
	shmem[REAR].size = size;
	shmem[REAR].time = time;
	shmem[REAR].semid = mySem;

//Incrementing REAR pointer of buffer
	REAR = (REAR + 1) % buf;

//Leaving bounded buffer
	v(MUTEX, sem_id);
	v(FULL, sem_id);

//Deleting sem and killing producer if flagged to stop after bounded buffer
	if (STOP != 0)
	{
	semctl(mySem, 0, IPC_RMID, 0);
	return(0);
	}

//Blocking producer on producer's sem, unblocks after request is complete
	p(0, mySem);

//Printing finish message
	printf("%d has finished request for %d blocks of RAM for %d seconds.\n", getpid(), size, time);

//Deleting sem and exiting
	semctl(mySem, 0, IPC_RMID, 0);
	return(0);

	
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

