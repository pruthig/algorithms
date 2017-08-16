#include<iostream>
#include<stack>

using namespace std;

int main() {
	string s = "[{]";
	bool no_match = false;
	
	stack<char> st;
	for(int i = 0; i < s.size(); ++i) {
		if(s[i] == '(' || s[i] == '{' || s[i] == '[')
			st.push(s[i]);
		else {
			if(s[i] == ')' ){
				if(st.empty() || st.top() != '(') {
					no_match = true;
					goto EXIT;
				}
				st.pop();
			}
			else if(s[i] == '}') {
				if(st.empty() || st.top() != '{') {
					no_match = true;
					goto EXIT;
				}
				st.pop();
			}
			else if(s[i] == ']') {
				if(st.empty() || st.top() != '[') {
					no_match = true;
					goto EXIT;
				}
				st.pop();
			}
			
		}
	}
EXIT:
	if(no_match || !st.empty())
		cout<<"Parentheses are not balanced";
	else
		cout<<"Parentheses are balanced\n";
	return 0;
}
