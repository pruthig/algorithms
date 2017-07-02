// This program is the solution to Dutuch national flag problem where-in we have array of values 0, 1, and 2.
// We have to arrange values in arrays such that we have values as 0 -- 1-- 2 in sequence
#include<iostream>

using namespace std;


int main() {
	int arr[] = { 0, 1, 2, 1, 2, 1, 0, 0, 2};
	int size = sizeof(arr)/sizeof(int);
	// variable to traverse array
	int i0 = -1, i2 = size;
	int i = 0;  // Index for traversal...
	
	// correcting the position of i2
	while(arr[i2-1] == 2)
		--i2;
	// Left of i will be 0 and 1, while right will contain undiscovered elements
	while( i < i2 ) {
		if(arr[i] == 0) {
			swap(arr[i0+1], arr[i]);
			i0++; i++; 
		}
		else if(arr[i] == 2 ) {
			swap(arr[i2-1], arr[i]);  // Don't increment because we don't know what is the current element after swap
			--i2;
		}
		else
			i++;
	}
	// Now that we have traversed whole array, we will print
	for(auto a : arr)
		cout<<a<<" ";
}
