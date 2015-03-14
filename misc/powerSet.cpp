//power set is a set of all subsets of a given array ...

#include<iostream>
#include<cmath>


using namespace std;

void findAndPrint(char *a, int size);

int main(){

char c[] = {'a', 'b', 'c'};
findAndPrint(c, sizeof(c));
return 0;
}


void findAndPrint(char *a, int size){
//i = 0  to 7
for(int i = 0; i<pow(2, size);i++){

	for(int j = 0; j<size; ++j){
		if( (1<<j) & i)
			cout<<a[j];
	}
	cout<<endl;
}

return;
}
