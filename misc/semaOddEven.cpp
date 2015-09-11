//Sequential printing by threads
#include<iostream>
#include<semaphore.h>
#include<pthread.h>

using namespace std;

//global sema
sem_t sm[2];
int count = 1;


void* odd(void* uu){
   while(1){
	sem_wait(&sm[0]);
	cout<< " "<< count++ <<" "<<endl;
	sem_post(&sm[1]);
   }
}

void* even(void* uu){
    while(1){
	sem_wait(&sm[1]);
	cout<< " "<< count++ <<" "<<endl;
	sem_post(&sm[0]);
    }
}

int main(){
pthread_t thr[2];
void *status;

sem_init(&sm[0], 0, 1);
sem_init(&sm[1], 0, 0);


//create 3
pthread_create(&thr[0], NULL, odd, NULL);
pthread_create(&thr[1], NULL, even, NULL);

pthread_join(thr[0], &status);
pthread_join(thr[1], &status);

return 0;
}
