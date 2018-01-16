/* Given two integers n and k, return all possible combinations of k numbers out of 1 2 3 ... n.
Make sure the combinations are sorted. */
#include<iostream>
#include<vector>
#include<algorithm>

using namespace std;

namespace {
	vector<vector<int>> res{};
}
	
void func(int index, vector<int> tmp, int k, vector<int>& input) {
	if(tmp.size() == k) {
		res.push_back(tmp);
		return;
	}

	for(int j = index; j < input.size() ; ++j) {
		tmp.push_back(input[j]);
		func(j+1, tmp, k, input);
		tmp.pop_back();
	}
}

int main() {
    int k = 0;
	res.clear();
	vector<int> input { 1, 2, 3, 4};
    sort(input.begin(), input.end());
	vector<int> tmp{};
	cout<<"Enter k\n";
	cin>>k;
	func(0, tmp, k, input);
	
	for(auto a : res) {
		for(auto b : a) {
			cout<<b<<" ";
		}
		cout<<endl;
	}
		
	return 0;
}
	

