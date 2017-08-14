#include<iostream>

using namespace std;


int main() {
	unsigned int old_number = 0;
	
	int count = sizeof(int)*8 -1;
	unsigned int new_number = 0;
	while(old_number != 0 && count >= 0) {
		int last_bit = old_number&1;
		if(last_bit)
			new_number ^= (1<<count);
		old_number>>=1;
		--count;
	}
	cout<<"New number is: "<<new_number<<endl;
	return 0;
}
