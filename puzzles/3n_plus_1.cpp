/*For an input n, the cycle-lengthofnis the number of numbers generated up to and
including the 1. In the example above, the cycle length of 22 is 16. Given any two
numbersi andj, you are to determine the maximum cycle length over all numbers
betweeniandj, including both endpoints.
*/

#include<iostream>

using namespace std;


int main(){
int max_cycle = 0;
int counter = 0;
int p1, p2;
cout<<"Enter the integers for range\n";
cin>>p1>>p2;

if(p1 >= 1000000 || p2 >= 1000000){
cout<<"Invalid input , should be < 1,000,000\n";
return -1;
}

for(int i = p1; i <= p2; i++)
{

int temp = i;
counter = 0;

while(temp!=1)
{
	temp = temp%2?(3*temp+1):temp/2;
	counter++;
}

counter++;
max_cycle = (max_cycle<counter)?counter:max_cycle;
}//End of for loop

cout<<"Max cycle range found is :"<<max_cycle<<endl;

return 0;
}





