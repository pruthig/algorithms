/*
Given n non-negative integers representing an elevation map where the width of each bar is 1, 
compute how much water it is able to trap after raining.
Example :
Given [0,1,0,2,1,0,1,3,2,1,2,1], return 6.
*/

#include<iostream>
#include<stack>
#include<algorithm>
#include<cmath>
using namespace std;


int main() {
	vector<int> vec{ 0,1,0,2,1,0,1,3,2,1,2,1};
	stack<int> st{};
	int i = 0;
	int max = INT_MIN, count = 0;
	int size = vec.size();
	while(i < size-1 && vec[i] <= vec[i+1])
		++i;
	if(i == size-1)
		return 0;
		
	int max_n = vec[i];
	// It's all abut desc and inc.
	bool inc = false;
	bool dec = true;
	while(i < size-1) {
		if(vec[i+1] < vec[i]) {
			// dec. order
			if(inc) {
				int mn = min(max_n, vec[i]);
				while(!st.empty()) {
					count = count + abs(mn - st.top());
					st.pop();	
				}
				inc = false;
				max_n = vec[i];
			}
			st.push(vec[i]);
			++i;
		}
		else if(vec[i+1] > vec[i]){
			if(dec) {
				dec = false;
				inc = true;
			}
			st.push(vec[i]);
			++i;
		}
		else {
			st.push(vec[i]);
			++i;
		}
	}
	if(!st.empty()) {
		int top = st.top();
		st.pop();
		while(!st.empty()) {
			if(st.top() < top)
				count = count + abs(top - st.top());
			st.pop();	
		}
	}
	cout<<"Count is: "<<count<<endl;
	return 0;
}
