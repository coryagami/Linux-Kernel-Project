#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void start();
void stop();
void issue(char, int, int);

int main()
{
	char wait;

	printf("press key to issue requests (with some bad ones): ");
	wait = getchar();

	issue('P', 3, 4);
	issue('A', 3, 4);
	issue('L', 3, 4);
	issue('L', 3, 4);
	issue('L', 3, 4);
	issue('A', 6, 4);
	issue('A', 2, 0);
	issue('A', -7, 4);
	issue('A', 2, -1);

	printf("press key to call start_shuttle(): ");
	wait = getchar();
	start();

	int i=0;
	char type = 'X';
	int init;
	int dest;

	printf("press key to issue 100 random requests: ");
	wait = getchar();
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

	printf("press key to call stop_shuttle(): ");
	wait = getchar();
	stop();

	printf("press key to call issue another request: ");
	wait = getchar();
	issue('A', 1, 1);

	printf("press key to call start_shuttle(): ");
	wait = getchar();
	start();

	printf("press key to call stop_shuttle(): ");
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