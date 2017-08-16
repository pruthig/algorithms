#include<iostream>
#include<vector>

using namespace std;

int main() {
	vector<int> v{6, 7, 5, 4, 5, 3, 5, 2, 34, 5, 2,34, 6, 3,4 };
	unsigned int number = 0;
	
	for(auto a : v) {
		number ^= a;
	}
	cout<<"missing is: "<<number<<endl;
	return 0;
}
