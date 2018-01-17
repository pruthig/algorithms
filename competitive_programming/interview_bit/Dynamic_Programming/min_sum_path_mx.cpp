/*
Given a m x n grid filled with non-negative numbers, find a path from top left to bottom right which minimizes the sum of all numbers along its path.
*/

#include<vector>
#include<iostream>

using namespace std;


int main() {
	vector<vector<int>> input{ 
	  {20, 29, 84, 4, 32, 60, 86, 8, 7, 37},
	  {77, 69, 85, 83, 81, 78, 22, 45, 43, 63},
	  {60, 21, 0, 94, 59, 88, 9, 54, 30, 80},
	  {40, 78, 52, 58, 26, 84, 47, 0, 24, 60},
	  {40, 17, 69, 5, 38, 5, 75, 59, 35, 26},
	  {64, 41, 85, 22, 44, 25, 3, 63, 33, 13},
	  {2, 21, 39, 51, 75, 70, 76, 57, 56, 22},
	  {31, 45, 47, 100, 65, 10, 94, 96, 81, 14}
  	};
  	
	if(input.size() == 0)
		return 0;
	
	int row = input.size();
	int col = input[0].size();
	
	for(int i = 0; i < row; ++i) {
		for(int j = 0; j < col; ++j) {
			if( i == 0 && j == 0) {
				continue;
			}
			if(i == 0) {
				input[i][j] += input[i][j-1]; 
			}
			else if(j == 0) {
				input[i][j] += input[i-1][j]; 
			}
			else {
				input[i][j] += min(input[i-1][j], input[i][j-1]);
			}
		}
	}
	cout<<"Minimum sum in path is: "<<input[row-1][col-1];
	return 0;
}
