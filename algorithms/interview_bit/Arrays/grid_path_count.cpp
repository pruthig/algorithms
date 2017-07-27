// Print number of grid unique paths 
#include<iostream>
#include<vector>

using namespace std;

void findPaths(int& count, int i, int j, int A, int B);

// Let the dimensions be A * B, So the base cases are: 
// >=0 && < A and
// >=0 && < B
// Go down and right, when we find corrdinate as A-1, B-1 we will increment the count
// Here we go
int main() {
	int count = 0;
	int A = 7, B = 7;
	findPaths(count, 0, 0, A, B);
	cout<<"Count is: "<<count;
}

void findPaths(int& count, int i, int j, int A, int B) {
	if(i == A-1 && j == B-1) {
		++count;
		return;
	}
		
	if(i < 0 || j < 0  || i >=A || j >= B)
		return;
	// Move right
	findPaths(count, i, ++j, A, B);
	--j;
	findPaths(count, ++i, j, A, B);
	
}
