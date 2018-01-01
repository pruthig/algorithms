#include<iostream>
#include<climits>
#include<algorithm>
#include<vector>
#include<cmath>

using namespace std;

int main() {
	vector<int> A{ -1, 2, 1, -4};
	sort(A.begin(), A.end());
	int B = 1;
	int sum = 0;
	int min = INT_MAX;
	if(A.size() <= 2) {
		cout<<"Size should be >2\n";
		return 0;
	}
	for(int i = 0; i <= A.size()-3; ++i) {
		int j = i+1;
		int k = A.size()-1;
		while(j<k) {
			int s = A[i] + A[j] + A[k];
			if( abs(s-B) < min) {
				min = abs(s-B);
				sum = s;
			}
			if(s>B)
				--k;
			else		
				++j;
		}
	}
	cout<<"Sum is: "<<sum<<endl;
	return 0;
}
