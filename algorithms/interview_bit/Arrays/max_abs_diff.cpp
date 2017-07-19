//You are given an array of N integers, A1, A2 ,…, AN. Return maximum value of f(i, j) for all 1 = i, j = N.
//f(i, j) is defined as |A[i] - A[j]| + |i - j|, where |x| denotes absolute value of x.

// Solution approach:
/*
f(i, j) = |A[i] - A[j]| + |i - j| can be written in 4 ways (Since we are looking at max value, we don’t even care 
if the value becomes negative as long as we are also covering the max value in some way).

(A[i] + i) - (A[j] + j)
-(A[i] - i) + (A[j] - j) 
(A[i] - i) - (A[j] - j) 
(-A[i] - i) + (A[j] + j) = -(A[i] + i) + (A[j] + j)
Note that case 1 and 4 are equivalent and so are case 2 and 3.

We can construct two arrays with values: A[i] + i and A[i] - i. Then, for above 2 cases, we find the maximum value possible. For that, we just have to store minimum and maximum values of expressions A[i] + i and A[i] - i for all i.
*/

#include<iostream>
#include<algorithm>
#include<vector>
#include<climits>

using namespace std;

int main() {

	vector<int> A {1, 3, -1};
	typedef vector<int>::iterator vecIter;	
	vector<int> a_i_plus_i(A.size());
	vector<int> a_i_minus_i(A.size());
	
	for(int i = 0; i < A.size(); ++i) 
	{
		a_i_plus_i[i] = A[i] + i;
		a_i_minus_i[i] = A[i] - i;
	}
	vecIter iter_max_plus = max_element(a_i_plus_i.begin(), a_i_plus_i.end());
	vecIter iter_min_plus = min_element(a_i_plus_i.begin(), a_i_plus_i.end());
	
	vecIter iter_max_minus = max_element(a_i_minus_i.begin(), a_i_minus_i.end());
	vecIter iter_min_minus = min_element(a_i_minus_i.begin(), a_i_minus_i.end());
	
	cout<<"Solution is : "<<max( *iter_max_plus - *iter_min_plus, *iter_max_minus - *iter_min_minus )<<endl;
	return 0;
}
