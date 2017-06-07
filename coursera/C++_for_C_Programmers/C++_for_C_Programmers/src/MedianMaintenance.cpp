// This program will calculate sum of median maintenance of incoming stream of numbers
// stored in 2 heaps. Two heaps will be used: min-heap and max-heap for this

#include<queue>
#include<iostream>
#include<string>
#include<fstream>


using namespace std;

// Create 2 priority queues, by default priority queue is max-heap
priority_queue<int, vector<int>> low_heap;   // We will take max of this
priority_queue<int, vector<int>, std::greater<int> > high_heap;  // We will take min of this


void median_maintenance_main() {

	string str;
	int index = 0, counter = 0;
	std::ifstream infile("resources/Median.txt");
	long total_median = 0;
	std::string line;
	int number = 0;
	
	while (getline(infile, line))
	{
		number = stoi(line);
		if((low_heap.size() > 0 && number < low_heap.top()) || (high_heap.size() >0 && low_heap.size() >0 && number>low_heap.top() &&  number<high_heap.top()))
			low_heap.push(number);
		else
			high_heap.push(number);

		if(low_heap.size()-high_heap.size() == 2) {
			high_heap.push(low_heap.top());
			low_heap.pop();
		}
		else if(high_heap.size()-low_heap.size() == 2) {
			low_heap.push(high_heap.top());
			high_heap.pop();
		}
		else {
			/* do nothing */
		}

		// Add to total median. If size of low and high heap is equal then median is top element of max heap
		if(low_heap.size() > high_heap.size())
			total_median += low_heap.top();
		else if(high_heap.size() > low_heap.size())
			total_median += high_heap.top();
		else
			total_median += low_heap.top();
	}
	
	cout<<"Median  is : "<<total_median%10000<<endl;
	cin.get();
	return;
}