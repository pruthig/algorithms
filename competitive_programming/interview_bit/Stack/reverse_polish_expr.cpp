#include<iostream>
#include<string>
//#include<locale>
#include<stack>
#include<algorithm>
using namespace std;

/* This program find result of a reverse-polish notation
bool has_only_digits(const string s){
  return s.find_first_not_of( "0123456789" ) == string::npos;
}
*/
bool has_only_digits(std::string const &in) {
    char *end;

    strtol(in.c_str(), &end, 10);
    return !in.empty() && *end == '\0';
}

int main() {
	vector<string> vec{ "-1", "10", "+"}; //, "/", "+"};
	stack<string> st{};
	for(auto  str : vec) {

		if(has_only_digits(str)) {
			st.push(str);
		}
		else {
			int e1 = stoi(st.top()); st.pop(); 
			int e2 = stoi(st.top()); st.pop();
			
			if(str == "+") { 
				st.push(to_string(e1+e2));
			}
			else if(str == "-") { 
				st.push(to_string(e2-e1));
			}
			else if(str == "*") { 
				st.push(to_string(e1*e2));
			}
			else { 
				st.push(to_string(e2/e1));
			}
		}
	}
	cout<<"result is: "<<stoi(st.top())<<endl;
	st.pop();
	return 0;
}
