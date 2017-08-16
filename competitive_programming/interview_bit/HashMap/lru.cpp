#include<iostream>
#include<map>
#include<deque>
#include<string>
#include<algorithm>
using namespace std;

namespace {
	map<int, int> lru_map;
	deque<int> lru_queue;
	int size = 0;
}

void cleanser() {
	cout<<"Size of map is: "<<lru_map.size()<<endl;
	if(lru_map.size() == size) {
		int element = lru_queue.back();
		lru_queue.pop_back();
		lru_map.erase(element);
	}
}

int get(int key) {
    auto t = lru_map.find(key);
    if (t == lru_map.end()) {
    	cout<<"Key was not found\n";
		return -1;
	}
	
    int value = t->second;
    lru_queue.erase(std::remove(lru_queue.begin(), lru_queue.end(), key), lru_queue.end());
    lru_queue.push_front(key);
    cout<<"Element is: "<<value<<endl;
    return value;

}

void set(int key, int value) {
    int check = get(key);
    if(check != -1) {
        // Key already exists
        lru_queue.erase(std::remove(lru_queue.begin(), lru_queue.end(), key), lru_queue.end());
    }
    else if(lru_map.size() == size)
	    cleanser();
	else{}
	lru_map[key] = value;
	lru_queue.push_front(key);
	cout<<"Key updated\n";
}

void insert() {
	int key = 0, value = 0;
	cleanser();
	cout<<"Enter key, value"<<endl;
	cin>>key>>value;
	set(key, value);

	cout<<endl<<"Queue after insertion is: ";
	for(auto a : lru_queue)
		cout<<a<<" ";
	cout<<endl;
}	
	
int main() {

	cout<<"Enter the size of LRU\n";
	cin>>size;
	lru_map.clear();
	lru_queue.clear();
	while(1){
		int choice;
		cout<<"Enter 1 for get and 2 for insert\n";
		cin>>choice;
		if(choice == 1) {
			int element;
			cout<<"Enter the element to retrieve: ";
			cin>>element;
			get(element);
		}
		else if(choice == 2)
			insert();
		else {
			cout<<"Try again\n";
		}
	}

	return 0;
}

