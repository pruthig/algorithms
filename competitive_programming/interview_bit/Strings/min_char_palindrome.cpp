#include<string>
#include<iostream>

/*
A : "hqghumeaylnlfdxfi"
Your function returned the following :
14
The expected returned value :
16
*/
using namespace std;

int main() {
	string s = "hqghumeaylnlfdxfi";
	bool equal = false;
	int len = s.length();
	
	int i = 0, j = len-1;
	int count = 0;
	while(i < j) {
		if(s[i] == s[j]) {
			++i; --j;
			equal = true;
		}
		else {
			--j;
			if(equal)
				count += 2;
			else
				++count;
		}
	}
	cout<<"Total required: "<<count<<endl;
	return 0;
}
