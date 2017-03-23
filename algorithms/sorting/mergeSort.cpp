//MergeSort.cpp
//MergeSort works on the principle of divide and conquer... dividing the array into smaller subarrays
//sorting the smaller subarrays and finally merging all of them .. 
//merging takes O(n) time and sorting takes O(logn)-> O(n*logn) in total


#include<iostream>
#include<stdlib.h>

using namespace std;

void sort(int low, int high, int *p_arr);
void merge(int lo, int mid, int hi, int *ptrArr);


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

sort(0, count-1, elements);

cout<<"After sorting output is "<<endl;
for(int i=0;i<count; i++)
	cout<<*(elements + i)<<" ";
cout<<endl;
system("pause");
return 0;
}

void sort(int low, int high, int *p_arr)
{
	int mid= low + (high - low)/2;
	if(mid == high)
	{
		if(*(p_arr + mid) < *(p_arr + low) )
		{
		cout<<"Valu eof mid :"<<*(p_arr + mid)<<endl;
		*(p_arr + mid) =  *(p_arr + mid)  + *(p_arr + low) ;
		*(p_arr + low) =  *(p_arr + mid)  - *(p_arr + low) ;
		*(p_arr + mid) =  *(p_arr + mid) - *(p_arr + low);
		}
		return;
	}
	
	if(mid == low)
	{
		if(*(p_arr + high) < *(p_arr + mid) )
		{
		 cout<<"Valu eof mid :"<<*(p_arr + mid)<<endl;
		*(p_arr + mid) =  *(p_arr + mid)  + *(p_arr + high) ;
		*(p_arr + high) = *(p_arr + mid)  - *(p_arr + high) ;
		*(p_arr + mid) =  *(p_arr + mid) - *(p_arr + high);
		}
		return;
	}

sort(low, mid, p_arr);
sort(mid+1, high, p_arr);
merge(low, mid,  high, p_arr);
}

void merge(int lo, int mid, int hi, int *ptrArr)
{
	int i = 0, j = 0; //indices for 2 dup. arrays
	
	//find the number of elements..
	int count = (hi - lo + 1);
	//create a duplicate copy of array
	int *dup1 = (int*)malloc(sizeof(int) * (mid-lo+1));
	int *dup2 = (int*)malloc(sizeof(int) * (hi - mid));
	
	//fill both arrays..
	for(int i=0; i < (mid-lo + 1); i++)
		*(dup1 + i) = *(ptrArr + lo + i);
		
	for(int i = 0; i < (hi - mid); i++)
	*(dup2 + i) = *(ptrArr + mid +1 + i);



	for(int k = lo; k <= hi; k++)
	{
		if(i == (mid-lo+1)) 
		{
			*(ptrArr + k) = dup2[j]; 
			j++;
			continue;
		}
		if(j == (hi - mid))
		{
			*(ptrArr + k) = dup1[i]; 
			i++;
			continue;
		}
		
		if(dup1[i] < dup2[j]) 
		{
			*(ptrArr + k) = dup1[i]; 
			i++;
		}
		else//(dup2[j] < dup1[i]) 
		{
			*(ptrArr + k) = dup2[j]; 
			j++;
		}
	}
}




