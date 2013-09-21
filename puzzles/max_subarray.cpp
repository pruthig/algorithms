#include<iostream>
#include<stdlib.h>


using namespace std;

int main()
{

  int dimension;
  cout<<"Please enter the dimension for input array :"<<endl;
  cin>>dimension;
  if(dimension <=2 ){
    cout<<"Invalid input "<<endl;
    return -1;
  }
  
  int *a   = (int*)malloc(sizeof(int) * dimension);
  int *max = (int*)malloc(sizeof(int) * dimension-1);

  cout<<"Enter the input values for your array\n";
  for(int i=0;i<dimension; i++)
    cin>>*(a+i);

  //int a[] = {1, -1, 8, 34, -98, 3, -5};
  for(int i=0;i<dimension-1;i++)
    *(max+i) = 0;

  for(int i=0;i<=dimension-2;i++)
  {
    for(int j=i;j<dimension;j++)
    {
      int dummy  = a[j];
      if(max[i] + a[j] > max[i])
        max[i] = max[i] + a[j];
      else break;
    }
  }

  int maximum = max[0]; 
  cout<<"Initial maximum "<<max[0];
  for(int i=0;i<dimension-1;i++)
    if(max[i] > maximum)
      maximum = max[i];
  cout<<"max sum is ; "<<maximum<<endl;
  return 0;
}




