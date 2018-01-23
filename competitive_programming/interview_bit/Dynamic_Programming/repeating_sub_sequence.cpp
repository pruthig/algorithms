// Finding if there exists 2 equivalent subsequences
#include<iostream>
#include<vector>
#include<utility>
#include<unordered_map>

using namespace std;

int main() {
	unordered_map<string, pair<int, int>> mp{};
	string input = "aabb";
	
	for(int i = 0; i < input.size()-1; ++i) {
		string temp = "";
		temp += input[i];
		for(int j = i+1; j < input.size(); ++j) {
			temp += input[j];
			if(mp.find(temp) != mp.end()) {
				if(mp.find(temp)->second.first != i && mp.find(temp)->second.second != j) {
					cout<<"Yes\n";
					return 1;
				}
			}
			else {
				mp.insert(std::make_pair(temp, make_pair(i, j)));
			}
			temp.pop_back();
		}
	}
	cout<<"No\n";
	return 0;
}
