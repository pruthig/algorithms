/* print anti-diagonals
Input: 	

1 2 3
4 5 6
7 8 9

Return the following :

[ 
  [1],
  [2, 4],
  [3, 5, 7],
  [6, 8],
  [9]
]
*/

#include<iostream>
#include<vector>
#include<unistd.h>

using namespace std;

int main() {
	int n  = 4;
	vector< vector<int> > vec(2*n-1);
	vector<vector<int> > A = { 
							 {1, 2, 3, 4},
							 {5, 6, 7, 8},
							 {9 ,10, 11, 12},
							 {13, 14, 15, 16}
							 };
	
	int i = 0;
	for(; i <= n-1; ++i) {
		int k = i;
		for(int j = 0; j <= i; ++j, --k){
			vec.at(i).push_back(A[j][k]);
		}
	}
	int p = i;
	for(int i = 1; i <= n-1; ++i, ++p) {
		int k = i;
		for(int j = n-1; j >= i && i <= n-1; j--, ++k)
			vec.at(p).push_back(A[k][j]);
	}
	for(auto a : vec) {
		for(auto b : a)
			cout<<b;
	cout<<endl;
	}
	sleep(10000);
}
