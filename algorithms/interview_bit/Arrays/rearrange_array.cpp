#include<vector>
#include<iostream>

using namespace std;

int main() {
	vector<int> A{0, 2, 1, 4, 3};
	int n = A.size();
	for(int i = 0; i < n; ++i) {
		A[i] += (A[A[i]] % n)*n;
	}
	for(auto &a: A)
		a = a/n;
	
	for(auto a: A)
		cout<<a<<" ";
	cout<<endl;
}
