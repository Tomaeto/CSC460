#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
//Adrian Faircloth
//4-17-22
//CSC460
//Memory Manager 1: Consumer

//Creates all shared memory, syncs w/ producers and manages simulated RAM

//Defines for buffer, RAM, and queue semaphores
#define BUF_MUTEX 0
#define FULL 1
#define EMPTY 2
#define RAM_MUTEX 3
#define Q_MUTEX 4

//Defines for buffer and queue pointers, and stop flag
#define FRONT shmem[buf].size
#define REAR shmem[buf+1].size
#define STOP shmem[buf+2].size
#define QFRONT shmem[buf].pid
#define QREAR shmem[buf+1].pid

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
	int i;
	int sem_id, shmid;
	int ram_id, q_id;
	FILE *fp;
	struct product *shmem;
	struct product *ram;
	struct product *queue;

//Checking if shmemdata file exists to see if system is already active
	if ( (fp = fopen("shmemdata", "r")) != NULL)
	{
		printf("System already active.\n");
		return(0);
	}

//Checking for correct number of command-line args
	if (argc != 4)
	{
		printf("Incorrect command-line arguments.\n");
		return(0);
	}

//Getting row count from command line and checking range
	int row = atoi(argv[1]);
	if (row > 20 || row < 1)
	{
		printf("Row num out of range (1 - 20).\n");
		return(0);
	}		

//Getting col count from command line and checking range
	int col = atoi(argv[2]);
	if (col > 50 || col < 1)
	{
		printf("Column num out of range (1 - 50).\n");
		return(0);
	}

//Getting buffer size from command line and checking range
	int buf = atoi(argv[3]);
	if (buf > 26 || buf < 1)
	{
		printf("Buffer size out of range (1 - 26).\n");
		return(0);
	}

//Getting semaphores for buffer mutex, full, empty, ram mutex, and queue mutex
	sem_id = semget(IPC_PRIVATE, 5, 0777);
	if (sem_id == -1)
	{
		printf("Failed to get sems.\n");
		return(0);
	}
	
//Initalizing all semaphores
	semctl(sem_id, BUF_MUTEX, SETVAL, 1);
	semctl(sem_id, FULL, SETVAL, 0);
	semctl(sem_id, EMPTY, SETVAL, buf);
	semctl(sem_id, RAM_MUTEX, SETVAL, 1);
	semctl(sem_id, Q_MUTEX, SETVAL, 1);

//Getting and attatching to shared memory buffer, RAM, and queue
	shmid = shmget(IPC_PRIVATE, (buf+3)*sizeof(struct product), 0777);

	ram_id = shmget(IPC_PRIVATE, (row*col)*sizeof(struct product), 0777);

	q_id = shmget(IPC_PRIVATE, 30*sizeof(struct product), 0777);

	if (shmid == -1 || ram_id == -1 || q_id == -1)
	{
		printf("Failed to get shmem.\n");
		return(0);
	}

	shmem = (struct product*) shmat(shmid, NULL, SHM_RND);
	ram = (struct product*) shmat(ram_id, NULL, SHM_RND);
	queue = (struct product*) shmat(q_id, NULL, SHM_RND);

//Initializing all RAM and queue entries w/ a PID of 0
	for (i = 0; i < row*col; i++)
	{
		ram[i].pid = 0;
	}

	for (i = 0; i < 30; i++)
	{
		queue[i].pid = 0;
	}

//Initializing all shmem pointers and STOP flag
	FRONT = 0;
	REAR = 0;
	STOP = 0;
	QFRONT = 0;
	QREAR = 0;

//Creating shmemdata file and writing ram size, buffer size, and shared memory id's
	if ((fp = fopen("shmemdata", "w+")) == NULL)
	{
		printf("Failed to open shmemdata.\n");
		return(0);
	}
	fprintf(fp, "%d %d %d %d %d", row*col, buf, sem_id, shmid, ram_id);
	fclose(fp);


