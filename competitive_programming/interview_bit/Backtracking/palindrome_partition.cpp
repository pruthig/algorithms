/*
Given a string s, partition s such that every string of the partition is a palindrome. Return all possible palindrome partitioning of s.
For example, given s = "aab",
Return
  [
    ["a","a","b"]
    ["aa","b"],
  ]
*/
#include<iostream>
#include<string>
#include<vector>

using namespace std;

namespace {
	vector<vector<string>> res{};
}

bool is_palindrome(string input) {
	if (input == string(input.rbegin(), input.rend())) 
    	return true;
    return false;
}

void func(int i, string input, vector<string> tmp) {
	if(i == input.size()) {
		res.push_back(tmp);
		tmp.pop_back();
		return;
	}
	
	for(int j = i; j < input.size(); ++j) {
		if(is_palindrome(input.substr(i, j-i+1))) {
			tmp.push_back(input.substr(i, j-i+1));
			func(j+1, input, tmp);
			tmp.pop_back();
		}
	}
}

int main() {
	string input = "a";
	if(input.size() == 0)
		return 0;
	vector<string> tmp{};
	func(0, input, tmp);
	// printing results
	for(auto a : res) {
		for(auto b : a)
			cout<<b<<" ";
		cout<<endl;
	}
	
	return 0;
}
