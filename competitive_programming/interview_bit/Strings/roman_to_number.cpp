#include<iostream>
#include<map>

using namespace std;

int main() {
	// Initialize
	map<char, int> mp;
	mp['I'] = 1;
	mp['V'] = 5;
	mp['X'] = 10;
	mp['L'] = 50;
	mp['C'] = 100;
	mp['D'] = 500;
	mp['M'] = 1000;
	
	string s = "LM"; //- 1990
	int size = s.size();
	if(s.size() == 1) {
		cout<<"Sum is : "<<mp[s[0]];
		return 0;
	}
	
	int sum = 0;
	char old = s[0];
	for(int i = 1; i < s.size(); ++i) {
		char cur = s[i];
		if(old == '\0') {
			old = s[i];
			continue;
		}

		if(mp[cur] <= mp[old]) {
			sum = sum + mp[old];
			old = cur; // old = 'C'
			continue;
		}
		else {
			sum = sum +  ( mp[cur] - mp[old] );
			cout<<"Added : "<<mp[cur] - mp[old]<<" total: "<<sum<<endl;
			old = '\0';
		}		
	}
	if(mp[s[size-1]] <= mp[s[size-2]]) {
		sum = sum + mp[s[size-1]];
	}
	cout<<"Sum is: "<<sum<<endl;
	return 0;
}
