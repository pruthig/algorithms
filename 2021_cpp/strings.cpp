#include <iostream>
#include <unordered_map>
#include <climits>
#include <vector>
#include <queue>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <set>
#include <iomanip>
#include <map>

using namespace std; 

#define MAX_CHARS 256 

// This function returns true if str1 and str2 are ismorphic 
bool areIsomorphic(string str1, string str2) {
    int m = str1.length(), n = str2.length(); 

    // Length of both strings must be same for one to one 
    // corresponance 
    if (m != n) 
    return false; 

    // To mark visited characters in str2 
    bool marked[MAX_CHARS] = {false}; 

    // To store mapping of every character from str1 to 
    // that of str2. Initialize all entries of map as -1. 
    int map[MAX_CHARS]; 
    memset(map, -1, sizeof(map)); 

    for (int i = 0; i < n; i++) 
    { 
        // If current character of str1 is seen first 
        // time in it. 
        if (map[str1[i]] == -1) 
        { 
            // If current character of str2 is already 
            // seen, one to one mapping not possible 
            if (marked[str2[i]] == true) 
                return false; 

            // Mark current character of str2 as visited 
            marked[str2[i]] = true; 

            // Store mapping of current characters 
            map[str1[i]] = str2[i]; 
        } 

        // If this is not first appearance of current 
        // character in str1, then check if previous 
        // appearance mapped to same character of str2 
        else if (map[str1[i]] != str2[i]) 
            return false; 
    } 

    return true; 
} 

// First first non-repeating character
void nonrepeatingCharacter(string S) {
   int marked[256] = {0}; 
   int res = INT_MAX, n = S.length();
   bool found = false;
   
   for (int i = 0; i < n; i++)
       if (marked[S[i]] == 0 || marked[S[i]] == 1) 
         ++marked[S[i]];
         
   for (int i = 0; i < n; i++)
       if(marked[S[i]] == 1) {
           found = true;
           res = min(res, i);
       }
    if(found)
        cout<<"first non-repeating character is: "<<S[res]<<endl;
    else
        cout<<"All elements are repeated";
}

void concatenatedString(string s1, string s2) 
{ 
    int marked[256] = {0}; 
    int n = s1.length(), m = s2.length();
    string res{};
    
    for (int i = 0; i < n; i++) {
        marked[s1[i]] = 1;
    }
    
    for (int i = 0; i < m; i++) {
        if(marked[s2[i]] >= 1)
            marked[s2[i]] = 2;
        else
            marked[s2[i]] = -1;
    }
    
    for (int i = 0; i < n; i++) {
        if(marked[s1[i]] == 1) {
            res += s1[i];
        }
    }
    
    for (int i = 0; i < m; i++) {
        if(marked[s2[i]] == -1) {
            res += s2[i];
        }
    }

    cout<<"String is: "<<res;
}

// Reverse words in a given string
string reverseWords(string s) {
    vector<string> vec{};
    reverse(s.begin(), s.end());
    size_t pos = 0;
    std::string token{}, res{};
    while ((pos = s.find('.')) != std::string::npos) {
        token = s.substr(0, pos);
        cout<<"token is: "<<token<<endl;
        vec.push_back(token);
        s.erase(0, pos + 1);
    }
    vec.push_back(s);
    
    for(int i=0;i<vec.size();++i) {
        reverse(vec[i].begin(), vec[i].end());
    }
    for(auto a: vec) {
        res += a;
        res += ".";
    }
    res = res.substr(0, res.length());
    return res;
} 

//Sum of numbers in string
int findSum(string str)
{
    string  res{};
    int tmp = 0;
    for(int i=0;i<str.length();++i) {
        if(isdigit(str[i])) {
            res += to_string(str[i]-'0');
        }
        else {
            if(!res.empty()) {
                tmp += stoi(res);
                res = "";
            }
        }
    }
    if(!res.empty()) {
        tmp += stoi(res);
        res = "";
    }
    return tmp;
}

// Find the character in patt that is present at the minimum index in str
int minIndexChar(string str, string patt)
{
    // Your code here
    set<char> st{};
    for(int i=0;i<patt.length();++i) {
        if(st.find(patt[i]) == st.end())
            st.insert(patt[i]);
    }
    for(int i=0;i<str.length();++i) {
        if(st.find(str[i]) != st.end()) {
            return i;
        }
    }
    cout<<"No character present";
    return -1;
}

bool containsAll(int map[], string p, unordered_map<char, int>& mp) {
    for(int i=0;i<p.length(); ++i) {
        if(mp[p[i]-'a'] != map[p[i]-'a']) 
            return false;
    }
    return true;
}


