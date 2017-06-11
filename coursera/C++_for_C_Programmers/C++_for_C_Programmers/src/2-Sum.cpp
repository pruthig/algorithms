// This program implements the solution for 2-Sum problem wherein we have to find number of pairs
// who sum is equal to a given number

#include<iostream>
#include<fstream>
#include<string>
#include<set>
#include<ctime>
#include<vector>
#include <algorithm>  

using namespace std;

#define SIZE_SUM_ARR	20001

bool is_equal_to_one(int i) { return i == 1; }


void _2_sum_main() {
	// Variable decls
	string str = "";
	long long number = 0, total_count = 0;
	long long a = 0;
	std::vector<long long>::iterator low_left, up_left, low_right, up_right;

	//read the lines from file
	ifstream inputFile("resources/2-Sum.txt");
	set<long long> intSet;
	long long sum_contains[SIZE_SUM_ARR];
	int i = 0;
	
	// calculate time taken to read
	clock_t start = clock();
	while (getline(inputFile, str)) {
		number = stoll(str);
		intSet.insert(number);
	}
	int size = intSet.size();
	std::vector<long long> intVector(intSet.size());
	std::copy(intSet.begin(), intSet.end(), intVector.begin());
	cout << "Time taken to insert in set " << (double)(clock() - start) / CLOCKS_PER_SEC<<endl;
	start = clock();
	for (std::vector<long long>::iterator itr = intVector.begin(); itr != intVector.end(); ++itr) {
		// a is the number
		a = *itr;
		// Do for first half
		low_left = std::lower_bound(intVector.begin(), itr==intVector.begin()?itr:(itr - 1), -10000 - a);
		up_left = std::upper_bound(intVector.begin(), itr == intVector.begin() ? itr : (itr - 1), 10000 - a);
		for (vector<long long>::iterator itr_left = low_left; itr_left != up_left; ++itr_left)
			sum_contains[*itr_left + a + 10000] = 1;

		// same for 2nd half
		low_right = std::lower_bound(itr == intVector.end() ? itr : (itr + 1) , intVector.end(), -10000 - a);
		up_right = std::upper_bound(itr == intVector.end() ? itr : (itr + 1), intVector.end(), 10000 - a);
		for (vector<long long>::iterator itr_right = low_right; itr_right != up_right; ++itr_right)
			sum_contains[*itr_right + a+ 10000] = 1;

	}
	cout << "Time taken to calculate " << (double)(clock() - start) / CLOCKS_PER_SEC<<endl;

	vector<long long> tempVector(sum_contains, sum_contains + SIZE_SUM_ARR);
	int mycount = count_if(tempVector.begin(), tempVector.end(), is_equal_to_one);

	cout << "total count is: " << mycount << endl;

}