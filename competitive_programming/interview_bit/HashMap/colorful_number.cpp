// For Given Number N find if its COLORFUL number or not
// A number can be broken into different contiguous sub-subsequence parts. 
// Suppose, a number 3245 can be broken into parts like 3 2 4 5 32 24 45 324 245. 
// And this number is a COLORFUL number, since product of every digit of a contiguous subsequence is different

#include<iostream>
#include<string>
#include<set>

using namespace std;

namespace{
    set<long> st{};
};

long get_product(long n) {
    long p = 1;
    while(n) {
        p = (p)*(n%10);
        n /= 10;
    }
    return p;
}

int main() {
    st.clear();
    long number = 3245;
    string s = to_string(number);
    for(int i = 0; i < s.size(); ++i) {
        for(int j = i; j < s.size(); ++j) {
            string t = s.substr(i, j-i+1);
            cout<<"Substr is: "<<t<<endl;
            long product = get_product(atol(t.c_str()));
            if(st.find(product) != st.end()) {
                cout<<"Number is not colorful\n";
                goto EXIT;
            }
            else {
                st.insert(product);
            }
        }
    }
    cout<<"Number is colorful\n";

EXIT:
    return 0;
}

                
