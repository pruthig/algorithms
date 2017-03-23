#include<iostream>
#include<climits>


using namespace std;


int global[6] = { 1, 2, 5, 10, 50, 100};

int findMin(int *ptr, int i);
int getCount(int number);

int main(){

	int sum;
	int count;
	cin>>count;
	if(count <=0 || count > 1000)
		return 0;

	  for(int j = 1; j <= count; ++j){
		cin>>sum;
		if(sum <= 0 || sum > 1000000)
			return 0;
		int count_res = getCount(sum);
		cout<<count_res<<endl;
	}
	return 0;
}

int findMin(int *ptr, int i){
	int min = INT_MAX;
	for(int j = 0; j <= 5; ++j) {
		if( (i-global[j]) < 0)
			continue;
		if(min < ptr[ i-global[j] ]);
			min = ptr[ i-global[j] ];
	}
	return min;
}

int getCount(int sum){	
	int *arr  = new int[1000000] {0,};
	arr[0] = 0;
	arr[1] = 1;
	arr[2] = 1;
	arr[5] = 1;
	arr[10] = 1;
	arr[50] = 1;
	arr[100] = 1;

	for(int i = 1; i <=sum ; ++i)
		arr[i] = findMin(arr, i) + 1 ;

	return arr[sum];
}

	
	

