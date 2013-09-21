#include<iostream>
#include<stdlib.h>
#include<sys/time.h>

using namespace std;

void fill(int dimension, int d1, int d2);

int **a;

int main()
{
bool valid_dimension = false;
clock_t t1, t2;
t1 = clock();
int dimension;
cout<<"Enter the dimension"<<endl;
cin>>dimension;

for(int i=2; i<=128; i*=2){
if(dimension == i){
valid_dimension = true;
break;
}
}

if(!valid_dimension){
cout<<"Invalid dimension"<<endl;
return -1;
}

// N is the number of rows 
a = ( int**)malloc(dimension*sizeof(int*));

for (int i = 0; i < dimension; i++ )
{
   a[i] = (int*)malloc(sizeof(int) * dimension);
}


for(int k=0; k<dimension; k++)
  *(*(a+0) + k) = k+1;


cout<<"Intial assignment :"<<endl;

//call fill function recursively
fill(dimension, 0, 0);

for(int i = 0;i <dimension; i++){
for(int j = 0;j <dimension; j++){
	cout<<*(*(a+i) + j)<<", ";
}
	cout<<endl;
}

t2 = clock();

cout<<"Action took :"<<(t2-t1)/CLOCKS_PER_SEC<<" seconds"<<endl;
//end of main function...
return 0;
}

//n represents the dimension of matrix..

void fill(int n, int d1, int d2){



if(n == 1)
return;

cout<<"Value of a[0][4] :"<<a[0][4]<<endl;

fill(n/2, d1, d2);

fill(n/2, d1, (d2+(n/2)));

cout<<"First loop : n = "<<n<<" d1 = "<<d1<<" d2 = "<<d2<<endl;
for(int i = d1; i <= d1 + (n/2) - 1; i++)
for(int j = d2; j <= d2 + (n/2) - 1; j++)
{
  *(*(a + i + (n/2)) +j + (n/2)) = *(*(a + i) + j);
}
cout<<"Second loop n = "<<n<<" d1 =  "<<d1<<" d2 = "<<d2<<endl;

for(int i = d1; i <= d1 + (n/2) - 1; i++)
for(int j = d2 + (n/2); j<= (d2  + n - 1); j++)
{
  *(*(a + i + (n/2)) +j - (n/2))  = *(*(a + i) + j);

}

}



