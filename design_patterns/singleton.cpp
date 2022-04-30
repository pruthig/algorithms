//creating singleton pattern file
#include<iostream>
#include<pthread.h>



class single{
private:
    single operator=(const single&);
    single(const single& s){}
    single(){}
public:
    static single *instance;
    static pthread_mutex_t mx;
    static single* getInstance();
};

// ########  For c++11, following will work
class Singleton {
    private:
        Singleton() = default;
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
public:
        static Singleton& getSingleton() {
            static Singleton single;
            return single;
        }
};
// #########  End c++11 example

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
	
