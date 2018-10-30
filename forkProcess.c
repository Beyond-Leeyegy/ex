#include<stdio.h>
#include <unistd.h>
#include <sys/types.h>

main()
{ 
	int p1, p2;
	while((p1=fork())== -1);
	if(p1==0){
		putchar('b'); /*在子进程 1 中*/
//		putchar('\n');
	}
	else /*在父进程中*/
	{
		 while((p2=fork())==-1);
		if(p2==0){
			putchar('c'); /*在子进程 2 中*/
//			putchar('\n');
		}
		else /*在父进程中*/
			putchar('a');	
//			putchar('\n');
	}
}
