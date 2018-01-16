// This program finds kth permutation of a array containing elements from 1 to n 
// with 'n' and 'k' are provided as inputs

#include<iostream>
#include<string>
#include<algorithm>

using namespace std;

namespace {
	string res{};
}

int factorial(int n) {
  return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

void find_kth_permutation(vector<int> input, int n, int k) {
	if(k == 1 || k == 0) {
		// Add all;
		for(auto a : input)
			res += to_string(a);
		return;
	}
	int index = ceil( double(k) / factorial(n-1) ) - 1;
	res += to_string(input.at(index));
	input.erase(input.begin() + index);
	find_kth_permutation(input, n-1, (k%factorial(n-1)));
}
int main() {
	int n = 0, k = 0, count = 0;
	vector<int> input{};
	cout<<"Enter n and k respectively\n";
	cin>>n>>k;
	for(int i = 1; i <= n; ++i)
		input.push_back(i);
	res.clear();
	//string tmp = "";
	find_kth_permutation(input, n, k);
		
	cout<<"Result is: "<<res<<endl;
	
	return 0;
}
