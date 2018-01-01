#include<iostream>
#include<climits>
#include<algorithm>
#include<vector>
#include<cmath>

using namespace std;

int main() {
	vector<int> A{ 1, -4, 0, 0, 5, -5, 1, 0, -2, 4, -4, 1, -1, -4, 3, 4, -1, -1, -3 };
	sort(A.begin(), A.end());
	cout<<"res: "<<endl;
	for(auto a : A) {
		cout<<a<<"  ";
	}
	
	if(A.size() <= 2) {
		cout<<"Size should be >2\n";
		return 0;
	}
	
	vector<vector<int>> res;
	res.clear();
	
	
	int sum = 0;
	
	
	for(int i = 0; i <= A.size()-3; ++i) {
		// To prevent duplicacy
		if(i>0 && A[i] == A[i-1])
			continue;
		int j = i+1;
		int k = A.size()-1;
		while(j<k) {
			// To prevent duplicacy
			if(j>i+1 && A[j] == A[j-1]) {
				++j;
				continue;
			}
			
			int s = A[i] + A[j] + A[k];
			if(s == 0) {
				vector<int> tmp  { A[i], A[j], A[k] };
				res.push_back(tmp);
			}
			if(s>0)
				--k;
			else		
				++j;
		}
	}
	cout<<"Triplets are: "<<endl;
	for(auto a : res) {
		for(auto b : a)
			cout<<b<<" ";
		cout<<endl;
	}
	return 0;
}
