//best approach for spiral is here
//just print one less than the last element 
#include<iostream>


using namespace std;

void right_down(int a, int b, int c, int d);
void left_up(int a, int b, int c, int d);


int arr[][4] = 
{
{ 1,  2,   3,  4},
{ 12, 13,  14,  5},
{ 11,  16,  15,  6},
{ 10,  9,  8,  7}

};


int main(){
right_down(0, 0, 3,3);
return 0;

}




void right_down(int a, int b, int c, int d){
  if(a > c || b > d)
	return;

  //Move right
  for(int i = b; i<=d-1; ++i)
	cout<<" "<<arr[a][i];

  //Move down
  for(int i = a; i <= c-1; ++i)
       cout<<", "<<arr[i][d];
  left_up(a, b, c, d);
}


void left_up(int a, int b, int c, int d){
 if(a > c || b > d)
	return;


 for(int i = d; i >= b+1; i--)
	 cout<<", "<<arr[c][i];
  
 //Move up
 for(int i = c; i  >= a+1; --i)
	cout<<" , "<<arr[i][b];

  right_down(a+1, b+1, c-1, d-1);
}
 //Move left
  
