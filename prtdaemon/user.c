#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>
//Adrian Faircloth
//3-22-22
//CSC460
//Beasty Print Daemon: User

//Creates file containing random quote, calls daemon to print quote 5 times & syncs w/ sems and shared memory


//Defines for sems
#define MUTEX 0
#define FULL 1
#define EMPTY 2

//Defines for shmem and buffer size
#define SIZE size[0]
#define FRONT shmem[SIZE].id
#define REAR shmem[SIZE + 1].id
#define STOP shmem[SIZE + 2].id
//struct for shared memory, contains PID and filename of file w/ random quote
struct usrData {
	int id;
	char filename[15];
};

main()
{
	struct usrData *shmem;
	int sem_id, shmid;
	int size[1];
	int randVal;
	FILE *fp;
	char filename[] = "myfile_";

//Opening shmemdata and reading in sem ID, shmem ID, and buffer size
	if ((fp = fopen("shmemdata", "r")) == NULL)
	{
		printf("Failed to open shmemdata to read.\n");
		return(0);
	}

	fscanf(fp, "%d %d %d", &sem_id, &shmid, &size[0]);
	fclose(fp);

//Attatching to shared memory	
	shmem = (struct usrData *) shmat(shmid, NULL, SHM_RND);

//Appending PID to filename, result is 'myfile_<PID>'
	sprintf(filename, "%s%d", filename, getpid());


//Creating user file
	if ((fp = fopen(filename, "w")) == NULL)
	{
		printf("Failed to create user file.\n");
		return(0);
	}

//Building bash command to get random quote and insert into user file
	char syscall[] = "curl -s http://api.quotable.io/random | cut -d\":\" -f4 | cut -d\"\\\"\" -f2 > ";
	sprintf(syscall, "%s%s", syscall, filename);

//Performing system call for bash command
	system(syscall);

//Rebuilding filename, system call cuts off 'myfile_' for some reason
	sprintf(filename, "myfile_%d", getpid());

//Loop for signalling daemon to print quote from file 5 times
	int i;
	for (i = 0; i < 5; i++)
	{
	//'Working' for a random amount of seconds b/w 1 and 3
		randVal = (srand(getpid()) % 3) + 1;
		printf("User %d is working for %d seconds...\n", getpid(), randVal);
		sleep(randVal);

	//Syncing w/ daemon and other users 
		p(EMPTY, sem_id);
	//breaking loop if system is stopped while user is running
		if (STOP !=0) break;	
		p(MUTEX, sem_id);
	//Critical Section
	//adds user's PID and filename to buffer and Vs on FULL to allow daemon to access buffer and print quote
		printf("User %d is printing %s\n", getpid(), filename);	

		shmem[REAR].id = getpid();
		strcpy(shmem[REAR].filename, filename);

	//Incrementing tracker for rear of buffer
		REAR = (REAR + 1) % SIZE;
		
		v(MUTEX, sem_id);
		v(FULL, sem_id);
	
	}	

//Printing completion message and exiting
	printf("User %d is logging off.\n", getpid());
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





         
