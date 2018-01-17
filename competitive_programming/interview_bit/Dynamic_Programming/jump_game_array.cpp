/* Given an array of non-negative integers, you are initially positioned at the first index of the array.
Each element in the array represents your maximum jump length at that position.
Determine if you are able to reach the last index.
For example:
A = [2,3,1,1,4], return 1 ( true ).
*/

#include<iostream>
#include<vector>

using namespace std;

int main() {
	vector<int> vec{ 1 ,2 ,2, 2, 0, 1};
	vector<int> res(vec.size(), 0);
	
	res[vec.size()-1] = 1;
	for(int i = vec.size()-2; i >= 0; --i) {
		int x = vec[i];
		for(int j = 1; j < vec.size() && j <=x ; ++j) {
			if(res[j+i] == 1) {
				res[i] = 1;
				break;
			}
		}
	}
	cout<<"Result is: "<<res[0];
	return 0;
}
