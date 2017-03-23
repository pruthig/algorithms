#include<string>
#include<stdlib.h>
#include<iostream>

using namespace std;

string reverse(string & s){

int r = s.size()-1;
int l = 0;

while(l<r){
char tmp = s[l];
s[l] = s[r];
s[r] = tmp;

l++;
r--;
}

return s;

}
int main(){
int count;
int n1, n2;
cin>>count;
for(int i = 1; i <= count; ++i){
cin>>n1>>n2;

string s1 = to_string(n1);
string s2 = to_string(n2);

int n1rev = stoi( reverse(s1) );
int n2rev = stoi( reverse(s2) );

int origSum = n1rev + n2rev;
string s3 = to_string(origSum);
cout<<endl<<stoi(reverse(s3));
}

return 0;
}
