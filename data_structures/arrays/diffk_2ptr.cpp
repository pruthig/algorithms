// Diffk - This program searches for two indices so that the difference of elements at those indices is a given integer
#include<iostream>
#include<unordered_set>
#include<algorithm>

using namespace std;

namespace {
};

int main() {
	vector<int> vec { 2, 3, 4, 5, 7, 8};
	int desired = 5;
	
	sort(vec.begin(), vec.end());
	int i = 0;
	int j = i+1;
	while(i < j && i <= vec.size()-2 && j <= vec.size()-1) {
		if(vec[j]-vec[i] == desired) {
			cout<<"Diff found\n";
			return 1;
		}
		else if(vec[j]-vec[i] < desired)
			++j;
		else
			++i;
			
	}
	cout<<"No diff found\n";
	return 0;
}
