//this program will find the  contiguous sum

#include<iostream>


using namespace std;


int main()
{
	int max, t;

	int arr[10] = { 1, 1, 1, 1, 2, 1, 2, 23, 12, 1};

	int arrSize = sizeof(arr)/sizeof(arr[0]);
	max = 0;
	for(int i = 1; i < arrSize; i++)
	{
		t = max + arr[i];
		if((arr[i] + t) > max)
			max = t;
		else{
			max = 0;
			continue;
		}
	}
	cout<<"Max Sum is "<<max<<endl;

	return 0;
}
	
	 




