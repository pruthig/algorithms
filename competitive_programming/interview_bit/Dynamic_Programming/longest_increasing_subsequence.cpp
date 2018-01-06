// This program calculates the longest increasing subsequence
// using dynamic programming

#include<iostream>
#include<algorithm>

using namespace std;

int calculateLIS(const vector<int>& vec) {
    int n = vec.size();
    if(n == 0)
        return 0;

    vector<int> lis(n);
    std::fill(lis.begin(), lis.end(), 1);
    // Now calculate the length of LIS 

    for(int i = 1; i < n; ++i) {
        for(int j = 0; j < i; ++j) {
            if(vec[i]>=vec[j] && lis[j]+1>lis[i])
                lis[i] = lis[j] + 1;
        }
    }
    return *(max_element(lis.begin(), lis.end()));
}

int main() {
    vector<int> v {14, 24, 18, 46, 55, 53, 82, 18, 101, 20, 78, 35, 68, 9, 16, 93, 101, 85, 81, 28, 78};

    int res = calculateLIS(v);
    cout<<"Length of longest subsequence is: "<<res<<endl;
    return 0;
}
