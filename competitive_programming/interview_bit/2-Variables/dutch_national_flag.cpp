#include<iostream>
#include<algorithm>
#include<vector>

using namespace std;

int main(){

	vector<int> A{0, 2, 1, 2, 2,2, 1, 0, 0 , 0, 0};
	int i = -1, j = 0, k = A.size()-1;
	while(A[k] == 2)
		--k;
		
	while(j <= k) {
		if(A[j] == 1) {
			++j;
			continue;
		}

		else if(A[j] == 0) {
			++i;
			swap(A[i], A[j]);
			++j;
			continue;
		}
		else {
			swap(A[j], A[k]);
			--k;
		}
	}
	for(auto a : A)
		cout<<a<<" ";

	return 0;
}


