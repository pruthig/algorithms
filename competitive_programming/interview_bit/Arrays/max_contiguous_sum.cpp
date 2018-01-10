#include<iostream>
#include<vector>
#include<climits>

using namespace std;


int main() {
	vector<int> A{ -1 };
	int cur_sum = 0;
	int sum = INT_MIN;
	for(int i = 0; i < A.size(); ++i) {
		cur_sum = cur_sum + A[i];
		if(cur_sum > sum)
			sum = cur_sum;
		if(cur_sum < 0)
			cur_sum = 0;
	}
	cout<<"Max sum is: "<<sum<<endl;
	return 0;
}
