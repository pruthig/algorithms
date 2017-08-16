#include<iostream>
#include<vector>

using namespace std;

int count_set_bits(int n1, int n2) {
	int x = n1^n2;
	int count = 0;
	while(x != 0) {
		++count;
		x &= (x-1);
	}
	return count;
}

int main() {
	vector<int> vec{ 1, 3, 5};
	int sum = 0;
	for(int i = 0; i < vec.size()-1; ++i) {
		for(int j = i+1; j < vec.size(); ++j) {
			sum = sum + count_set_bits(vec[i], vec[j])*2;
		}
	}
	cout<<"Sum is: "<<sum<<endl;
	return 0;
}
