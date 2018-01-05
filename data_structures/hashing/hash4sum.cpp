// This program creates a 4-Sum HashMap
// Given an array A of integers, find the index of values that satisfy A + B = C + D, where A,B,C & D are integers values in the array
// The below solution has been done using O(n^2)
#include<iostream>
#include<unordered_map>
#include<vector>
#include<utility>

using namespace std;


namespace {
	// map of <sum, pair_of_indices>
	unordered_map<int, std::pair<int, int>> mp;
};



int main()
{
	vector<int> vec {1, 5, 2, 4, 2, 0, 2, 5, 1, 0, 5, 0 };
	int index[4] = {-1};
	vector<int> res{INT_MAX, INT_MAX, INT_MAX, INT_MAX};

	for(int i = 0; i < vec.size()-1; ++i) {
		for(int j = i+1; j < vec.size(); ++j) {
			if(mp.find(vec[i] + vec[j]) != mp.end()) {
				// Update current ones
				index[2] = i; index[3] = j;
				index[0] = (mp.find(vec[i] + vec[j]))->second.first;
				index[1] = (mp.find(vec[i] + vec[j]))->second.second;
				// Update result
				if( ((index[0]<res[0]) || (index[0] == res[0] && index[1] < res[1]) 
				    || (index[0] == res[0] && index[1] == res[1] && index[2] < res[2]) 
				    || (index[0] == res[0] && index[1] == res[1] && index[2] == res[2] && index[3] < res[3])) && index[0]<index[1] && index[2]<index[3] 
				&& index[0] < index[2] && index[1] != index[3] && index[1] != index[2]){
					res[0] = index[0];
					res[1] = index[1];
					res[2] = index[2];
					res[3] = index[3];
				}
				
			}
			else
				mp.insert( {  (vec[i]+vec[j]), {i, j } } );
		}
	}
	if(res[0] != INT_MAX) {
		cout<<"Indices are: "<<res[0]<<" "<<res[1]<<" "<<res[2]<<" "<<res[3]<<endl;
		//return res;
	}	
	
	return 0;
}



