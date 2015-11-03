#include<iostream>

using namespace std;


int main(){

	int sum = 0;
	int count = 0;
	int number = 0;
	cin>>count;

	for(int j = 1; j <= count; ++j){
	  cin>>number;
	  if(number>=1 && number <=3)
		cout<<"0"<<endl;
	  else {
		
        	for(int i = ( number/2 - 1 ); i >=1 ; --i)
			sum+=i;
		cout<<sum<<endl;
  		}
	sum = 0;
	}
	return 0;
}


	
	

