//Sequential printing by threads
#include<iostream>
#include<semaphore.h>
#include<pthread.h>

using namespace std;

//global sema
sem_t sm[3];


void* one(void* uu){
   while(1){
	sem_wait(&sm[0]);
	cout<<"  1 "<<endl;
	sem_post(&sm[1]);
   }
}

void* two(void* uu){
    while(1){
	sem_wait(&sm[1]);
	cout<<"  2 "<<endl;
	sem_post(&sm[2]);
    }
}

void* three(void* uu){
  while(1){
	sem_wait(&sm[2]);
	cout<<"  3 "<<endl;
	sem_post(&sm[0]);
  }
}


int main(){
pthread_t thr[3];
void *status;

sem_init(&sm[0], 0, 1);
sem_init(&sm[1], 0, 0);
sem_init(&sm[2], 0, 0);


//create 3
pthread_create(&thr[0], NULL, one, NULL);
pthread_create(&thr[1], NULL, two, NULL);
pthread_create(&thr[2], NULL, three, NULL);

pthread_join(thr[0], &status);
pthread_join(thr[1], &status);
pthread_join(thr[1], &status);

return 0;
}
