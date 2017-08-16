#include<iostream>
#include<algorithm>
#include<vector>

using namespace std;

int main() {
	vector<int>::iterator vecIter;
	
	int element = 2;
	
	vector<int> A{ 101, 103, 106, 109, 158, 164, 182, 187, 202,    205,    2, 3, 32, 57, 69, 74, 81, 99, 100 };
	
	int start = 0, end = A.size()-1;
	while(start <= end) {
		int mid = (start+end)>>1;
		if(A[mid] == element)
			return mid;
		else if(element >= A[mid+1] && A[end]  >=  A[mid+1] && element <= A[end] || (element >= A[mid] && A[end] < A[mid]))
			start = mid+1;
		else
			end = mid-1;
	}
	cout<<"Element not found"<<endl;
}
