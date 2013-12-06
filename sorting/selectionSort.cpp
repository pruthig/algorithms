//SelectionSort.cpp
//Selection sort works on the principle that for each iteration we find the smallest element and exchange
//it with the index under consideration...
//worst and best case is N^2
//consider the case when the value is at correct position then swapping will fail..


#include<iostream>
#include<stdlib.h>

using namespace std;

int main(){
int count, *elements, min = 0;
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

for(int i=0; i<count-1; i++)
{
	min = i ;
	for(int j=i+1; j<count; j++)
	{
		if(*(elements + j) < *(elements + min))
			min = j;
	}
	//Swap the two
	if(min != i)
	{
            *(elements + i)	 = *(elements + i) + *(elements + min);
	    *(elements + min) = *(elements + i)  - *(elements + min);
	    *(elements + i) = *(elements + i) - *(elements + min);
	}
}

cout<<"After sorting output is "<<endl;
for(int i=0;i<count; i++)
	cout<<*(elements + i)<<" ";
cout<<endl;

return 0;
}






