// Diffk - This program searches for two indices so that the difference of elements at those indices is a given integer
#include<iostream>
#include<unordered_set>
#include<algorithm>

using namespace std;

namespace {
};

int main() {
	vector<int> vec { 2, 3, 4, 5, 7, 8};
	unordered_set<int> u_m;
	unordered_set<int>::iterator it;
	int desired = 89;
	for(auto a : vec) {
		if((it = u_m.find(a-desired)) != u_m.end()) {
			cout<<"Diff found\n";
			return 1;
		}
			
		else
			u_m.insert(a);
	}
	cout<<"No diff found\n";
	return 0;
}
