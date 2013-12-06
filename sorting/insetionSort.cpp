//InsertionSort.cpp
//Insertion sort works on the principle that all of the elements in the array on left are already sorted
// and we have to insert each element to its proper position by backtracking
//worst case is N^2 and best case is N when array is already sorted..in that case no backtracking


#include<iostream>
#include<stdlib.h>

using namespace std;

int main(){
int count, *elements;
cout<<"Enter the no. of elements"<<endl;
cin>>count;

elements = (int*)malloc(sizeof(int) * count);
cout<<"Enter the elements now oneByone"<<endl;
for(int i=0;i<count; i++)
cin>>*(elements + i);

cout<<"Before sorting output is "<<endl;

for(int i=0;i<count; i++)
	cout<<*(elements + i)<<" ";
cout<<endl;

//Sorting code;

for(int i=1; i<count; i++)
{
	for(int j=i; j>0; j--)
	{
		if(*(elements + j) < *(elements + j-1))
	        {
			//Swap the 2
			*(elements + j)	 = *(elements + j) + *(elements + j - 1);
	          	*(elements + j - 1) = *(elements + j)  - *(elements + j - 1);
	    		*(elements + j) = *(elements + j) - *(elements + j - 1);
		}
		else
			break;
	}
            
}

cout<<"After sorting output is "<<endl;
for(int i=0;i<count; i++)
	cout<<*(elements + i)<<" ";
cout<<endl;

return 0;
}






