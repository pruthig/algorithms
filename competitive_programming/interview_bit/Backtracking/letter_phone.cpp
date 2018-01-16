/* Given a digit string, return all possible letter combinations that the number could represent.
A mapping of digit to letters (just like on the telephone buttons) is given below. */

#include<iostream>
#include<vector>

using namespace std;

namespace {
	vector<string> vec{};
	vector<string> A { "0", "1", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};
}

void func(string input, int i, string partial) {
	if(i == input.size()) {
		vec.push_back(partial);
		return;
	}
	// str is clubbed string from array
	string str = A[ input[i]-'0' ];
	for(int j = 0; j < str.size() ;++j) {
		func(input, i+1, (partial+str[j]));
	}
}
int main() {
	string input = "23";
	func(input, 0, "");
	for(auto a : vec) {
		cout<<a<<" ";
	}
	return 0;
}
