#include<iostream>
#include<stack>
#include<vector>
#include<algorithm>

using namespace std;

int main() {
	//This program helps fine th nearest smallest element
	vector<int> vec{1, 1, 1};
	vector<int> res_vec{};
	stack<int> st{};
	if(vec.empty())
		return 0; // res_vec;
		
	st.push(vec[0]);
	for(int i = 1; i < vec.size(); ++i) {
		if(st.top() >= vec[i])
			st.push(vec[i]);
	}
	
	for(int i = vec.size()-1; i >= 0; --i) {
		if(vec[i] == st.top()) {
			res_vec.push_back(-1);
			st.pop();
		}
		else
			res_vec.push_back(st.top());
	}
	reverse(res_vec.begin(), res_vec.end());
	for(auto a : res_vec)
		cout<<a<<" ";
	return 0;
	
}
