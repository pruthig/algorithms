#include<iostream>


using namespace std;

int index = 0;
int a[] = {1, 5, 3, 89, 9, 10, 10, 43, 4, 4 };

void remove(int);
void print();

void print(){

for(int i = 0; i <= 9; ++i)
	cout<<a[i]<<" ,";
}


int main(){

remove(0);
print();
return 0;
}

int cur_index = 0;

void remove(int index){
if(index == 9){
	cur_index = 9;
	return;
}

remove(index+1);
if(a[index] == a[cur_index])
	a[index] = -1;
else
	cur_index = index;

}




