#include<iostream>
#include<algorithm>
#include<iterator>
#include<vector>
#include<functional>
using namespace std;



int main() {
	int init = 0;
	vector<int> v{ 1, 2, 3, 4 };
	
	cout<<"Sum of elements is: "<<accumulate(v.begin(), v.end(), init)<<endl;
	init = 100;
	cout<<"Sum with custom function is: "<<
	accumulate(v.begin(), v.end(), init, [](int i, int j) 
	{ 
		return i+j;
	});
	return 0;
}

