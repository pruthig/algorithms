// The zigzag string takes a string and number of rows and feed that string to vector with
// number of rows
#include<vector>
#include<iostream>

using namespace std;


int main() {
    string s = "ITEM", intr = "";
    int rows = 1;
    bool down = true, up = false;
    vector<string> v;
    for(int i = 0; i < rows; ++i)
        v.push_back("");
    int i = 0; //index for string
    int j = 0; //index for vector
    while(i < s.size()) {
        //if(i == s.size()-1)
        //    break;
        if(down) {
            for(;j < rows && i < s.size(); ++j, ++i)
                v[j] += s[i];
            down = false;
            up = true;
            if(rows>1)
                j = rows-2;
            else
                j = rows-1;
        }
        if(up) {
            for(;j >=0 && i < s.size(); --j, ++i)
                v[j] += s[i];
            down = true;
            up = false;;
            if(rows>1)
                j += 2;
            else
                j+=1;
        }
    }

    for(auto a : v) {
        cout<<"a is: "<<a<<endl;
        intr = intr + a;
    }
    cout<<"ZigZag string is: "<<intr<<endl;
    return 0;
}
    
