// This program removes duplicates from a sorted array such that a number is allowed to come only twice
#include<iostream>
#include<algorithm>
#include<vector>

using namespace std;

int main() {
	vector<int> A{1,2,3,4, 4, 4,5,6,6,6, 6, 6, 6, 6, 6,6,7,7,8,9};
	int j = 0;
	int old = INT_MIN;
	bool flag = false;
	for(int i = 0; i < A.size(); ++i) {
		if(A[i] == old && flag) {
			A[i] = INT_MIN;
			continue;
		}
		else if(A[i] == old) {
			flag = true;
			continue;
		}
		else {
			flag = false;
			old = A[i];
		}
	}
	A.erase(std::remove(A.begin(), A.end(), INT_MIN), A.end());
	for(auto a : A)
		cout<<a<<" ";
}


