/*
Find the largest continuous sequence in a array which sums to zero.
Example:
Input:  {1 ,2 ,-2 ,4 ,-4}
Output: {2 ,-2 ,4 ,-4}
*/

// Solution: Create another array with each cell has entry = sum of elements till that index
#include<iostream>
#include<unordered_map>
#include<vector>
#include<climits>

using namespace std;


namespace {
    // map of <sum, index>
    unordered_map<int, int> mp{};
};

int main() {
    vector<int> a{ 22, -7, 15, -21, -20, -8, 16, -20, 5, 2, -15, -24, 19, 25, 3, 23, 18, 0, 16, 26, 13, 4, -20, -13, -25, -2 };
    //9 -2 25 7 -21 -46 -31 -8 -13 2 -7 -24 -9 12 -6 -1 11
    vector<int> res{};
    int sum = 0, start = 0, end = -1, diff = INT_MIN;
    for(int i = 0; i <  a.size() ; ++i) {
        sum += a[i];
        cout<<"Sum is: "<<sum<<endl;
        if(sum == 0) {
             if(diff < i) {
                start = 0;
                end = i;
                diff = i;
            }

        }
        // check if exists
        if(mp.find(sum) == mp.end())
            mp.insert( {sum, i} );
        else {
            int old = mp.find(sum)->second;
            if(diff < (i-old)) {
                start = old+1;
                end = i;
                diff = i-old;
            }
        }   
    }
    // Create vector
    for(int i = start; i <= end; ++i) 
        res.push_back(a[i]);
    
    for(auto a : res)
        cout<<a<<" ";
    
    return 0;
}
                
                 
