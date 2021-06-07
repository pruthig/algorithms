#include <iostream>
#include <string>
#include <climits>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stack>
#include <cmath>
#include <cstdlib>
#include <set>
#include <map>

using namespace std; 

void cutRodUtil(int price[], int w, int val, int arr_len, int& mx) {
    if(w == 0) {
        if(val>mx)
            mx = val;
        return;
    }
    for(int i=1;i<=arr_len;++i) {
        if(i>w)
            return;
        cutRodUtil(price, w-i, val+price[i-1], arr_len, mx);
    }
}
int cutRod(int price[], int n) {
    int max_len = INT_MIN;
    cutRodUtil(price, n, 0, n, max_len);
    return max_len;
}

void findSubsets(char s[], unsigned int sz, vector<string>& subsets, string str, int start) {
    if(start == sz)
        return;
    for(int i=start;i<sz;++i) {
        subsets.push_back(str+s[i]);
        findSubsets(s, sz, subsets, str+s[i], i+1);
    }
}

// Here middle will be the pivot will be used for transferring the disks.
// So, suppose if disk is being moved from 'A' to 'B' then we use
// 'C' as intermediary tower
// First, we will use A to B using C
// Then, we will move disk from A to C
// Now, we'll move (n-1) disks from B to C (using A)
void towerOfHanoi(int n, int A, int B, int C) {
    if(n == 0)
        return;
    towerOfHanoi(n-1, A, C, B);
    cout<<"Move from A:"<<A<<"  to C: "<<C<<endl;
    towerOfHanoi(n-1, B, A, C);
}
    
int main() {
    int price[] = {1, 5, 8, 9, 10, 17, 17, 20};
    char s[] = { 'a', 'b', 'c' };
    //string str = "";
    //vector<string> subsets{};
    //cout<<"Max value obtained is: "<<cutRod(price, sizeof(price)/sizeof(price[0]))<<endl;
    //subsets.push_back(str);
    //findSubsets(s, sizeof(s)/sizeof(s[0]), subsets, str, 0);
    //cout<<"Subsets are: "<<endl;
    //for(auto a : subsets) {
    //    cout<<a<<endl;
    //}
    towerOfHanoi(3, 1, 2, 3);
    
    return 0;
}
    
