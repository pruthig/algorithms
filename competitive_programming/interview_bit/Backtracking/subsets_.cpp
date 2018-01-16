#include<iostream>
#include<algorithm>
#include<vector>

using namespace std;

namespace {
	vector<vector<int> > res{};
}

void func(int index, vector<int> tmp, vector<int> input) {
	if(index == input.size()) {
		res.push_back(tmp);
		tmp.pop_back();
		return;
	}
	// j = 0 -> 1
	for(int j = index; j < input.size(); ++j) {
		tmp.push_back(input[j]);
		func(++j, tmp, input);
		tmp.pop_back();
	}
}
int main() {
	res.clear();
	vector<int> t{};
	res.push_back(t);
	vector<int> input {1, 2};
	sort(input.begin(), input.end());
	func(0, t, input);
	
	for(auto a : res) {
		for(auto b : a) {
			cout<<b<<" ";
		}
		cout<<endl;
	}
	//func(0, input);
	return 0;
}
