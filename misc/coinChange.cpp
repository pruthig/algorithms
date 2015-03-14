//following is a recursive implementation of coin change problme

#include<iostream>
#include<algorithm>

using namespace std;

int arr[1024] ;
int sum = 0;
int tmp = 0;
int index = 0;

void printArray()
{
		int sum = 0;
		for(int i = 0; i<=1023; ++i){
			if(arr[i] == -1)
				break;
			//sum = sum + arr[i];
			cout<<" "<<arr[i]<<",";
		}
		cout<<endl;
		return;
}

void findSoln(int i)
{

		arr[++index] = i;
		tmp = tmp+i;


		if(tmp >= sum){
			
			if(tmp == sum){
				//cout<<"Calling or with tmp is "<<tmp<<" and i is "<<i<<endl;
				printArray();
			}
			tmp = tmp-i;
			arr[index] = -1;
			--index;
			
			return;
		}
		findSoln(1);
		findSoln(2);
		findSoln(3);

		tmp = tmp-i;
		arr[index] = -1;
		--index;
}

int main()
{

		std::fill_n(arr, 1024, -1);
		printArray();
		cout<<"Enter the sum ";
		cin>>sum;
		for(int j = 1; j<=3; ++j){
			index = -1;
			tmp = 0;
			findSoln(j);
		}

		return 0;
}


