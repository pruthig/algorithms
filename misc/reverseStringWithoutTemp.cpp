#include<iostream>

using namespace std;


int main(){

char str[] = "crackpot";


int i = 0;
int j = sizeof(str)/sizeof(char) - 1;

while(i < j){
str[i]  = char ( str[i] + str[j]  );
str[j]  = char ( str[i] - str[j] );
str[i] = char ( str[i] - str[j] );

++i;
--j;
}
for(int i = 0; i<= 8; ++i)
cout<<str[i]<<endl;
return 0;
}





