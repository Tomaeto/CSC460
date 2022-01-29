#include <stdio.h>
//Adrian Faircloth
//CSC 460
//1-28-22
//Project 1: Fork in the Road

//Creates tree of processes w/ n generations
//n passed in on command line
//Each parent forks n times, each child forks n - 1 times

int main(int argc, char* argv[])
{
	//If no argument was passed in, print message and exit
	if (argc == 1)
	{
		printf("No input value\n");
		return 0;
	}

	//Getting argument for gen count from command line
	int n = atoi(argv[1]);

	//If argument is out of range, print message and exit
	if (n > 5 || n < 0)
	{
		printf("Input value out of range\n");
		return 0;
	}	

	//Printing header
	printf("Gen\tPID\tPPID\n");

	//Setting counters for fork loop and gen of process
	int i = n;
	int gen = 0;

	//Loop for creating child processes
	//Parent will create N processes
	//Child will increment gen counter and create N-1 processes
	while (i > 0)
	{
		//Child
		if (fork() == 0)
		{
			//Decrementing n and resetting i so child process
			//	will create n-1 processes
			gen++;
			n--;
			i = n;
		}
		//Parent
		else
			//Decrementing loop variable
			i--;
	}
	
	//Printing info for current process
	printf("%d\t%d\t%d\n", gen, getpid(), getppid());

	//Sleep to keep parent processes alive
	sleep(1);
}
