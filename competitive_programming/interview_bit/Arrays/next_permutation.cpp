#include<algorithm>
#include<iostream>
#include<vector>
using namespace std;


int main() {

	// designator is 0
	vector<int> A{ 100 };
	
	int vec_size = A.size();
	int min_index = vec_size-1;
	
	if(vec_size == 0)
		return 0;
	int i = 0;
	
	for(i = vec_size-1; i >= 1; --i) {
		if(A[i]< A[min_index])
			min_index = i;
		if(A[i-1] < A[i]) {
			cout<<"Hit at: "<<i<<endl;
			break;
		}
	}

	if(i == 0) {
		sort(A.begin(), A.end());
		for(auto a : A) {
			cout<<a<<" ";
		}
		return 0;
	}
	// Now find the interger larger than i and less than all elements in next
	int min_right = i;
	for(int j = i; j <= vec_size-1; ++j) {
		if(A[j] > A[i-1] && A[j] < A[min_right])
			min_right = j;
	}
	
	
	swap(A[i-1], A[min_right]);
	sort(A.begin()+i, A.end());
	for(auto a : A) {
		cout<<a<<" ";
	}
	return 0;

}
