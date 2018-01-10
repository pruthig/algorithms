// Given an array A of integers and another non negative integer k, find if there exists 2 indices i and j 
// such that A[i] - A[j] = k, i != j

#include<iostream>
#include<vector>
#include<unordered_set>

using namespace std;

namespace {
    unordered_set<int> st;
};


int main() {
    vector<int> vec{ 70, 48, 90 };
    int k = 0;
    cout<<"Enter k:"<<endl;
    cin>>k;
    for(auto&a : vec) {
        if(st.find(a+k) != st.end() || st.find(a-k) != st.end()) {
            cout<<"\nElement found\n";
            return 1;
        }
        else
            st.insert(a);
    }
    cout<<"Sorry, element doesn't exist\n";
    return 0;
}
