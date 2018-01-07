/* Given a string, find the length of the longest substring without repeating characters.
Example:
The longest substring without repeating letters for "abcabcbb" is "abc", which the length is 3.
For "bbbbb" the longest substring is "b", with the length of 1.
 */

#include<iostream>
#include<climits>
#include<set>
#include<algorithm>

using namespace std;

namespace {
    set<char> st{};
};

int main() {
    string s = "dadbc";
    int max_n = INT_MIN, start = 0, i = 0;
    while(start <= i && i < s.size()) {
        if(st.find(s[i]) == st.end()) {
            st.insert(s[i]);
            ++i;
        }
        else {
            max_n = max ( max_n, (i-start));
            ++start;
            ++i;
        }
    }
    max_n = max ( max_n, (i-start));
    cout<<"max_n is: "<<max_n<<endl;
    return 0;
}
