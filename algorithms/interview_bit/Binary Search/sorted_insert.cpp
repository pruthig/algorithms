#include<iostream>
#include<vector>

using namespace std;

int findPosition(vector<int> A, int n, int start, int end) {
	int mid = (start+end)/2;
	int mid_n = A[mid];
	if(mid_n == n)
		return mid;

	if(end == mid) {
		if(A[mid] > n) {
		
			if(mid == 0)
				return 0;
			else
				return mid;
		}
		else
        	return mid+1;

	}
	if( n < mid_n)
		return findPosition(A, n, start, mid-1);
	else
		return findPosition(A, n, mid+1, end);
}
int main() {
	vector<int> A{2,5,6,7};
	int n = -67;

	int pos = findPosition(A, n, 0, A.size()-1);
	cout<<"Position is : "<<pos<<endl;
}


