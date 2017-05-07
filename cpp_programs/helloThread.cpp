//Simple hello world program...Use pthread library to compile it by passing flag -lpthread
#include<iostream>
#include<thread>

using namespace std;

void hello(){
std::cout<<"Hello World"<<endl;
}

int main(){

std::thread t(hello);
t.join();
int p = std::thread::hardware_concurrency();
cout<<"Number of cores in system are : "<<p<<" and id is "<<std::this_thread::get_id()<<endl;
return 0;
}
