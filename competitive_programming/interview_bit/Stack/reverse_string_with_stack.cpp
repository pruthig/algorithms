#include<iostream>
#include<stack>

using namespace std;

int main() {
	string s = "abc";
	stack<char> st;
	for(int i = 0; i < s.size(); ++i)
		st.push(s[i]);
	
	string mod_s = "";
	while(!st.empty()) {
		mod_s = mod_s + st.top();
		st.pop();
	}
	cout<<"Reversed is: "<<mod_s<<endl;
	return 0;
}
