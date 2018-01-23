/*
Given an array of integers, find the length of longest subsequence which is first increasing then decreasing.
**Example: **
For the given array [1 11 2 10 4 5 2 1]
Longest subsequence is [1 2 10 4 2 1]
Return value 6
*/

//1 11 2 10 4 5 2 1
//1  2 2  3 3 4 2 0

//1 2 5 4 10 2 11 1
//1 2 3 3  4 2    3 3 4 2 0 

#include<iostream>
#include<vector>
#include<algorithm>

using namespace std;

int main() {
	vector<int> input{1, 3, 5, 6, 4, 8, 4, 3, 2, 1};
	if(input.size() == 0)
		return 0;
	vector<int> l(input.size(), 1), r(input.size(), 1);
	
	l[0] = 1;
	//first traversal
	for(int i = 1; i < input.size(); ++i) {
		for(int j = i-1; j >= 0; --j) {
			if(input[j] < input[i] && l[j] +1 > l[i])
				l[i] = l[j]+1;
		}
	}
	reverse(input.begin(), input.end());
	// second traversal
	r[0] = 1;
	for(int i = 1; i < input.size(); ++i) {
		for(int j = i-1; j >= 0; --j) {
			if(input[j] < input[i] && r[j] +1 > r[i])
				r[i] = r[j]+1;
		}
	}
	reverse(r.begin(), r.end());
	// both traversal
	int mx = INT_MIN;
	for(int i = 0; i < input.size(); ++i) {
		if(l[i]+r[i]-1 > mx)
			mx = l[i]+r[i]-1;
	}
	cout<<"Size is: "<<mx<<endl;
	
	
}
