//This program will find the equilibrium index, which has the left sum = right sum if is there.

#include<iostream>


using namespace std;


int main()
{
	int sum = 0;
	int leftSum = 0;
	int index = -1;
	int arr[] = { -1, 2, -1};

	int arrSize = sizeof(arr)/sizeof(arr[0]);
	//find the sum of whole array
	for(int i = 0; i < arrSize; i++)
		sum = sum + arr[i];

	//total - 2*left = ptr
	for(int i = 0; i < arrSize; i++){

	if(sum - 2*leftSum == arr[i]){
		index = i;
		cout<<"Index is :"<<index<<endl;
	}
	leftSum = leftSum + arr[i];
	}

	if(index == -1)
		cout<<"No equilibrium index\n";
		

	return 0;
}
	
	 




