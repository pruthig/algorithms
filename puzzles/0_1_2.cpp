//dutch national flag problem

/*
0,1 --------------------------- 2
-->(for 1)                   <---
*/

#include<iostream>


using namespace std;

void swap(int &i, int &j){
	int temp = i;
	i = j;
	j = temp;
}

int main(){
//two pointers to track 0 and 1 on left + 2 on right side
int i = -1;
int j = 0;
int n = 17;
int a[] = { 0, 1, 1, 2, 2, 0, 2, 0, 1, 2, 1, 1, 0, 0, 2, 2, 2, 1};

//so now ptrr0 and ptr1 points to the starting point and  2 to the point where 2 size starts
while(j < 17 && n-j > 1){
	if(a[j] == 0){
		swap(a[++i], a[j]);
		++j;
	}
	else if( a[j] == 2){
		while(a[n] == 2) --n;
		swap(a[n], a[j]);
	}
	else
		++j;
}

for(int i : a)
	cout<<i<<", ";
return 0;
}


	


