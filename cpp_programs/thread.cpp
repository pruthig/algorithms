#include<iostream>
#include<cstdlib>
#include<cstdio>
#include<pthread.h>
#include<unistd.h>

using namespace std;

bool even = false;
int count = 1;
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_init(&mx)

void* fun1(void *l){
while(count<100){
while(even);
cout<<"odd : "<<count<<endl;
++count;
even = true;
}
}

void* fun2(void *p){
while(count<100){
while(!even);
cout<<"Even : "<<count<<endl;
++count;
even = false;
}
}


int main(){
int r1;
int r2;
pthread_t p1;
pthread_t p2;
pthread_create(&p1, NULL, fun1, NULL);
pthread_create(&p2, NULL, fun2, NULL);

pthread_join(p1, NULL);
pthread_join(p2, NULL);
return 0;
return 0;
}




