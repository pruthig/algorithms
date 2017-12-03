// This program shows you the sampe of using async() and future in C++11 programming
// This combination is basically used to get the value from thread at some later date...
// here, we have a program that returns sum of 2 integers;
#include<future>
#include<iostream>

using namespace std;

int sum(int a, int b) {
    return a+b;
}

int main() {

    future<int> f = std::async(std::launch::async, sum, 1, 2);
    // get the value
    cout<<"sum of 2 integers: "<<f.get()<<endl;
    return 0;
}

