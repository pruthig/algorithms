#include<iostream>
#include<vector>

using namespace std;

void count_duplicates(vector<int>& vec, int B, int&count, int start, int end) {
	if(start > end)
		return;
	int mid = (start+end)/2;
	if(vec[mid] == B)
		++count;
	count_duplicates(vec, B, count, start, mid-1);
	count_duplicates(vec, B, count, mid+1, end);
}
int main() {
	vector<int> A{ 1, 2, 3, 4, 5};
	int B = 2;
	int count = 0;
	count_duplicates(A, B, count, 0, A.size()-1);
	cout<<"Count for "<<B<<" is: "<<count<<endl;
}
