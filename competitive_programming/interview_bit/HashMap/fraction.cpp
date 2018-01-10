#include<iostream>
#include<set>
#include<string>
#include<algorithm>
#include<iomanip>
#include<cstdio>

using namespace std;


int main() {
    set<int> st{};
    long double i = 0;
    long double j = 0; 
    cout<<"Enter two integers: ";
    cin>>i>>j;
    char buf[64] = {0};
    bool repetition = false;
    bool zero_found = true;
    string decimal = "";
    string res_f = "";
    string result = "";

    // Add data to buffer
    sprintf(buf, "%.32Lf", i/j);
    string s(buf);
    decimal = s.substr(0, s.find("."));
    result += decimal;
    string fraction =  s.substr(s.find(".") + 1); 
    string ma = "", am = "";
    reverse(fraction.begin(), fraction.end()); 
    for(int i = 0; i < fraction.size(); ++i) {
        if(fraction[i] == '0' && zero_found) {
            continue;
        }
        else {
            zero_found = false;
            am += fraction[i];
        }
    }
    fraction = am;
    reverse(fraction.begin(), fraction.end()); 
    
    if(fraction.size() > 0) {
        for(auto& a : fraction) {
            if(st.find(a) != st.end()) {
                repetition = true;
                break;
            }
            res_f += a;
            st.insert(a);
        }
        result += ".";
        if(repetition)
            result += "(";
        result += res_f;
        if(repetition)
            result += ")";
    }
    cout<<"Result is: "<<result<<endl;   
    return 0;
}

