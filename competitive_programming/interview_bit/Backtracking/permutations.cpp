// This program generates all permutations of a given string
#include<iostream>
#include<vector>
#include<string>
#include<algorithm>

using namespace std;

namespace {
	vector<vector<int>> res{};
}

void func(int i, vector<int> input) {
	if(i == input.size()) {
		res.push_back(input);
		return;
	}
		
		
	for(int j = i; j < input.size(); ++j) {
		swap(input[j], input[i]);
		func(i+1, input);
		swap(input[j], input[i]);
	}
}

int main() {
	vector<int> input{1, 2, 3};
	res.clear();
	//string tmp = "";
	func(0, input);
	
	// Printing results
	for(auto a: res) {
		for(auto b : a) {
			cout<<b<<" ";
		}
		cout<<endl;
	}
	return 0;
}
