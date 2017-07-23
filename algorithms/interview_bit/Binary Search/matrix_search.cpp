// Simple program for matrix search.
// Logic: We will traverse left and right columns to find appropriate row, if indices are equal we will apply
// binary search on that
#include<iostream>
#include<vector>
#include<algorithm>

using namespace std;

int main() {
	vector<vector<int> > A {
	{1, 1, 2, 2, 5, 6, 6, 6, 7},
  {9, 10, 10, 12, 12, 13, 14, 21, 21},
  {23, 26, 26, 29, 29, 31, 32, 35, 37},
  {38, 39, 39, 39, 41, 41, 42, 42, 43},
  {45, 45, 46, 46, 46, 47, 48, 48, 51},
  {51, 51, 54, 54, 54, 54, 56, 58, 59},
  {60, 61, 61, 62, 63, 64, 65, 66, 67},
  {67, 67, 70, 70, 71, 73, 73, 73, 74},
  {74, 79, 79, 80, 82, 84, 84, 84, 87},
  {89, 93, 94, 94, 97, 98, 98, 98, 98}
							};
	cout<<"Enter element  to search"<<endl;
	int element = 0;
	cin>>element;

	vector<int> v_left, v_right;
	for(int i = 0; i < A.size(); ++i)
		v_left.push_back(A[i][0]);

	for(int i = 0; i < A.size(); ++i)
		v_right.push_back(A[i][A[0].size()-1]);

	std::vector<int>::iterator left, right;
	left=std::lower_bound (v_left.begin(), v_left.end(), element);
	int idx_l = left - v_left.begin();
	if(v_left[idx_l] == element) {
		cout<<"Found";
		return 0;
	}
	--idx_l;

	right=std::lower_bound (v_right.begin(), v_right.end(), element);
	auto idx_r = right - v_right.begin();
	if(v_right[idx_r] == element) {
		cout<<"Found";
		return 0;
	}
	if(idx_l != idx_r)
		return -1;

	if (std::binary_search (A[idx_l].begin(), A[idx_l].end(), element))
    	cout << "found!\n";
	else {
		std::cout << "not found.\n";
		return -1;
	}
}
