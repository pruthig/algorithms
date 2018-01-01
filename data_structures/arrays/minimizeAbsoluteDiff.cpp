// Given three sorted arrays A, B and C of not necessarily same sizes.
// Calculate the minimum absolute difference between the maximum and minimum number from the triplet a, b, c such that a, b, c 
// belongs arrays A, B, C respectively. i.e. minimize | max(a,b,c) - min(a,b,c) |.
#include<iostream>
#include<vector>

using namespace std;

int main() {
	vector<int> A { 1, 4, 5, 8, 10  };
	vector<int> B { 6, 9, 15 };
	vector<int> C { 2, 3, 6, 6 };
	int diff = INT_MAX;
	
	int i = 0, j = 0, k = 0;
	while(i < A.size() && j < B.size() && k < C.size()) {
		// Store the diff
		int mx = max (max(A[i], B[j]), C[k]);
		int mn = min (min(A[i], B[j]), C[k]);
		if(mx-mn < diff)
			diff = mx-mn;
		// check the max
		if(A[i] == mn)
			++i;
		else if(B[j] == mn)
			++j;
		else
			++k;
	}
	cout<<"Minimized result is: "<<diff<<endl;
}
