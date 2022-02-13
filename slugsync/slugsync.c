#include <stdio.h>

//Adrian Faircloth
//02-13-22
//CSC460
//Sluggish Sync

//Uses busy wait method of process syncing to print n process IDs in order n times
main(int argc, char *argv[]) 
{
	//Checking if command-line arg is given
	if (argc == 1)
	{
		printf("No value given.\n");
		return(0);
	}

	int n = atoi(argv[1]);

	//Checking if command-line arg is in range 1 - 26
	if (n > 26 || n < 1)
	{
		printf("Value out of range.\n");
		return(0);
	}

	FILE *fopen(), *fptr;

	//Opening syncfile for writing value
	if ( (fptr = fopen("syncfile", "w")) == NULL)
	{
		printf("couldn't open syncfile to write.\n");
		return(0);
	}

	//Writing starting sync value into syncfile
	fprintf(fptr, "%d", 1);
	fclose(fptr);

	//Creating ID variables for checking and writing to syncfile
	int myID = 1;
	int nextID = 2;

	//*****  Creating n total processes w/ unique myID and nextID pointing to next process  *****
	int i;
	for (i = 1; i < n; i++)
	{
		if (fork() == 0)
		{
			myID++;
		
			//If current process is the nth process, set nextID to myID of first process
			if (myID == n)
			{
				nextID = 1;
			}
			else
				nextID++;
		}
		else
			break;
	}

	//Getting letter corresponding to process based on myID
	char letter = 64 + myID;

	//*****  Performing busy wait n times to print each process ID in order  *****
	int found;
	for (i = 0; i < n; i++)
	{
		found = -1;

		//***** Until myID is found in syncfile, wait for process' turn  *****
		while(found != myID)
		{
			if ( (fptr = fopen("syncfile", "r")) == NULL)
			{
				printf("Could not open syncfile to read.\n");
				return(0);
			}

			fscanf(fptr, "%d", &found);
			fclose(fptr);
		}

		//Printing process' letter and PID
		printf("%c: %d\n", letter, getpid());

		//*****  Writing nextID into syncfile to continue process printing sequence  *****
		if ( (fptr = fopen("syncfile", "w")) == NULL)
		{
			printf("Could not open syncfile to write.\n");
			return(0);
		}
		
		fprintf(fptr, "%d", nextID);
		fclose(fptr);
	}

	return(0);
}