string smallestWindow (string s, string p){
    // Your code here
    if(p.length() > s.length())
        return "-1";
    unordered_map<char, int> mp{}; 
    int map[26] = {0};
    int i=0;
    int n = s.length()-1;
    int s_index = 0, e_index = n-1;
    int window_length = INT_MAX;
    int min_s_index = INT_MAX;
    
    for(int i=0;i<p.length();++i) {
        if(mp.find(p[i]-'a') != mp.end()) {
            mp[p[i]-'a']++;
        }
        else
            mp[p[i]-'a'] = 1;
     }
    ++map[s[0]-'a'];
    while(i<n && s_index<n) {
        if(containsAll(map, p, mp)) {
            if(i - s_index +1 < window_length) {
                window_length = i - s_index +1;
                min_s_index = s_index;
                e_index = i;
            }
            else {
                --map[s[s_index]-'a'];
                ++s_index;
                continue;
            }
        }
        else {
            ++i;
            ++map[s[i]-'a'];
        }
    }
    if(min_s_index == INT_MAX)
        return "-1";
    cout<<"s and e are: "<<min_s_index<<" "<<e_index<<endl;
    return s.substr(min_s_index, e_index-min_s_index+1);
}

// Nth number made of prime digits
int primeDigits(int n) {
    queue<string> qu{};
    qu.push("2");
    qu.push("3");
    qu.push("5");
    qu.push("7");
    string str="";
    for(int i=0;i<n;i++){
        str=qu.front();qu.pop();
        qu.push(str+"2");
        qu.push(str+"3");
        qu.push(str+"5");
        qu.push(str+"7");
    }
    int t=stoi(str);
    return t;
}

// Find the minimum number of characters which needs to be inserted in a string 
// so that consecutive characters cannot be >=3 in length
int stringWithNoConsecutive3Chars(string str) {
    if(str.length() <= 2)
        return 0;
    char prev_char = str[0];
    int local_count = 1;
    int i=1, counter = 0;

    while(i<str.length()) {
        if(str[i] == prev_char)
            local_count++;
        else {
            if(local_count >= 3) {
                if(local_count%2 == 0)
                    counter += (local_count/2-1);
                else
                    counter += local_count/2;
            }
            local_count = 1;
            prev_char = str[i];
        }
        ++i;
    }
    if(local_count >= 3) {
        if(local_count%2 == 0)
            counter += (local_count/2-1);
        else
            counter += local_count/2;
    }
    return counter;
}

int factorial(int n) { 
    return (n==1 || n==0) ? 1: n * factorial(n - 1);  
} 

int lexicographical_rank(string str) {

    map<char, int> mp{};
    int count = 0;
    // To optimize this solution... use count array of size [256] and
    // where value at every index contains count of smaller characters in whole string
    for(int i=0;i<str.length();++i) {
        if(mp.find(str[i]) != mp.end()) {
            mp[str[i]]++;
        }
        else
            mp[str[i]] = 1;
    }
    int idx = 0;
    for(int i=0;i<str.length(); ++i) {
        idx = std::distance(std::begin(mp), mp.find(str[i]));
        count += idx*((factorial(mp.size()-1)));
        mp.erase(str[i]);
    }
    return count+1;
}
  
// Rabin-karp string search algorithm. In this algorithm, we use rolling hash to find the string
// First, we will generate hash of actual pattern and then try matching that hash with hash of
// all substrings of that length in main string. The beauty of this algo is that you don't need to 
// calculate hash of complete substring... it's just subtraction of first element, division by prime
// number and adding next_value*prime^(len-1)
bool robin_karp_search(string str, string patt) {
    int n = str.length();
    int m = patt.length();
    
    if(n < m)
        return false;
    // calculte hash of pattern, let prime number be 3
    int hash_patt = 0;
    for(int i=0;i<m;++i)
        hash_patt += (patt[i] - 'a' + 1)* pow(3, i);
        
    // Now calculate rolling hash..
    // First calculate hash of 'n' characters in main string.'n' being length of pattern
    int hash_str = 0;
    for(int i=0;i<m;++i)
        hash_str += (str[i] - 'a' + 1)* pow(3, i);
    
    if(hash_str == hash_patt) {
        // re-verify
        int j=0;
        for(;j<m;++j)
           if(str[j] != patt[j])
                break;
        if(j == m)
            return true;
    }
        
    // pattern is not in first 'n' characters.. search main string in full now
    for(int i=m;i<n;++i) {
        // Subtract the first character
        hash_str -= (str[i-m] - 'a' + 1);
        // Divide by prime number, which is 3 in our case
        hash_str /= 3;
        // Add the current value*(pattern_length-1)
        hash_str += (str[i] - 'a' + 1)* pow(3, m-1);
        
        // Match now
        if(hash_str == hash_patt) {
            // re-verify
            int j=0;
            for(;j<m;++j)
               if(str[i-m+1+j] != patt[j])
                    break;
            if(j == m)
                return true;
        }
    }
    return false;
}

