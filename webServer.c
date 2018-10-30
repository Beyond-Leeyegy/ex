#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

void header(FILE*fp, char * content_type){
	fprintf(fp,"HTTP/1.0 200 OK\r\n");
	if(content_type)
		fprintf(fp,"Content-type:%s\r\n",content_type);
}


void read_til_crnl(FILE *fp){
	char buf[BUFSIZ];
	while(fgets(buf,BUFSIZ,fp)!= NULL &&  strcmp(buf,"\r\n")!=0)
		;
}


void do_404(char*item,int fd){
	FILE *fp = fdopen(fd,"w");
	
	fprintf(fp,"HTTP/1.0 404 Not Found\r\n");
	fprintf(fp,"Content-type:text/plain\r\n");
	fprintf(fp,"\r\n");

	fprintf(fp,"resource%s not found\r\n",item);
	fclose(fp);
}

int not_exist(char *f){
	struct stat info;
	return (stat(f,&info) == -1);
}

int isadir(char *f){
	struct stat info;
//	printf(S_ISDIR(info.st_mode));
	return( stat(f,&info)!= -1 && S_ISDIR(info.st_mode));
}


void do_ls(char* request , int fd){

//	FILE *tmp = fopen("do_ls.txt","w");
	int f = open("do_ls.txt", O_WRONLY);
//	fwrite("do_ls",5,1,tmp);
//	fclose(tmp);

	FILE *fp;
	
	
	fp = fdopen(fd,"w");
	header(fp,"text/plain");
	fprintf(fp,"\r\n");
	fflush(fp);
	/*direction from 1 and 2*/
	fprintf(fd,request);
	
	
	close(1);
	dup(f);
	close(f);

	printf("try a try ");
//	dup2(f,1);
//	dup2(fd,2);
//	close(fd);
	
	execlp("ls","ls","-l",request,NULL);
	perror(request);
	exit(1);
}

char*file_type(char *f){
	char *cp;
	if((cp = strrchr(f,'.'))!= NULL)
		return cp+1;
	return "";
}

int ends_in_cgi(char *f){
	return (strcmp(file_type(f),"cgi")==0);

}

void do_exec(char * prog,int fd)
{
	FILE *fp = fdopen(fd,"w");

	header(fp,NULL);
	fprintf(fp,"\r\n");
	fflush(fp);
	
	dup2(fd,1);
	dup2(fd,2);
	close(fd);
	
	execlp(prog,prog,NULL);
	perror(prog);
	fclose(fp);
	
}

void do_cat(char * f,int fd)
{
	char*extension = file_type(f);
	char* content = "text/plain";
	FILE * fpsock,*fpfile;
	int c;
	

	/*to judge which type */
	if(strcmp(file_type(f),"html")==0)
		content = "text/html";
	else if (strcmp(file_type(f),"gif") == 0)
		content = "image/gif";
        else if (strcmp(file_type(f),"jpg") == 0)
                content = "image/jpeg";
        else if (strcmp(file_type(f),"jpeg") == 0)
                content = "image/jpeg";

	fpsock = fdopen(fd,"w");
	fpfile = fopen(f,"r");

	if(fpsock != NULL && fpfile != NULL){
		//fprintf(fpsock,content);

		header(fpsock,content);	
		fprintf(fpsock,"\r\n");
		
		while((c = getc(fpfile))!= EOF)
			putc(c,fpsock);
		fclose(fpfile);
		fclose(fpsock);
	}
	exit(0);

}


void cannot_do(int fd){
	FILE *fp = fdopen(fd,"w");


	fprintf(fp,"HTTP/1.0 501 Not Implemented\r\n");
	fprintf(fp,"content-type:text/plain\r\n");
	fprintf(fp,"\r\n");

	fprintf(fp,"only GET is acceptable! \r\n");
	fclose(fp);
}


char * process_rq(char* request,int fd){

	char cmd[BUFSIZ];
	char arg[BUFSIZ];

	if(fork()!=0){
		return;
	}
	//arg = arg+1;
	strcpy(arg,"./");
	if(sscanf(request,"%s%s",cmd,arg+1) != 2)
		return ;
//	arg = arg+1;
///	arg = strcat(".",arg);

//	FILE* temp = fopen("list.txt","w");
//	fprintf(temp,arg);
//	fclose(temp);
	
	if(isadir(arg)){                FILE* fp = fopen("log.txt","w");
                fwrite(arg,strlen(arg),1,fp);
                fclose(fp);
                do_ls(arg,fd);
}	

	if(strcmp(cmd,"GET")!=0)
		cannot_do(fd);
	else if(not_exist(arg))
		do_404(arg,fd);	
	else if(isadir(arg)){
	        FILE* fp = fopen("log.txt","w");
                fwrite("dir",3,1,fp);
                fclose(fp);
		do_ls(arg,fd);
	}
	else if(ends_in_cgi(arg)){
		FILE* fp = fopen("log.txt","w");
		fwrite("cgi",3,1,fp);
		fclose(fp);
		do_exec(arg,fd);}
	else {
		FILE* fp = fopen("log.txt","w");
                fwrite("cat",3,1,fp);
                fclose(fp);

		do_cat(arg,fd);
	}
	return arg;

}


int main(int ac,char* av[]){
	int n;
	int sock,fd;
	FILE *fpin;
	char request[BUFSIZ];


	if(ac == 1){
		fprintf(stderr,"usage: port number is needed");
		printf("port number is needed!");
		exit(1);
	}
	

	sock  = make_server_v2(atoi(av[1]));

	if(sock == -1){
		printf("you see");
		exit(2);
	}

	while(1){
		fd = accept(sock,NULL,NULL);
//		fpin = fdopen(fd,"r");
		
		/* read request */
//		fgets(request, BUFSIZ,fpin);
//		printf("got a call:request = %s",request);
//		read_til_crnl(fpin);
  //              printf("got a call:request = %s",request);		
//		printf("file:%s\n",process_rq(request,fd));


//		fclose(fpin);

		n = read(fd,request,sizeof(request));
		process_rq(request,fd);
		close(fd);
	}

	return 0;
}
