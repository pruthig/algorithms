//creating singleton pattern file
#include<iostream>
#include<pthread.h>



class single{
private:
    single operator=(single&);
    single(single& s){}
    single(){}
public:
    static single *instance;
    static pthread_mutex_t mx;
    static single* getInstance();
};

// For c++11, following will work
static single& getInstance_CPP11() {
    static single singleInstance;
    return singleInstance;
}
};


pthread_mutex_t single::mx = PTHREAD_MUTEX_INITIALIZER;
single* single::instance = NULL;

single* single::getInstance(){
if(!instance){
	//lck.lock();
	pthread_mutex_lock(&mx);
	if(!instance)
		instance = new single();
	//lck.unlock();
	pthread_mutex_unlock(&mx);
}
return instance;
}


int main(){

single *s = single::getInstance();
return 0;
}
	
