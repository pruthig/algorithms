#include<iostream>


using namespace std;


int main(){
int number = 0;
int n = 45;
int a[45];
a[0] = a[1] = 1;

for(int i=2; i<45; ++i)
	a[i] = a[i-1] + a[i-2];

cout<<"Enter what you want";
cin>>number;

cout<<"The solution is "<<a[number]<<endl;

return 0;
}
