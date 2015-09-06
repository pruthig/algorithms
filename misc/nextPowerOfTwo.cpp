//Find the next power of 2 for a given element
#include<iostream>
#include<cmath>
using namespace std;

int main(){
int oneBitCount = 0, totalBitCount = 0, number = 0;
cout<<"Enter the number\n";
cin>>number;

while(number){

	if(number & 1 == 1) {
		oneBitCount++;
	}
	
	number >>= 1;
	totalBitCount++;
}
if(oneBitCount == 1)
	cout<<"Number already power of 2\n";
else 
	cout<<"Number is "<<pow(2, totalBitCount)<<endl;;
return 0;
}
  	

