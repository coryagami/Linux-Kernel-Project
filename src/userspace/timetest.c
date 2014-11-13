#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void start();
void stop();
void issue(char, int, int);

int main()
{
	int i=0;
	char type = 'X';
	int init;
	int dest;
	char wait;

	for(i; i<100; i++)
	{
		int r = (rand() % (3))+1;
		if(r == 1)
			type = 'C';
		else if(r == 2)
			type = 'A';
		else if(r == 3)
			type = 'L';
		init = (rand() % 5)+1;
		dest = (rand() % 5)+1;
		issue(type, init, dest);
	}

	start();

	printf("press any key to stop:");
	wait = getchar();
	stop();

	return 0;
}

void start()
{
	printf("%d\n", syscall(357, "start_shuttle"));
}
void stop()
{
	printf("%d\n", syscall(358, "stop_shuttle"));
}
void issue(char p, int i, int d)
{
	printf("%d\n", syscall(359, p, i, d));
}