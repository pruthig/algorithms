// This program creates a 4-Sum HashMap
// Given an array S of n integers, are there elements a, b, c, and d in S such that a + b + c + d = target
// Find all unique quadruplets in the array which gives the sum of target.
#include<iostream>
#include<unordered_map>
#include<vector>
#include<algorithm>
#include<utility>

using namespace std;


namespace {
	// map of <sum, pair_of_indices>
	unordered_map<int, std::pair<int, int>> mp;
};

bool notEqual(int *ind, vector<vector<int>>& res) {
	if(res.size() == 0)
		return true;
	vector<int> comp = res.at(res.size()-1);
	if(comp[0] != ind[0] || comp[1] != ind[1] || comp[2] != ind[2] || comp[3] != ind[3])
		return true;
	else
		return false;
	
}

void findQuadruplets()
{
	vector<int> vec {1, 5, 2, 4, 2, 0, 2, 5, 1, 0, 5, 0 };
	vector<vector<int>> res{};
	int target = 0;
	cin>>target;
	sort(vec.begin(), vec.end());
	int index[4] = {-1};

	for(int i = 0; i < vec.size()-1; ++i) {
			//if(i < vec.size()-1 && vec[i] == vec[i+1])
			//	continue;
		for(int j = i+1; j < vec.size(); ++j) {
			//if(j < vec.size()-1 && vec[j] == vec[j+1])
			//	continue;
			if(mp.find(target-(vec[i] + vec[j])) != mp.end()) {
				// Update current ones
				index[2] = i; index[3] = j;
				index[0] = (mp.find(target-(vec[i] + vec[j])))->second.first;
				index[1] = (mp.find(target-(vec[i] + vec[j])))->second.second;
				sort(index, index+3);
				// Update result
				if( index[0] <= index[1] && index[1] <= index[2] && index[2] <= index[3] && notEqual(index, res)) {
					vector<int> tmp{ vec[index[0]], vec[index[1]], vec[index[2]], vec[index[3]] };
					res.push_back(tmp);
				}
				
			}
			else
				mp.insert( {  (vec[i]+vec[j]), {i, j } } );
		}
	}
	for(auto a: res) {
		for(auto b : a) {
			cout<<b<<"  ";
		}
		cout<<endl<<endl;
	}
	//if(res[0] != INT_MAX) {
	//	cout<<"Indices are: "<<res[0]<<" "<<res[1]<<" "<<res[2]<<" "<<res[3]<<endl;
		//return res;
	//}	
}

int main() {
	findQuadruplets();
	return 0;
}



