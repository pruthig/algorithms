// container with most water
// there are n vertical lines between 0 to n-1. Each line if height 'i' . Find the endpoints of
// 2 vertices lines which have the max. area
#include<iostream>
#include<vector>

using namespace std;

int main() {
	
	int  i = 0;
	vector<int> A{ 1, 5, 4, 3};
	int j = A.size()-1;
	int max = INT_MIN, cur = 0;
	while(i < j) {
		if(A[i] <= A[j]) {
			cur = A[i]*(j-i);
			++i;
		}
		else {
			cur = A[j]*(j-i);
			--j;
		}
		if(cur > max)
			max = cur;
		
	}
	cout<<"Max water that can be hold in array is: "<<max;
	return 0;
}
