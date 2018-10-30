#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <stdlib.h>
int wait_mark;
void waiting();
void stop(int );
int main()
{
	int p1, p2;
	int status;
//	signal(SIGINT,stop);
	while((p1=fork())==-1);
	if(p1>0) /*在父进程中*/
	{

//	signal(SIGINT,stop);
		while((p2=fork())==-1);
		if(p2>0) /*在父进程中*/
		{
			
	signal(SIGINT,stop);
			wait_mark=1;
			waiting();
			kill(p1,10);
			kill(p2,12);
			wait(&status);
			wait(&status);
			printf("parent process is killed!\n");
			exit(0);
		}
		else /*在子进程 2 中*/
		{
			wait_mark=1;
			signal(12,stop);
			waiting();
			lockf(1,1,0);
			printf("child process 2 is killed by parent!\n");
			lockf(1,0,0);
			exit(0);
		}
	}
	else /*在子进程 1 中*/
	{
		wait_mark=1;
		signal(10,stop);
		waiting();
		lockf(1,1,0);
		printf("child process 1 is killed by parent!\n");
		lockf(1,0,0);
		exit(0);
	}
	return 0;
}

void waiting()
{
	while(wait_mark!=0);
}

void stop(int)
{
	wait_mark=0;
}
