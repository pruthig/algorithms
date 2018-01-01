// This program merges 2 arrays and creates another array
#include<iostream>
#include<vector>

using namespace std;

int main() {
	vector<int> A{ -4, 3 };
	vector<int> B{ -2, -2};
	vector<int> C;
	C.resize(A.size() + B.size());
	int i = 0, j = 0, k = 0;
	while(i < A.size() && j < B.size()) {
		if(A[i] <= B[j]) {
			C[k] = A[i];
			++i;
			++k;
		}
		else {
			C[k] = B[j];
			++k;
			++j;
		}
	}
	if(i == A.size()) {
		while(k < C.size()) {
			C[k] = B[j];
			++k;
			++j;
		}
	}
	if(j == B.size()){
		while(k < C.size()) {
			C[k] = A[i];
			++k;
			++i;
		}
	}
	A = C;
	for(auto a: A)
		cout<<a<<" ";
		
	return 0;
}
