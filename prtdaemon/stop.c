#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
//Adrian Faircloth
//3-22-22
//CSC460
//Beasty Print Daemon: Stop

//Signals daemon to stop and clean up sems, shared mem, and file w/ IDs

//Defines for FULL sem and STOP value in shared memory
#define FULL 1
#define STOP shmem[size+2].id

//Struct for shared memory
struct usrData{
	int id;
	char filename[15];
};

main()
{
	int sem_id, shmid;
	struct usrData *shmem;
	int size;
	FILE *fp;

//Opening shmemdata file and reading in sem ID, shmem ID, and buffer size	
	if ((fp = fopen("shmemdata", "r")) == NULL)
	{
		printf("Failed to open shmemdata.\n");
		return(0);
	}
	
	fscanf(fp, "%d %d %d", &sem_id, &shmid, &size);

//Attatching to shared memory
	shmem = (struct usrData*) shmat(shmid, NULL, SHM_RND);

//Setting stop var in shared memory to 1, signals daemon to break infinite loop and continue to reset shared resources
	STOP = 1;

//V on FULL to unblock daemon if buffer is empty
	v(FULL, sem_id);
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


               
