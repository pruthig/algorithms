#include<iostream>

using namespace std;

int main() {
	unsigned int x = 11;
	int count = 0;
	
	while(x != 0) {
		++count;
		x = x&(x-1);
	}
	cout<<"Value is: "<<count<<endl;
	return 0;
}
