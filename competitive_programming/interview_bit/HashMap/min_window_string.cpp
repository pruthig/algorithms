// Given a string S and a string T, find the minimum window in S which will contain all the characters in T in linear time complexity.
// S = "ADOBECODEBANC"
// T = "ABC"
// Minimum window is "BANC"
#include<string>
#include<unordered_map>
#include<climits>
#include<iostream>
#include<algorithm>

using namespace std;


int main() {
    string S = "rGusddQS6UvK9GzxPSJDMSyoTOpkLK18ZfxKF64HwZ0";
    string T = "o8athbAkGyGg7B79xJzPZAXmnqw1dWlUMmA3LehdRaXl2S7HVrgmpUvj9m2RtnZggXG9B";
    if(S.size() == 0 || T.size() == 0 || T.size() > S.size())
        return 0;
    char reqd[255] = {0};
    int actual[255] = {0};
    int diff = INT_MAX;
    int reqd_count = 0;
    bool found = false;
    for(int i = 0; i < T.size(); ++i) {
        reqd[ (T[i] - ' ') ]++;
        reqd_count++;
    }
    int start = 0, end = -1;
    int actual_count = 0;
    for(int i = 0; i < S.size(); ++i) {
        int index = S[i]-' ';
        if(reqd[index] != 0) {
            if(actual[index] < reqd[index])
                actual_count++;
            ++actual[index];
            if(actual_count == reqd_count && !found){
                found = true;
                end = i;
                diff = end-start;
            }
            // Update start index
	    int startPos = S[start]-' '; 
            while(reqd[startPos] == 0 || actual[startPos] > reqd[startPos]) {
                if( actual[startPos] > reqd[startPos])
                    --actual[startPos];
                ++start;
                startPos = S[start]-' '; 

            }
            if(actual_count == reqd_count && i -start < diff)
                end = i;
        }
    }
    cout<<"Substring is: "<<S.substr(start, end-start+1)<<endl;
    if(end != -1) 
        cout<<"Start and End indices are: "<<start<<" and "<<end<<endl;

    return 0;
}
