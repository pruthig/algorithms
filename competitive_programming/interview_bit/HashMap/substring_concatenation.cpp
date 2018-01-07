/* You are given a string, S, and a list of words, L, that are all of the same length.
   Find all starting indices of substring(s) in S that is a concatenation of each word in L exactly once and without any intervening characters.

Example :
S: "barfoothefoobarman"
L: ["foo", "bar"]
You should return the indices: [0,9].
 */

#include<iostream>
#include<map>
#include<vector>
#include<algorithm>

using namespace std;

namespace {
    map<string, int> mp_orig;
    map<string, int> mp_copy;
};


void resetMap() {
    mp_copy = mp_orig;
}

int main() {
    mp_orig.clear();
    mp_copy.clear();
    string A = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    int start = 0;
    vector<string> B { "aaa", "aaa", "aaa", "aaa", "aaa" };
    if(A.size() == 0 || B.size() == 0 || B.size()*B[0].size() > A.size())
        return 0;

    vector<int> res{};

    int visit_count = 0, visit_target = B.size(); // visit target is B.size()
    int word_length = B[0].size(); // word length

    // Add to map
    for(auto& a : B) {
        if(mp_orig.find(a) == mp_orig.end())
            mp_orig[a] = 1;
        else
            mp_orig[a]++;
    }


    for(int i = 0; i <= A.size()-word_length; ++i) {
        start = i;
        visit_count = 0;
        resetMap();
        for(int j = i; j <= A.size(); j += word_length) {
            string substr = A.substr(j, word_length);
            // Word count already reached.. check other options
            if(visit_count == visit_target-1 && mp_copy[substr] == 1) {  
                res.push_back(start);
                break;
            }

            // Word found
            if(mp_copy.find(substr) != mp_copy.end() && mp_copy.find(substr)->second >= 1)  
            {

                ++visit_count;
                mp_copy.find(substr)->second--;
            }
            else {
                break;
            }
        }
    }

    cout<<"\nIndices are: :\n";
    sort(res.begin(), res.end());
    res.erase(std::unique(res.begin(), res.end()), res.end());

    for(auto a: res)
        cout<<a<<" ";
    return 0;
}



