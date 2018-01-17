//You are given an array of N integers, A1, A2 ,…, AN and an integer K. Return the of count of distinct numbers in all windows of size K.
/*A=[1, 2, 1, 3, 4, 3] and K = 3
All windows of size K are
[1, 2, 1], [2, 1, 3], [1, 3, 4], [3, 4, 3], So, we return an array [2, 3, 3, 2].
2 3 2 1
1 - 1
3 - 1
4 - 1
dis = 3
*/
#include<iostream>
#include<vector>
#include<map>

using namespace std;

vector<int> window_distinct_count(vector<int>& input, int k) {
	vector<int> res{};
	map<int, int> mp{};
	int n = input.size();
	if(n < k) {
		return res;
	}
	
	// first add k to map
	for(int i = 0; i < k ; ++i) 
		if(mp.find(input[i]) == mp.end())
			mp[input[i]] = 1;
		else
			mp[input[i]]++; 

	// Add first distinct
	res.push_back(mp.size());
	// Now we are at k+1th position.. move 1 by 1
	int start = 0;
	for(int i = k; i < n; ++i) {
		// Check for start
		if(mp[input[start]] > 1) 
			--mp[input[start]];
		else 
			mp.erase(input[start]);
	
		++start;
		// Check for k
		if(mp.find(input[i]) != mp.end()) 
			++mp[input[i]];
		else
			mp[input[i]] = 1;
		res.push_back(mp.size());
	}
	return res;

}

int main() {
	vector<int> input { 1, 2, 1, 3, 4, 3};
	vector<int> res = window_distinct_count(input, 4);
	for(auto a : res)
		cout<<a<<" ";
	cout<<endl;
	return 0;
}
