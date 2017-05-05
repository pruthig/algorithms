#include<iostream>
#include<algorithm>
#include<string>

using namespace std;


int main(){
    string str;
    int ind = 0;
    bool found = false;
    cout<<"Enter string\n";
    cin>>str;
    int i = str.length()-1;
    if(i == 0){
        cout<<"Single letter entered"<<endl;
        return 0;
    }
    while(i>=1){
        if(str[i]>str[i-1]) {
            found = true;
            break;
        }
        --i;
    }
    if(!found) {
        cout<<"No next palindrome exists"<<endl;
        return 0;
    }
    char min='{';
    for(int j=i; j<=str.length()-1; ++j) {
        if(str[j] < min) {
            min = str[j];
            ind = j;
        }
    }
    swap(str[i-1], str[ind]);
    sort(str.begin()+i, str.end());
    cout<<"Next is : "<<str<<endl;
    return 0;
}
