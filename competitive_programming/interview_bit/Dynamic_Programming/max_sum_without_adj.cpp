/* Given a 2 * N Grid of numbers, choose numbers such that the sum of the numbers is maximum and no two chosen numbers are adjacent 
horizontally, vertically or diagonally, and return it.
Example:
	1 2 3 4
	2 3 4 5
so we will choose 3 and 5 so sum will be 3 + 5 = 8
*/
#include<iostream>
#include<vector>

using namespace std;


int main() {
	vector<vector<int>> input{ {16, 5, 54, 55, 36, 82, 61, 77, 66, 61}, {31, 30, 36, 70, 9, 37, 1, 11, 68, 14}};
	if(input.size() == 0)
		return 0;

	int sz = input[0].size();
	vector<int> res(sz);
	for(int i = 0; i < res.size(); ++i) {
		res[i] = max(input[0][i], input[1][i]);
	}
	if(res.size() == 1) {
		cout<<"Sum is: "<<res[0];
		return 0;
	}
	else if(res.size() == 2) {
		cout<<"Sum is: "<<max(res[0], res[1]);
		return 0;
	}
	else if(res.size() == 3) {
		cout<<"Sum is: "<<max(res[0]+res[2], res[1]);
		return 0;
	}
	else {
		res[2] = res[0]+res[2];
		for(int i = 3; i < sz; ++i) 
			res[i] += max(res[i-2], res[i-3]);
	}

	cout<<"Max is: "<<max(res[sz-1], res[sz-2]);
	return 0;
}
