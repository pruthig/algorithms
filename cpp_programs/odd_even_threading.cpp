#include<thread>
#include<iostream>
#include<condition_variable>
#include<atomic>
#include<mutex>
#include<chrono>

using namespace std;

namespace {
	/* atomic<int> */ int ready{-1};
	mutex m;
	condition_variable cv;
	int data = 0;
}

void print(int id) {
	while(1) {
		unique_lock<mutex> lck(m);
		while(ready == -1 || ready%2 != id) {
			cv.wait(lck);
		}
		++ready;
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		cout<<"thread Id is: "<<this_thread::get_id()<<" and data is: "<<ready<<endl;
		lck.unlock();
		cv.notify_all();
	}
}

int main() {
	thread t1{print, 0};
	thread t2{print ,1};
	ready = 0;
	cv.notify_all();
	t1.join();
	t2.join();
	return 0;
}
