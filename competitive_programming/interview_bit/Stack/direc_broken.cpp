/*
Given an absolute path for a file (Unix-style), simplify it.

Examples:

path = "/home/", => "/home"
path = "/a/./b/../../c/", => "/c"
Note that absolute path always begin with ‘/’ ( root directory )
Path will not have whitespace characters.
*/
#include<stack>
#include<iostream>
#include<vector>
#include<algorithm>
#include<sstream>

using namespace std;

std::vector<std::string> my_split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
  }
  return elems;
}

int main() {
	string str = "/../..//";
	stack<string> st{};
	vector<string> vec{};
	vector<string> vecStr = my_split(str, '/');
	for(auto  a : vecStr) {
		if(a == ".")
			continue;
		else if(a == "..") {
			if(!st.empty())
				st.pop();
		}
		else
			st.push(a);
	}
	while(!st.empty()) {
		string t = st.top();
		if(!t.empty())
			vec.push_back(st.top());
		st.pop();
	}
	//reverse(vec.begin(), vec.end());
	string s = "";
	
	for(auto a : vec) {
		s = s + "/" + vec.back();
		vec.pop_back();
	}
	
	if(s.size() == 0)
		cout<<"/";
	else
		cout<<"String is: "<<s<<endl;
	return 0;
}
	
