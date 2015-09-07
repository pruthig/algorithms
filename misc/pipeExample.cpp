#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>



//This program demonstrates the basic pipe funda
//where one program creates a pipe and another one
int main(){
int fd[2];
pid_t pd;
int status = 0;
if( (status = pipe(fd)) < 0){
	printf("Error occurred");
	return 0;
}
if( (pd = fork()) < 0 ) {
	printf("Error occurred\n");
	return 0;
}
if(pd > 0){ //parent
	char message[100];
	strcpy(message, "Hello Pipe");
	close(fd[0]);
	write(fd[1], message , sizeof(message));
	exit(0);
}
else if(pd == 0) { //child process
	char message[25];
	close(fd[1]);
	read(fd[0], message, sizeof(message));
	printf("Read string is : %s\n", message);
}
else{}
return 0;
}


