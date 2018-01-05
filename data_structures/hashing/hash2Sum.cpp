// This program creates a 2-Sum HashMap
#include<iostream>
#include<unordered_map>
#include<vector>

using namespace std;


namespace {
	unordered_map<int, int> mp;
};



int main()
{
	vector<int> vec {1, 1, 1, 4};
	int k = 0, index1 = -1, index2 = -1;
	cout<<"Enter the sum\n";
	cin>>k;
	for(int i = 0; i < vec.size(); ++i) {
		if(mp.find(k-vec[i]) != mp.end()) {
			index2 = i;
			index1 = (mp.find(k-vec[i]))->second;
			break;
		}
		else
			mp.insert(std::make_pair(vec[i], i));
	}
	cout<<"Starting and end index are: "<<index1<<" and "<<index2<<endl;
	return 0;
}



