#include<iostream>
#include<vector>

using namespace std;

int main() {
	int numRows = 0;
	vector< vector<int> > vec(numRows);
	for(int i = 0; i < numRows; ++i ) {
		for(int j = 0; j <=i; ++j) {
			if(j == 0) {
				vec.at(i).push_back(1);
				continue;
			}
			else if(j == i) {
				vec.at(i).push_back(1);
				continue;
			}
			else {
				vec.at(i).push_back( vec.at(i-1).at(j-1) + vec.at(i-1).at(j) );
			}
		}
	}
	for(auto a : vec) {
		for(auto b : a)
			cout<<b;
		cout<<endl;
	}
}