//Forking processes to perform buffer sync, memory management, and RAM display
if (fork() == 0)
{
//Fork for bounded buffer sync
	char letter = 'A';
	int index;
	int numInsert = 0;
	while (STOP == 0)
	{
	//Checking for entry in buffer and getting mutex for buffer, RAM, and queue
		p(FULL, sem_id);
		p(BUF_MUTEX, sem_id);
		p(RAM_MUTEX, sem_id);
		p(Q_MUTEX, sem_id);
	//breaking loop if STOP flag is set
		if (STOP != 0) break;
	//Checking if request from buffer can fit into RAM
		index = checkFit(ram, shmem[FRONT].size, row*col);

		if (index != -1)
		{
		//If request fits, insert data into each space in RAM
			while (numInsert < shmem[FRONT].size)
			{
				ram[index].pid = shmem[FRONT].pid;
				ram[index].semid = shmem[FRONT].semid;
				ram[index].size = shmem[FRONT].size;
				ram[index].time = shmem[FRONT].time;
				ram[index].letter = letter;
				ram[index].start = time(NULL);
				numInsert++;
				index++;
			}
			numInsert = 0;
		}

		else
		{
		//If request does not fit, insert data into queue
			queue[QFRONT].pid = shmem[FRONT].pid;
			queue[QFRONT].semid = shmem[FRONT].semid;
			queue[QFRONT].size = shmem[FRONT].size;
			queue[QFRONT].time = shmem[FRONT].time;
			queue[QFRONT].letter = letter;
		
			QFRONT = (QFRONT + 1) % 30;	
		}

	//Removing Mutex for queue, RAM, and buffer, and v on empty
		v(Q_MUTEX, sem_id);	
		v(RAM_MUTEX, sem_id);
		v(BUF_MUTEX, sem_id);
		v(EMPTY, sem_id);

	//Incrementing letter variable for next request
		if (letter == 'Z')
			letter = 'A';
		else
			letter++;
			
		
	}
}
else
{

if (fork() == 0)
{
//Fork for memory management and placement from queue
	int index;
	int numInsert = 0;
	int lastPid = 0;
while(STOP == 0)
{
//Getting mutex for RAM and queue
	p(RAM_MUTEX, sem_id);
	p(Q_MUTEX, sem_id);
//Breaking loop if STOP flag is set
	if (STOP != 0) break;	

//For each request in RAM, check if request time is done
	for (i = 0; i < row*col; i++)
	{
		if (ram[i].pid != 0 && time(NULL) - ram[i].start >= ram[i].time)
		{
		//If request time is done, v on producer semaphore
			if (lastPid != ram[i].pid)
				v(0, ram[i].semid);

		//Set RAM space's PID to 0
			lastPid = ram[i].pid;
			ram[i].pid = 0;

		//Checking if request in queue can fit into RAM
			if (queue[QREAR].pid != 0)
			{
			//If queue contains a request, check if it can fit
				index = checkFit(ram, queue[QREAR].size, row*col);
				if (index != -1)
				{
				//If request fits into RAM, insert into each space of RAM needed
					while (numInsert < queue[QREAR].size)
					{
						ram[index].pid = queue[QREAR].pid;
						ram[index].semid = queue[QREAR].semid;
						ram[index].size = queue[QREAR].size;
						ram[index].time = queue[QREAR].time;
						ram[index].letter = queue[REAR].letter;
						ram[index].start = time(NULL);
						numInsert++;
						index++;
					}
	
					numInsert = 0;
					QREAR = (QREAR + 1) % 30;		
				}
			}
		}
	}

//Removing mutex for queue and RAM
	v(Q_MUTEX, sem_id);
	v(RAM_MUTEX, sem_id);
}
//After breaking loop, v on all producer semaphores in queue and remove queue shared memory
	for (i = 0; i < 30; i++)
	{
		if (queue[i].pid != 0)
			v(0, queue[i].semid);
	}
	shmctl(q_id, 0, IPC_RMID, 0);
}
else
{
//Fork for displaying RAM and current requests
	int currPid;
	int index;
	int j;

	while(STOP == 0)
	{
	//Getting mutex for RAM and queue
		p(RAM_MUTEX, sem_id);
		p(Q_MUTEX, sem_id);

	//Waiting one second
		sleep(1);

	//Printing info for each process either in RAM or in queue
		printf("ID thePID Size Sec\n");

	//Checking each process in RAM, printing info when first filled space for each request is found
		for (i = 0; i < row*col; i++)
		{
			if (ram[i].pid != 0 && currPid != ram[i].pid)
			{
				currPid = ram[i].pid;
				printf("%c: %d %d %d\n", ram[i].letter, ram[i].pid, ram[i].size, ram[i].time);
			}
		}

	//Checking each process in Queue, printing info for each process waiting to enter RAM
		for (i = 0; i < 30; i++)
		{
			if (queue[i].pid != 0 && currPid != queue[i].pid)
			{
				currPid = queue[i].pid;
				printf("%c: %d %d %d\n", queue[i].letter, queue[i].pid, queue[i].size, queue[i].time);
			}
		}

	//Printing current state of RAM
	//Prints RAM as a grid based on row and col counts
		printf("Memory Manager\n");
		printf("--------------\n");
		index = 0;
		for (i = 0; i < row; i++)
		{
			for (j = 0; j < col; j++)
			{
			//Printing '.' for each empty space and the request letter for each request in RAM	
				if (ram[index].pid == 0)
					printf(".");
				else
					printf("%c", ram[index].letter);
				index++;
			}
	
			printf("\n");
		}

	//Spacing out each display
		printf("\n\n");

	//Removing mutex for queue and RAM
		v(Q_MUTEX, sem_id);
		v(RAM_MUTEX, sem_id);
	}
}

}

}
//Function for checking if a process can fit into RAM
//Finds first fitting space
checkFit(struct product *ram, int prodsize, int ramsize)
{
	int index = 0;
	int offset = 1;

	//Checking each space in RAM to see if it is empty
	for (index = 0; index < ramsize; index++)
	{
		if (ram[index].pid == 0)
		{
		//If an empty space is found, check spaces in front up to product's requested size
			while (ram[index+offset].pid == 0 && index+offset < prodsize && index+offset < ramsize)
				offset++;

		//If all spaces checked are empty, return starting index
			if (index+offset == prodsize)
				return index;
		//If not all spaces were empty, return index of first filled space found
			else
				index = index+offset;
		}
	}

	//If no fitting space is found, return -1
	return -1;
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

