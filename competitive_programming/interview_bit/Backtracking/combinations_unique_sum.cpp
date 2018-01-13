/* Given a set of candidate numbers (C) and a target number (T), find all unique combinations in C where the candidate numbers sums to T.
*/

#include<iostream>
#include<vector>
#include<algorithm>

using namespace std;

namespace {
	vector<vector<int>> res{};
}
	
void func(int index, vector<int> tmp, int k, vector<int>& input, int sum) {
	if(sum >= k) {
        if(sum == k)
    		res.push_back(tmp);
		return;
	}

	for(int j = index; j < input.size() ; ++j) {
        if(index == 0 && j>0 && input[j] == input[j-1])
          continue;
		tmp.push_back(input[j]);
		func(j+1, tmp, k, input, sum+input[j]);
		tmp.pop_back();
	}
}

int main() {
    int k = 0;
	res.clear();
	vector<int> input {10,1,2,7,6,1,5};
    sort(input.begin(), input.end());
    cout<<"Vector is: ";
    for(auto t : input)
        cout<<t<<" ";
    cout<<"\n";
	vector<int> tmp{};
	cout<<"Enter sum:\n";
	cin>>k;
	func(0, tmp, k, input, 0);
	
	for(auto a : res) {
		for(auto b : a) {
			cout<<b<<" ";
		}
		cout<<endl;
	}
		
	return 0;
}
	

