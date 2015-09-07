//This program demonstrates the example in case of multiple writers
//and one reader.
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int fd[2];
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* reader(void* i){
	char ch;
	while(1){
	read(fd[0], &ch, 1);
	printf("Got input as : %c\n", ch);
	}
}

void* writerA(void* i){
	char ch = 'A';
	while(1){
	//pthread_mutex_lock(&mx);
	//pthread_cond_wait(&cond, &mx);
	write(fd[1], &ch, 1);
	++ch;
	//pthread_cond_signal(&cond);
	//pthread_mutex_unlock(&mx);
	}
}

void* writera(void* u){
	char ch = 'a';
	while(1){
	//pthread_mutex_lock(&mx);
	//pthread_cond_wait(&cond, &mx);
	write(fd[1], &ch, 1);
	++ch;
	//pthread_cond_signal(&cond);
	//pthread_mutex_unlock(&mx);
	}
}

//This program demonstrates the basic pipe funda
//where one program creates a pipe and another one
int main(){
int status = 0;
if( (status = pipe(fd)) < 0){
	printf("Error occurred");
	return 0;
}
pthread_t p1, p2, p3;

pthread_create(&p1, NULL, reader, NULL);
pthread_create(&p2, NULL, writerA, NULL);
pthread_create(&p3, NULL, writera, NULL);

pthread_join(p1, NULL);
pthread_join(p2, NULL);
pthread_join(p3, NULL);
return 0;
}


