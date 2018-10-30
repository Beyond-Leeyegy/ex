#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <cstring>
#include <signal.h>
#define BUFSIZE 100
int main(){
	int n,fd[2];
	pid_t pid1,pid2;
	char buffer[BUFSIZE+1];

	pipe(fd);
	
	if((pid1=fork())>0){
		if((pid2 = fork())>0){
		//parent
			close(fd[0]);
			write(STDOUT_FILENO,"\nparent write:hello world!\n",strlen("\nparent write:hello world!\n"));
			write(fd[1],"Hello world!",12);
		}else if(pid2 == 0){
		//child2
			n = read(fd[0],buffer,12);
			write(STDOUT_FILENO,"\nchild2 get:\n",strlen("\nchild1 get:\n"));
			write(STDOUT_FILENO,buffer,n);
			write(fd[1],buffer,n);
			exit(0);
		}
	}else if(pid1 == 0){
		//child 1
		n = read(fd[0],buffer,12);
		write(STDOUT_FILENO,"\nchild1 get:\n",strlen("\nchild1 get:\n"));
		write(STDOUT_FILENO,buffer,n);
		write(fd[1],buffer,n);
		exit(0);
	}
	exit(0);

}
