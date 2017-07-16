#include<algorithm>
#include<iostream>
#include<vector>
using namespace std;


int main() {

	// designator is 0
	vector<int> A{ 1, 5, 1 };
	int vec_size = A.size();
	if(vec_size == 0)
		return 0;
	int i = 0;
	int j = 0;
	for(i = vec_size-1; i >= 1; --i) {
		for(j = i-1; j >= 0; --j) {
			if(A[j] < A[i]) {
				cout<<"Hit at: "<<i<<" and j : "<<j<<endl;
				goto START;
			}
		}
	}
START:
	if(i == 0) {
		sort(A.begin(), A.end());
		for(auto a : A) {
			cout<<a<<" ";
		}
		return 0;
	}
	swap(A[j], A[i]);
	sort(A.begin()+i, A.end());
	for(auto a : A) {
		cout<<a<<" ";
	}
	return 0;

}
