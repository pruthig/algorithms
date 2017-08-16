// Find min XOR between any 2 elements

#include<iostream>
#include<vector>
#include<algorithm>
#include<limits>

using namespace std;

int main() {
	vector<int> v{7, 0, 9, 13, 16, 23, 30};
	int x = 0, y = 0, min = INT_MAX;
	sort(v.begin(), v.end());
	
	for(int i = 0; i < v.size()-1; ++i) {
		if((v[i]^v[i+1]) < min) {
			min = v[i]^v[i+1];
			x = v[i];
			y = v[i+1];
		}
	}
	cout<<"NUmber and min XOr: Min - x - y: "<<min<<" "<<x<<"  "<<y<<endl;
	return 0;
}
