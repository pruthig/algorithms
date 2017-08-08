#include<string>
#include<iostream>

using namespace std;

int main() {
	string str = "aab aaa aba baa bbb   abb abbbaabababaaaaaababaaabbabbabbabbaaaabbbbbbaabbabbbbbabababbaaabbaabbbababbb";
	string pattern = "bba";
	int pos = -1;
	int j = 0;
	if(str.size() == 0 || pattern.size() == 0 || pattern.size() > str.size())
		return -1;
		
	for(int i = 0; i < str.size(); ++i) {
		if(pattern[j] == str[i]) {
			++j;
			if(j  == pattern.size()) {
				pos = i-j+1;
				break; 
			}
			continue;
		}
		else {
			if(j != 0) {
				--i;
			}
		}
		j = 0;
	}
	cout<<"Position is: "<<pos<<endl;
}
