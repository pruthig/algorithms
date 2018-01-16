#include<iostream>
#include<vector>

using namespace std;

namespace {
	vector<vector<int>> res{};
}

void func(int i, vector<int> tmp, vector<int> input) {
	if(i == input.size()) {
		return;
	}
	for(int j = i; j  < input.size(); ++j) {
		tmp.push_back(input[j]);
		res.push_back(tmp);
		func(j+1, tmp, input);
		tmp.pop_back();
	}
}

int main() {
	vector<int> input{ 1, 2, 3 };
	sort(input.begin(), input.end());
	vector<int> tmp{};
	res.push_back(tmp);
	func(0, tmp, input);
	for(auto a : res) {
		for(auto b : a)
			cout<<b<<" ";
		cout<<endl;
	}
	return 0;
}
