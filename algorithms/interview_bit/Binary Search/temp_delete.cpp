#include<iostream>
#include<vector>
#include<algorithm>

using namespace std;


int  main() {
	vector<int> v{2, 4, 6, 18, 23, 45, 67};
	vector<int>::iterator itr;
	itr = lower_bound(v.begin(), v.end(), 461);
	cout<<"index is: "<<(itr-v.begin());
	return 0;
}
