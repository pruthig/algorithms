#include<iostream>
#include<vector>

using namespace std;

int main() {
	int arr[] = { 1, 2, 3, 4, 5, 6};
	bool isRight = true;
	int size = sizeof(arr)/sizeof(int);
	for(int i = 0 ; i < size-1; i++) {
		if(isRight) {
			if(arr[i] < arr[i+1])
				swap(arr[i], arr[i+1]);
			isRight = false;
		}
		else {
		
			if(arr[i] > arr[i+1])
				swap(arr[i], arr[i+1]);
			isRight = true;
		}
	}

	cout<<"After change\n";
	vector<int> vInt(arr, arr+size);
	for(auto a : vInt) {
		cout<<a<<" ";
	}
	return 0;
}
