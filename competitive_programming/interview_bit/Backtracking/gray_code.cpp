/*
   The gray code is a binary numeral system where two successive values differ in only one bit.
   Given a non-negative integer n representing the total number of bits in the code, print the sequence of gray code. A gray code sequence must begin with 0.
   For example, given n = 2, return [0,1,3,2]. Its gray code sequence is:
   00 - 0
   01 - 1
   11 - 3
   10 - 2
 */
// TO achieve this, we'll use a flag flip
// vector<int> Solution::grayCode(int A)
// We will generae vector of strings first
#include<iostream>
#include<vector>
#include<cmath>

using namespace std;

namespace {
    vector<string> res_s{};
    vector<int> res_i{};
    int k;
};

int get_decimal(string binaryString) {
    int value = 0;
    int indexCounter = 0;
    for(int i=binaryString.length()-1;i>=0;i--){

        if(binaryString[i]=='1'){
            value += pow(2, indexCounter);
        }
        indexCounter++;
    }
    return value;
}

void func(string temp, int k, bool flip) {
    if(temp.size() == k) {
        res_s.push_back(temp);
        return;
    }
    if(flip)
        func(temp+'1', k, false);
    else
        func(temp+'0', k, false);

    if(flip)
        func(temp+'0', k, true);
    else
        func(temp+'1', k, true);
}

int main() {
    k = 3;
    res_s.clear();
    res_i.clear();
    func("", k,false);

    for(auto s : res_s)
        res_i.push_back(get_decimal(s));

    for(auto a : res_i)
        cout<<a<<endl;
    return 0;
}

