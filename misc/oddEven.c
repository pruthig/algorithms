#include<stdio.h>
#include<iostream>
#include<unistd.h>
#include<pthread.h>

using namespace std;

pthread_t t1;
pthread_t t2;

int count = 0;
void **p;
void* odd(void *u){
while(1) {
		sleep(1);
		if(count%2 == 1) {
				cout<<"Odd count : "<<count<<endl;
				++count;
	}
}
}

void* even(void*j){
while(1){
		sleep(1);
		if(count%2 == 0) {
				cout<<"Even count : "<<count<<endl;
				count++;
		}
}
}


int main(){
int status;
pthread_create(&t1, NULL, odd, NULL);
pthread_create(&t2, NULL, even, NULL);
pthread_join(t1, p);
pthread_join(t2, p);

return 0;
}
