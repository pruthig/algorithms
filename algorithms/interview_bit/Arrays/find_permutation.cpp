// Another optimized solution will be to start from left and right for D pick from last and
// add to result add element from start.
#include<algorithm>
#include<iostream>
#include<vector>
using namespace std;


int main() {

	// designator is 0
	int B = 6;
	string s = "IDDID";
	
	std::vector<int> vec(B) ; // vector with 100 ints.
	std::iota (std::begin(vec), std::end(vec), 1); // Fill with 0, 1, ..., 99.
	
	int vec_size = vec.size();
	if(vec_size == 0)
		return 0;

	int i = 0;
	// 'I' means next greater
	while(i < vec_size-1) { 
		if(s[i] == 'I') {
			++i;
			continue;
		}
		else {
			int j = i;
			while(i <= vec_size-2 && s[i] != 'I')
				++i;
			sort(vec.begin()+j, vec.begin()+i+1, std::greater<int>());
			continue;
		}
	}
	
	for(auto a : vec) {
		cout<<a<<" ";
	}
	return 0;

}
