//this program finds the k nearest elements around the number
//a[k]

#include<iostream>


using namespace std;


int main()
{
	int index = 0;
	int lSum = 0;
	int rSum = 0;
	int k;

	int a[] = { -1, -2, -3, 5, 9, 10};

	int n = sizeof(a)/sizeof(a[0]);
 	
	cout<<"Enter the index\n";
	cin>>k;
	if(k < 0 || k >= n){
		cout<<"Invalid input\n";
		return 0;
	}

	int l = k-1;
	int r = k+1;
	

	while(l >= 0 && r <= n-1 && (r - l) <= (k+2)) 
	{

		if(a[k] - a[l] < a[r] - a[k])
		{
			cout<<"Index"<<l<<"l = "<<l<<"r = "<<r<<endl;
			l--;
		}
		else
		{
			cout<<"Index"<<l<<"l = "<<l<<"r = "<<r<<endl;
			r++;
		}
	}


	if((r - l) <= (k+2))
	{
		if(l == 0)
			while(r <= n-1){
				cout<<"Index is "<<r<<endl;
				r++;
			}
		else if(r == n)
			while(l >= 0){
				cout<<"index is "<<l<<endl;
				l--;
			}
	}

	return 0;
}
	
	 




