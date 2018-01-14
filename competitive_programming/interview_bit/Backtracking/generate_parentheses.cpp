/* 
Given n pairs of parentheses, write a function to generate all combinations of well-formed parentheses of length 2*n.
For example, given n = , a solution set is:
"((()))", "(()())", "(())()", "()(())", "()()()"
*/
#include<iostream>
#include<vector>

using namespace std;

namespace {
    vector<string> res{};
};

// left_count refers to '(' left and right_count refers to ')'
void func(string tmp, int k, int left_count, int right_count) {
    if(tmp.size() == 2*k) {
        res.push_back(tmp);
        return;
    }

     if(left_count < k)
         func(tmp+'(', k, left_count+1, right_count);
     if(right_count < k && right_count < left_count)
         func(tmp+')', k, left_count, right_count+1);
}

int main() {
    int k = 3;
    if(k == 0)
        return 0;
    func("", k, 0, 0);
    for(auto a : res) {
        for(auto b : a)
            cout<<b;
        cout<<endl;
    }
    cout<<"Count is: "<<res.size()<<endl;
    return 0;
}
