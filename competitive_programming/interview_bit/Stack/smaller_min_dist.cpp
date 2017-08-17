#include<iostream>
#include<vector>
#include<stack>

using namespace std;

int main() {
	vector<int> vec{1, 1, 1, 20, 21, 21};
	vector<int> res_vec{};
	stack<int> st{};
	if(vec.size() == 0)
		return 0; //res_vec;
		
	st.push(vec[0]);
	res_vec.push_back(-1);
	
	for(int i = 1; i < vec.size(); ++i) {
		if(vec[i] > st.top()) {
			res_vec.push_back(st.top());
			st.push(vec[i]);
		}
		else {
			while(!st.empty() && st.top() >= vec[i])
				st.pop();
			if(st.empty()) 
				res_vec.push_back(-1);
			else
				res_vec.push_back(st.top());
			
			st.push(vec[i]);
			
		}
	}
	for(auto a : res_vec)
		cout<<a<<" ";
}

