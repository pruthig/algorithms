//this program will find the  majority elemnt(repeated N/2 or more times
//in an array of size N

#include<iostream>


using namespace std;


int main()
{
	int count;
	int element;

	int arr[10] = { 1, 1, 1, 1, 2, 1, 2, 23, 12, 1};

	int arrSize = sizeof(arr)/sizeof(arr[0]);

	count = 1;
	element  = arr[0];
	for(int i = 1; i < arrSize; i++)
	{

		if(arr[i] == element)
		{
			count++;
			
			continue;
		}

		else 
		{
			--count;
			if(count == 0)
			{
				element = arr[i];
				count = 1;
			}
		}
	}
	if(count = arrSize/2){
		cout<<"Majority element is ;"<<element<<endl;
	}
	else 
		cout<<"Major element not found"<<endl;

	return 0;
}
	
	 




