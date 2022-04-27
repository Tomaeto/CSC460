#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
//Adrian Faircloth
//4-17-22
//CSC460
//Memory Manager 1: Shutdown

//Flags consumer and producers to stop, removes semaphores and shared memory

//Defines for Buffer, RAM, and queue
#define BUF_MUTEX 0
#define FULL 1
#define EMPTY 2
#define RAM_MUTEX 3
#define Q_MUTEX 4

//Define for STOP flag
#define STOP shmem[buf+2].size

//struct for product requests
struct product {
	int pid;
	int semid;
	int size;
	int time;
	time_t start;
	char letter;
};

main()
{
	int sem_id, shmid;
	int ram_id, ramsize;
	int buf;
	struct product *shmem;
	struct product *ram;
	struct product *queue;
	FILE *fp;

//Opening shmemdata file and getting ram size, buffer size, and shared memory id's
	if ((fp = fopen("shmemdata", "r")) == NULL)
	{
		printf("Failed to open shmemdata.\n");
		return(0);
	}

	fscanf(fp, "%d %d %d %d %d", &ramsize, &buf, &sem_id, &shmid, &ram_id);

//Attatching to buffer and RAM
	shmem = (struct product*) shmat(shmid, NULL, SHM_RND);
	ram = (struct product*) shmat(ram_id, NULL, SHM_RND);

//Setting STOP flag to 1, signals other processes to stop
	STOP = 1;

//unblocking any process still blocked on buffer or ram
	v(BUF_MUTEX, sem_id);
	v(FULL, sem_id);
	v(EMPTY, sem_id);
	v(RAM_MUTEX, sem_id);
	v(Q_MUTEX, sem_id);

	int i;
	int curPid = 0;

//Unblocking every producer process w/ requests still in RAM
	for (i = 0; i < ramsize; i++)
	{
		if (ram[i].pid != 0 && ram[i].pid != curPid)
			v(0, ram[i].semid);
	}	

//Removing shmemdata file and all shared memory
	remove("shmemdata");
	shmdt(shmem);
	shmctl(shmid, IPC_RMID, NULL);
	semctl(sem_id, 0, IPC_RMID, 0);
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

