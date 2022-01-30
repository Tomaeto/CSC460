#include <stdio.h>
//Adrian Faircloth
//CSC 460
//01-29-22
//Alpha^Alpha

//Creates N processes (1 - 26), prints corresponding letter and PID N times
//N passed in from command line

int main(int argv, char* argc[])
{
	//If no argument passed in, print message and die
	if (argv == 1)
	{
		printf("No value given\n");
		return 0;
	}

	//Getting N value from command line
	int n = atoi(argc[1]);

	//If N is out of range, print message and die
	if (n < 1 || n > 26)
	{
		printf("Input value out of range\n");
		return 0;
	}

	//Counters for forking loop and printing loop
	int i,j;

	//Counters for stalling code
	int k, f;

	//Var for random math calcs for stalling
	int nonsense;
	
	//Char for letter of each child process
	char chara;

	//Printing newline for formatting purposes
	printf("\n");

	//Parent process creates N processes that each prints chara and PID N times
	for (i = 0; i < n; i++)
	{
		//Incrementing character for each new process forked
		chara = 'A' + i;

		//Each child process prints info N times and stalls w/ random math
		if (fork() == 0)
		{
			for (j = 0; j < n; j++)
			{
				//Printing character and PID of child process
				printf("%c: %d\n", chara, getpid());

				//Performing lots of random math to stall print loop
				//Creates variety in order of output lines
				for(k = 0; k <= 1000; k++)
				{
					for (f = 0; f <= 10000; f++)
					{
						nonsense = 445857 * 115;
						nonsense *= 88;
						nonsense = nonsense % 13;
					}
				}
				
			}

			//Breaking loop so child processes do not run outer loop
			break;
		}
	}
}
