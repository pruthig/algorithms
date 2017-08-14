// Intersection of 2 Arrays
#include<iostream>
#include<vector>

using namespace std;

int main() {
	vector<int> v1{ 4, 9, 11, 17, 17, 31, 31, 42, 46, 47, 50, 54, 55, 56, 58, 63, 65, 65, 66, 70, 74, 78, 81, 81, 83, 84, 85, 94, 101};
	vector<int> v2{ 1, 2, 7, 14, 23, 24, 24, 26, 27, 28, 29, 29, 29, 30, 33, 39, 40, 42, 44, 45, 45, 46, 56, 56, 59, 61, 64, 70, 71, 75, 76, 80, 83, 85, 86, 87, 89, 89, 91, 92, 95, 98, 100};
	
	int i = 0;
	int j = 0;
	vector<int> res;
	while(i < v1.size() && j < v2.size()) {
		if(v1[i] < v2[j]) {
			++i; continue;
		}
		if(v2[j] < v1[i]) {
			++j; continue;
		}
		if(v1[i] == v2[j]) {
			res.push_back(v1[i]);
			++i; ++j;
		}
	}
	cout<<"Intersection values are: ";
	for(auto a : res)
		cout<<a<<"  ";
}
