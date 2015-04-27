//max sum with no 2 number adjacent
/*
Loop for all elements in arr[] and maintain two sums incl and excl where incl = Max sum including the previous element and excl = Max sum excluding the previous element.

Max sum excluding the current element will be max(incl, excl) and max sum including the current element will be excl + current element (Note that only excl is considered because elements cannot be adjacent).

At the end of the loop return max of incl and excl.
*/
#include<iostream>

using namespace std;

void findMax();
int max(int u,int v);

int arr[] = {12, 3, 4, 56, 9, 21, 90, 24};
//extra array for DP
int n[8];


int main(){
findMax();
return 0;
}

void findMax(){
n[0] = arr[0];
n[1] = arr[1];

for(int i = 2; i < 8 ; ++i){
if(i-2 >= 0){
	n[i]=max(n[i-1],n[i-2]+arr[i]);
}

}

cout<<"MAx is "<<cout<<n[7];

}


int max(int u,int v){
return u > v ? u:v;
}
