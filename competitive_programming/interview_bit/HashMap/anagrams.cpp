/* Given an array of strings, return all groups of strings that are anagrams. Represent a group by a list of integers 
representing the index in the original list. Look at the sample case for clarification.
Input : cat dog god tca
Output : [[1, 4], [2, 3]]
*/

#include<iostream>
#include<unordered_map>
#include<vector>
#include<algorithm>

using namespace std;

namespace {
    // This map will store the mapping of string to the list of indices where the string has appeared..
    unordered_map<string, vector<int> > mp;
}

int main() {

    vector<string> vec { "cat", "tac", "act", "bal", "tca", "lab" }; //, "dog", "god", "tca" };
    vector<vector<int>> res{};
    if(vec.size() == 0)
        return 0;
    vector<string> unique{};
    int i = 0;
    for(auto& a : vec) {
        sort(a.begin(), a.end());
        // if it exists in map..append to list.....else if it doesn't exist add it to map and then to vector
        if(mp.find(a) == mp.end())  // doesn't exist
            unique.push_back(a);
        mp[a].push_back(i+1);
        ++i;
    } // done adding to map
    
    // Now traverse over the string
    for(auto& x : unique) {
        vector<int> tmp{ mp[x] };
        res.push_back(tmp);
    }
    //return res;
    for(auto& a: res) {
        for(auto&b : a) {
            cout<<b<<", ";
        }
        cout<<endl;
    }
    return 0;
}
