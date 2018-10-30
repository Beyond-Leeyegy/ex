#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAXSIZE 1024

int main(int arg_num ,char* arg[]){

	//error handling : omit arg
	if(arg_num != 3){printf("incorrect call format\n");return 1;}

	FILE* src = fopen(arg[1],"r");
	FILE* des = fopen(arg[2],"w");
	
	//src doesnot exist
	if(src == NULL){printf("file not exist\n");return 1;}


	//buffer
	char buffer[MAXSIZE];

	//memory to store the whole file
	fseek(src,0,SEEK_END);
	char *str = (char *) malloc( ftell(src));
	str[0] = 0;
	rewind(src);

	//get content
	while(fgets(buffer,MAXSIZE,src)!=NULL){
		strcat(str,buffer);
	}
	
	//write content
	fwrite(str,strlen(str),1,des);

	//close
	fclose(src);
	fclose(des);

	return 0;	
}
