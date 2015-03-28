#include <iostream>
#include<vector>
#include<initializer_list>

using namespace std;

int arr[] = {1,2,3,4}; 
std::vector<int> v(arr, arr+(sizeof(arr)/sizeof(arr[0])));


void tryMe(std::initializer_list<int> p){

for(auto t:p){
	cout<<"Value is "<<t<<endl;
}
}

int main()
{
	tryMe({1,2,3});
    return 0;
}
