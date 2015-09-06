//Program o rotate bits.. a  generic one for all programs
#include<iostream>

using namespace std;

 
int main(){
	long int newNumber = 0, rotationCount = 0, dupNumber = 0;

	cout<<"Enter the number followed by bit rotation count\n";
	cin>>dupNumber>>rotationCount;

	for(int i = 0 ;  i <rotationCount;  ++i){
        //newNumber will hold rotated bits on left hand side, every right side bit of dupNumber is being tested
		//left shifted required number of times and after that simple Or is done
		newNumber |= ( ( dupNumber & 1 ) << (32-rotationCount - i ));
		dupNumber>>=1;
	}
	//NOw Or'ng these 2 numbers
	dupNumber = dupNumber | newNumber;
 
	cout<<"Calculated number is :"<<dupNumber<<endl;
	return 0;
}
