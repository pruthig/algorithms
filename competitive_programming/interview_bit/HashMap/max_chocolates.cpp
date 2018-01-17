/*
Given N bags, each bag contains Ai chocolates. There is a kid and a magician. In one unit of time, kid chooses a random bag i, eats Ai chocolates, then the magician fills the ith bag with floor(Ai/2) chocolates.
Given Ai for 1 <= i <= N, find the maximum number of chocolates kid can eat in K units of time. e.g.
K = 3, N = 2, A = 6 5
Return: 14
*/
// We will use heap to arrive at a solution
#include<iostream>
#include<vector>
#include<algorithm>
#include<cmath>

using namespace std;

int func(vector<int>& A, int k) {
	if(A.size() == 0)
		return 0;
	vector<int> input{A};
	make_heap(input.begin(), input.end());
	long long count = 0;
	// Do for k operations
	for(int i = 0; i < k; ++i) {
		//pop_max
		int mx = input.front();
		
		pop_heap(input.begin(), input.end());
		input.pop_back();
		// push half
		input.push_back(floor(mx/2));
		push_heap(input.begin(), input.end());
		count += mx;
		count %= 1000000007;
	}
	return count;
}

int main() {
	vector<int> vec{ 6, 5 };
	int res = func(vec, 1);
	cout<<"Max child can eat is: "<<res<<endl;
	return 0;
}
