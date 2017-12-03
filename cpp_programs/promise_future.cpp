// Here is the program that uses std::promise and std::future to pass value to function. In this struct, we promise to the 
// thread that we will pass a value to it sometime in future which the thread later neeeded.

#include<iostream>
#include<thread>
#include<future>
//#include<promise>

using namespace std;
// This function will return the sum of 2 integers, the first one we have passed explicitly
// and secone one we will be passed by us later on
int sum(future<int> f, int b) {
    int a = f.get();
    return a+b;
}

int main() {
    promise<int> p;
    future<int> f = p.get_future();
    future<int> fu = std::async(sum, std::move(f), 2);
    p.set_value(1);
    cout<<"Sum is: "<<fu.get()<<endl;
    return 0;
}
