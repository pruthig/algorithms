#include<iostream>
#include<exception>
#include<vector>
#include<condition_variable>
#include<thread>
#include<atomic>
#include<chrono>
#include<string>

using namespace std;

namespace {
	condition_variable con;
	vector<thread> vec_threads;
	mutex m;
	int num_threads = 0;
	int freq_char = 0;
	string str = "";
	atomic<int> ready(-1);
	int index = 0;
};

void print(int id) {
	while(1) {
		unique_lock<mutex> lck(m);
		while(ready != id)
			con.wait(lck);
		cout<<"Thread ID: "<<id<<"  ";
		for(int j = index; j < index+freq_char; ++j) {
			cout<<str[j%(str.size())];
		}
		cout<<endl;
		index += freq_char;
		ready = (++ready)%num_threads;
		lck.unlock();
		con.notify_all();
		this_thread::sleep_for(chrono::milliseconds(4000));
	}
}

int main(int argc, char* argv[]) {
	
	if (argc != 4) {
		cout<<"Please enter the following parameters in sequence: <String> <Character count> <Number of Threads>";
		return -1;
	}
	str = argv[1];
	try {
		freq_char = stoi(argv[2]);
		num_threads = stoi(argv[3]);
	}
	catch(exception e) {
		cout<<"Either invalid number of threads or invalid character count has been supplied.";
		return -1;
	}
	
	for(int i = 0; i < num_threads; ++i) {
		vec_threads.push_back(thread(print, i));
	}
	// Notify the 1st thread
	ready = 0;
	con.notify_all();
	for(auto &a : vec_threads) {
		a.join();
	}
	return 0;
}
