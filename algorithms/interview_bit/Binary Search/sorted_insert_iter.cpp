#include<iostream>
#include<vector>

using namespace std;


int main() {
	vector<int> A{2,5,6,7};
	int n = -67;
	int start = 0, end = A.size()-1;

	while(1) {
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
			end =  mid-1;
		else
			start = mid+1;

	}
	//cout<<"Position is : "<<pos<<endl;
}


