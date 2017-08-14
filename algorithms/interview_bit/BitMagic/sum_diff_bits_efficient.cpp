#include<iostream>
#include<vector>


using namespace std;

int main() {
	int arr[32] = {0}; // Array to hold sum of 1's
	vector<int> vec{34, 56, 76, 12, 16, 72, 73, 78, 98, 90, 52, 54, 56, 58, 21, 24, 27, 28, 98, 78};
	int sum = 0;
	int size = vec.size();
	for(int i = 0; i < size; ++i) {
		for(int j = 31; j >= 0; j--) {
			if((1<<j)&vec[i])
				arr[j]++;
			if(arr[j] > 1000000007)
				arr[j] = arr[j]%1000000007;
		}
	}
	
	
	for(int i = 0; i <= 31; ++i) {
		if(arr[i] != 0 && arr[i] != size)
			sum += (size-arr[i])*arr[i]*2;
		if(sum > 1000000007)
			sum = sum%1000000007;
	}

	
	cout<<"Sum is: "<<sum<<endl;
	return 0;
}
