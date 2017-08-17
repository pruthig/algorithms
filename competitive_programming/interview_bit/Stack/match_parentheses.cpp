#include<stack>
#include<iostream>

using namespace std;

int main() {
	string s = "(a+b)";
	int count = 0;
	// Considered are '{', ;[' and '('
	for(int i = 0; i < s.size(); ++i) {
		string temp_str = "";
		if(s[i] == '{' || s[i] == '(' || s[i] == '[')
			count++;
		else if(s[i] == '+' || s[i] == '-' || s[i] == '*' || s[i] == '/')  
			--count;
		else {
		}
		if(count < 0)
			count = 0;
	}
	if(count == 0)
		return 0;
	else
		return 1;
}