// The function returns 1 if IP string is valid else return 0
int isValidIP(string s) {
    
    vector<string> vec{};
    for(int i=0;i<s.length();++i) {
        if(isalpha(s[i]))
            return 0;
    }
    size_t pos = 0;
    std::string token{}, res{};
    while ((pos = s.find('.')) != std::string::npos) {
        token = s.substr(0, pos);
        if(token.length() > 3 || token.length()<=0) 
            return false;
        if(token[0] == '0' && token != "0")
            return false;
        vec.push_back(token);
        s.erase(0, pos + 1);
    }
    if(s.length()>3 || s.length()<=0)
        return 0;
    if(s[0] == '0' && s != "0")
        return false;

    vec.push_back(s);
    if(vec.size() != 4)
        return 0;
    for(auto a : vec) {
        int i = 0;
        try{
            i = stoi(a);
        }
        catch (const std::invalid_argument& ia) {
	        return 0;
        }
        if(i <0 || i > 255)
            return 0;
    }
    return 1;
}

// This program prints the interleaved strings. An interleaved string of given two strings
// preserves the order of characters in individual strings
void printInterleave(string s1, string s2, int len1, int len2, int i, int j, string res, vector<string>& vec) {
	if(i == len1 && j == len2) {
		vec.push_back(res);
		return;
	}
	if(i < len1) {
		printInterleave(s1, s2, len1, len2, i+1, j, res+s1[i], vec);
	}
	if(j < len2) {
		printInterleave(s1, s2, len1, len2, i, j+1, res+s2[j], vec);
	}
	else { /* do nothing */ }

}

// Searching made simple
bool KMP(string str, string patt) {
    int m = patt.length();
    int n = str.length();
    if(m > n)
        return false;
    // construct partial match table from pattern
    int i = 1;
    int *pmt = new int[m];
    pmt[0] = 0;
    while(i < m) {
        for(int j = 0; j < m;) {
            if(patt[i] == patt[j]) {
                pmt[i] = j+1;
                ++j; ++i;
            }
            else {
                pmt[i] = 0;
                ++i;
                break;
            }
        }
    }
    // Search for pattern now
    i = 0;
    while(i<n) {
        int j=0;
        for(;j<m;++j) {
            if(str[i+j] != patt[j])
                break;
        }
        if(j == m)
            return true;
        else if(j != 0){
            int partial_match_len = j;
            i = i + partial_match_len - pmt[partial_match_len-1];
            continue;
        }
        ++i;
    }
    return false;
}
    
// The main function that checks if two given strings
// match. The first string may contain wildcard characters
bool match(char *first, char * second)
{
    // If we reach at the end of both strings, we are done
    if (*first == '\0' && *second == '\0')
        return true;
  
    // Make sure that the characters after '*' are present
    // in second string. This function assumes that the first
    // string will not contain two consecutive '*'
    if (*first == '*' && *(first+1) != '\0' && *second == '\0')
        return false;
  
    // If the first string contains '?', or current characters
    // of both strings match
    if (*first == '?' || *first == *second)
        return match(first+1, second+1);
  
    // If there is *, then there are two possibilities
    // a) We consider current character of second string
    // b) We ignore current character of second string.
    if (*first == '*')
        return match(first+1, second) || match(first, second+1);
    return false;
}
  
// A functio
int main() 
{ 
    //cout << areIsomorphic("aaba", "xxyx") << endl; 
    //cout<< nonrepeatingCharacter("bafab");
    //concatenatedString("bafab", "afa");
    //cout<<reverseWords("gaurav.pruthi.india")<<endl;
    //cout<<"Sum of integers in string is: "<<findSum("1ac3s24c9")<<endl;
    //cout<<minIndexChar("geeksforgeeks", "set")<<endl;
    //cout<<"Substring is: "<<smallestWindow("timetopractice", "toc")<<endl;
    //cout<<primeDigits(10)<<endl;
    //cout<<"Minimum characters insertion is: "<<stringWithNoConsecutive3Chars("aaaabbbccddddddbbaabbbccc")<<endl;
    //cout<<"Lexicographical rank of string is: "<<lexicographical_rank("cba")<<endl;
    //cout<<"Robin karp..is pattern found in string: "<<robin_karp_search("tiger in the hill", "hall")<<endl;
    //int s = isValidIP("67.53.56.29");
    cout<<std::boolalpha<<"Does string exist : "<<KMP("abababca", "babc")<<endl;
 
    return 0; 
} 


// Notes:
// To find longest non-palindromic substring if all chars are equal, then it's 0, if complete string is not a palindrome, return n, else it'll be n-1 if we remove a single character from either side.
// Magic square: 3 conditions hold ( (i-1, j+1), (i+1, j-2), (0, n-2))
// 1. The position of next number is (i-1, j+1)
// 2. If row becomes -1, it will wrap around to n-1. Similarly, if column becomes n, it will wrap around to 0.
// 3. If square already contains a number at that position, column position will be (j-2) and row will be (i+1) after calculation.
// 4. If the row position is -1 & column position is n, the new position would be: (0, n-2). 


